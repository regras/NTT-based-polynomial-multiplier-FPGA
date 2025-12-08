`include "defines.v"

module PolyMult ( // Renomeado para v9
    input wire clk,
    input wire reset,

    input wire start_transaction,
    input wire [2:0] mode,
    output reg busy,
    output reg done_all,

    input wire valid_in,
    input wire [`DATA_SIZE_ARB-1:0] din,
    output reg fifo_rd_enable,

    output wire valid_out,
    output wire [`DATA_SIZE_ARB-1:0] dout,
    output reg fifo_wr_enable,
    input wire fifo_full
);

    // --- Estados da FSM (Corrigidos para "done" em pulso) ---
    localparam STATE_IDLE = 6'd0;

    // Modos de Carga (v7 OK)
    localparam STATE_LOAD_W_PULSE  = 6'd1;
    localparam STATE_LOAD_W_WAIT   = 6'd2;
    localparam STATE_LOAD_W_STREAM = 6'd3;
    localparam STATE_LOAD_A_WAIT   = 6'd4;
    localparam STATE_LOAD_A_STREAM = 6'd5;
    localparam STATE_LOAD_B_WAIT   = 6'd6;
    localparam STATE_LOAD_B_STREAM = 6'd7;

    // Sequência "GO" - NTT(A)
    localparam STATE_GO_NTT_A_PULSE     = 6'd8;
    localparam STATE_GO_NTT_A_STREAM    = 6'd9; 
    localparam STATE_GO_NTT_A_BUBBLE    = 6'd10;
    localparam STATE_GO_NTT_A_START     = 6'd11;
    localparam STATE_GO_NTT_A_WAIT_DONE = 6'd12; // <-- NOVO (v9)
    localparam STATE_GO_NTT_A_STREAM_OUT = 6'd13; // <-- NOVO (v9) (Era STORE)
    
    // Sequência "GO" - NTT(B)
    localparam STATE_GO_NTT_B_PULSE     = 6'd14;
    localparam STATE_GO_NTT_B_STREAM    = 6'd15;
    localparam STATE_GO_NTT_B_BUBBLE    = 6'd16;
    localparam STATE_GO_NTT_B_START     = 6'd17;
    localparam STATE_GO_NTT_B_WAIT_DONE = 6'd18; // <-- NOVO (v9)
    localparam STATE_GO_NTTB_STREAM_OUT = 6'd19; // <-- NOVO (v9) (Era STORE_AND_MULT_LOAD)
    
    // Sequência "GO" - Mult e INTT
    localparam STATE_GO_MULT_EXEC    = 6'd20;
    localparam STATE_GO_MULT_READ    = 6'd21;
    localparam STATE_GO_INTT_PULSE   = 6'd22;
    localparam STATE_GO_INTT_STREAM  = 6'd23;
    localparam STATE_GO_INTT_BUBBLE  = 6'd24;
    localparam STATE_GO_INTT_START   = 6'd25;
    localparam STATE_GO_OUTPUT_WAIT  = 6'd26; // (Lógica v8)
    localparam STATE_GO_OUTPUT_STREAM = 6'd27; // (Lógica v8)

    reg [5:0] state; // 6 bits
    reg [15:0] cnt;

    // --- Sinais e RAMs (Inalterados) ---
    reg ntt_load_w, ntt_load_data, ntt_start, ntt_start_intt;
    reg [`DATA_SIZE_ARB-1:0] ntt_din;
    wire ntt_done;
    wire [`DATA_SIZE_ARB-1:0] ntt_dout;
    reg mult_start, mult_wr_en, mult_rd_en;
    reg [`RING_DEPTH-1:0] mult_wr_addr, mult_rd_addr;
    reg [`DATA_SIZE-1:0] mult_wr_A, mult_wr_B;
    wire mult_done;
    wire [`DATA_SIZE-1:0] mult_rd_data;
    reg [`DATA_SIZE_ARB-1:0] ram_a [0:`RING_SIZE-1];
    reg [`DATA_SIZE_ARB-1:0] ram_b [0:`RING_SIZE-1];
    reg [`DATA_SIZE_ARB-1:0] ram_c [0:`RING_SIZE-1]; 
    reg valid_out_reg;
    assign valid_out = valid_out_reg;
    assign dout = ntt_dout; // Conecta a saída da NTT à entrada da FIFO de saída

    // --- Função Bit Reverse (Inalterada) ---
    function [`RING_DEPTH-1:0] bit_reverse_index;
        input [`RING_DEPTH-1:0] x; integer i;
        begin
            for (i = 0; i < (`RING_DEPTH); i = i + 1)
                bit_reverse_index[i] = x[`RING_DEPTH-1-i];
        end
    endfunction
    
    // =========================================================================
    // FSM CONTROL (v9)
    // =========================================================================

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            state <= STATE_IDLE; busy <= 0; done_all <= 0; cnt <= 0;
            // (Resets)
            ntt_load_w <= 0; ntt_load_data <= 0; ntt_start <= 0; ntt_start_intt <= 0;
            ntt_din <= 0; mult_start <= 0; mult_wr_en <= 0; mult_rd_en <= 0;
            mult_wr_addr <= 0; mult_rd_addr <= 0; mult_wr_A <= 0; mult_wr_B <= 0;
            valid_out_reg <= 0; fifo_rd_enable <= 0; fifo_wr_enable <= 0;
        end else begin
            
            // --- Lógica Padrão ---
            ntt_load_w <= 0; ntt_load_data <= 0; ntt_start <= 0; ntt_start_intt <= 0;
            mult_start <= 0; mult_wr_en <= 0; mult_rd_en <= 0;
            valid_out_reg <= 0; // Padrão
            fifo_rd_enable <= 0;
            fifo_wr_enable <= 0;
            
            case (state)
                // --- IDLE e Modos de Carga (Inalterados v7) ---
                STATE_IDLE: begin
                    done_all <= 0; busy <= 0;
                    if (start_transaction) begin
                        busy <= 1; cnt <= 0;
                        case (mode)
                            3'd0: state <= STATE_LOAD_W_PULSE;
                            3'd1: state <= STATE_LOAD_A_WAIT;
                            3'd2: state <= STATE_LOAD_B_WAIT;
                            3'd3: state <= STATE_GO_NTT_A_PULSE;
                            default: begin state <= STATE_IDLE; busy <= 0; end
                        endcase
                    end
                end
                STATE_LOAD_W_PULSE:  begin ntt_load_w <= 1; state <= STATE_LOAD_W_WAIT; end
                STATE_LOAD_W_WAIT:   begin
                    if (valid_in) begin 
                        ntt_din <= din; 
                        fifo_rd_enable <= 1;
                        state <= STATE_LOAD_W_STREAM; 
                    end 
                end
                STATE_LOAD_W_STREAM: begin 
                    if (valid_in) begin
                        ntt_din <= din;
                        fifo_rd_enable <= 1; // << "Estoura" o dado atual 
                    end else begin
                        state <= STATE_IDLE; 
                    end
                end
                STATE_LOAD_A_WAIT:   begin 
                    if (valid_in) begin 
                        ram_a[cnt] <= din; 
                        cnt <= cnt + 1; 
                        fifo_rd_enable <= 1; // << "Estoura" A[0]
                        state <= STATE_LOAD_A_STREAM; 
                    end 
                end
                STATE_LOAD_A_STREAM: begin 
                    if (valid_in) begin 
                        ram_a[cnt] <= din; 
                        cnt <= cnt + 1; 
                        fifo_rd_enable <= 1; // << "Estoura" A[1], A[2],...
                        if (cnt == `RING_SIZE - 1) state <= STATE_IDLE; // Terminou
                    end
                    // Se 'valid_in' for 0, espere (a FIFO está vazia, mas
                    // o C ainda não terminou de enviar o DMA)
                end
                
                STATE_LOAD_B_WAIT:   begin 
                    if (valid_in) begin 
                        ram_b[cnt] <= din; 
                        cnt <= cnt + 1; 
                        fifo_rd_enable <= 1; // << "Estoura" B[0]
                        state <= STATE_LOAD_B_STREAM; 
                    end 
                end
                STATE_LOAD_B_STREAM: begin 
                    if (valid_in) begin 
                        ram_b[cnt] <= din; 
                        cnt <= cnt + 1; 
                        fifo_rd_enable <= 1; // << "Estoura" B[1], B[2],...
                        if (cnt == `RING_SIZE - 1) state <= STATE_IDLE; // Terminou
                    end
                end
                // --- GO: Processa A (Corrigido v9) ---
                STATE_GO_NTT_A_PULSE:  begin ntt_load_data <= 1; state <= STATE_GO_NTT_A_STREAM; cnt <= 0; end
                STATE_GO_NTT_A_STREAM: begin
                    ntt_din <= ram_a[cnt];
                    if (cnt == `RING_SIZE - 1) state <= STATE_GO_NTT_A_BUBBLE;
                    else cnt <= cnt + 1;
                end
                STATE_GO_NTT_A_BUBBLE: begin state <= STATE_GO_NTT_A_START; end
                STATE_GO_NTT_A_START:  begin ntt_start <= 1; state <= STATE_GO_NTT_A_WAIT_DONE; cnt <= 0; end // Zera o cnt!
                STATE_GO_NTT_A_WAIT_DONE: begin
                    if (ntt_done) state <= STATE_GO_NTT_A_STREAM_OUT; // Pega o pulso
                end
                STATE_GO_NTT_A_STREAM_OUT: begin // Agora conta 256 ciclos
                    ram_c[cnt] <= ntt_dout; // Salva NTT(A)[0], [1], ...
                    if (cnt == `RING_SIZE - 1) state <= STATE_GO_NTT_B_PULSE;
                    else cnt <= cnt + 1;
                end

                // --- GO: Processa B (Corrigido v9) ---
                STATE_GO_NTT_B_PULSE:  begin ntt_load_data <= 1; state <= STATE_GO_NTT_B_STREAM; cnt <= 0; end
                STATE_GO_NTT_B_STREAM: begin
                    ntt_din <= ram_b[cnt];
                    if (cnt == `RING_SIZE - 1) state <= STATE_GO_NTT_B_BUBBLE;
                    else cnt <= cnt + 1;
                end
                STATE_GO_NTT_B_BUBBLE: begin state <= STATE_GO_NTT_B_START; end
                STATE_GO_NTT_B_START:  begin ntt_start <= 1; state <= STATE_GO_NTT_B_WAIT_DONE; cnt <= 0; end // Zera o cnt!
                STATE_GO_NTT_B_WAIT_DONE: begin
                    if (ntt_done) state <= STATE_GO_NTTB_STREAM_OUT; // Pega o pulso
                end
                STATE_GO_NTTB_STREAM_OUT: begin // Agora conta 256 ciclos
                    mult_wr_en <= 1; mult_wr_addr <= cnt[`RING_DEPTH-1:0];
                    mult_wr_A <= ram_c[cnt];  // Envia NTT(A)
                    mult_wr_B <= ntt_dout;  // Envia NTT(B)
                    if (cnt == `RING_SIZE - 1) begin cnt <= 0; state <= STATE_GO_MULT_EXEC; end
                    else cnt <= cnt + 1;
                end
                
                // --- GO: Multiplicação (Inalterado v7) ---
                STATE_GO_MULT_EXEC: begin
                    mult_wr_en <= 0;
                    if (cnt == 0) begin mult_start <= 1; cnt <= 1;
                    end else begin
                        if (mult_done) begin cnt <= 0; mult_rd_addr <= 0; state <= STATE_GO_MULT_READ; end
                    end
                end
                STATE_GO_MULT_READ: begin
                    mult_rd_en <= 1; mult_rd_addr <= cnt[`RING_DEPTH-1:0];
                    if (cnt >= 2) begin ram_c[bit_reverse_index(cnt-2)] <= mult_rd_data; end
                    cnt <= cnt + 1;
                    if (cnt == `RING_SIZE + 2) begin mult_rd_en <= 0; cnt <= 0; state <= STATE_GO_INTT_PULSE; end
                end
                
                // --- GO: INTT e Saída (Lógica v8) ---
                STATE_GO_INTT_PULSE:   begin ntt_load_data <= 1; state <= STATE_GO_INTT_STREAM; cnt <= 0; end
                STATE_GO_INTT_STREAM: begin
                    ntt_din <= ram_c[cnt];
                    if (cnt == `RING_SIZE - 1) state <= STATE_GO_INTT_BUBBLE;
                    else cnt <= cnt + 1;
                end
                STATE_GO_INTT_BUBBLE:  begin state <= STATE_GO_INTT_START; end

                STATE_GO_INTT_START:   begin ntt_start_intt <= 1; state <= STATE_GO_OUTPUT_WAIT; cnt <= 0; end

                STATE_GO_OUTPUT_WAIT:  begin
                    if (ntt_done) begin 
                        valid_out_reg <= 1; 
                        fifo_wr_enable <= 1 && !fifo_full;
                        state <= STATE_GO_OUTPUT_STREAM; 
                    end
                end
               STATE_GO_OUTPUT_STREAM: begin
                    valid_out_reg <= 1; // Mantém o anúncio
                    fifo_wr_enable <= 1 && !fifo_full;
                    
                    // Lógica de "Stall" (Pausa)
                    if (!fifo_full) begin
                        // A FIFO tem espaço. A escrita (via assign) acontece.
                        // Avance para o próximo dado.
                        if (cnt == `RING_SIZE - 1) begin // Se este foi o último
                            done_all <= 1; 
                            state <= STATE_IDLE; 
                            valid_out_reg <= 0; // Para de anunciar
                            fifo_wr_enable <= 0;
                        end else begin
                            cnt <= cnt + 1;
                        end
                    end
                    // else: A FIFO está cheia. Não faça nada.
                    // A FSM fica "travada" aqui, com 'cnt' parado,
                    // 'valid_out_reg' alto, até 'fifo_full' cair.
                end

            endcase
        end
    end

    // --- Instâncias e Assigns (Inalterados) ---
    NTTN uut_ntt (
        .clk(clk), .reset(reset), .load_w(ntt_load_w), .load_data(ntt_load_data),
        .start(ntt_start), .start_intt(ntt_start_intt), .din(ntt_din),
        .done(ntt_done), .dout(ntt_dout)
    );
    PolyPointwiseMult uut_polypointwise (
        .clk(clk), .reset(reset), .start(mult_start), .done(mult_done),
        .wr_en(mult_wr_en), .wr_addr(mult_wr_addr), .wr_A(mult_wr_A), .wr_B(mult_wr_B),
        .q(7681), .rd_en(mult_rd_en), .rd_addr(mult_rd_addr), .rd_data(mult_rd_data)
    );
    assign dout = ntt_dout;

endmodule