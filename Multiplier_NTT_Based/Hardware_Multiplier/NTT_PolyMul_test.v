`include "defines.v"

module NTT_PolyMul_test();

    parameter HP = 5;
    parameter FP = (2*HP);

    reg clk, reset;
    reg start_transaction;
    reg [2:0] mode;
    reg valid_in;
    reg [`DATA_SIZE_ARB-1:0] din;
    
    wire busy;
    wire done_all;
    wire valid_out;
    wire [`DATA_SIZE_ARB-1:0] dout;

    wire fifo_rd_enable; // "Fio" para a saída da FSM
    wire fifo_wr_enable; // "Fio" para a saída da FSM
    reg  fifo_full;

    // Memórias locais
    reg [`DATA_SIZE_ARB-1:0] params [0:7];
    reg [`DATA_SIZE_ARB-1:0] w      [0:((((1<<(`RING_DEPTH-`PE_DEPTH))-1)+`PE_DEPTH)<<`PE_DEPTH)-1];
    reg [`DATA_SIZE_ARB-1:0] winv   [0:((((1<<(`RING_DEPTH-`PE_DEPTH))-1)+`PE_DEPTH)<<`PE_DEPTH)-1];
    
    reg [`DATA_SIZE_ARB-1:0] final_c_output [0:`RING_SIZE-1];
    reg [`RING_DEPTH-1:0]    out_idx;
    integer k;
    integer errors;

    // =========================================================================
    // INSTANCIAÇÃO DO UUT
    // =========================================================================
    PolyMult_v2 uut (
        .clk(clk),
        .reset(reset),
        .start_transaction(start_transaction),
        .mode(mode),
        .busy(busy),
        .done_all(done_all),

        // Conexões da FIFO de Entrada
        .valid_in(valid_in),
        .din(din),
        .fifo_rd_enable(fifo_rd_enable), 

        // Conexões da FIFO de Saída
        .dout(dout),
        .valid_out(valid_out),
        .fifo_wr_enable(fifo_wr_enable),
        .fifo_full(fifo_full)
    );

    // Clock e Reset
    always #HP clk = ~clk;
    initial begin
        clk = 0; reset = 1; #200; reset = 0;
    end

    // Carregar arquivos
    initial begin
        $readmemh("test/PARAM.txt", params);
        $readmemh("test/W.txt"    , w);
        $readmemh("test/WINV.txt" , winv);
    end

    // =========================================================================
    // ESTÍMULO
    // =========================================================================
    initial begin
        start_transaction <= 0;
        mode <= 0; valid_in <= 0; din <= 0;
        errors = 0; out_idx = 0;
        fifo_full <= 0;

        @(negedge reset);
        $display("TB: Reset concluído.");
        #(10*FP);

        // --- Carrega W, W_inv e Params (MODO 0) ---
        $display("TB: Iniciando MODO 0 - Carga de W/Params.");
        start_transaction <= 1; mode <= 3'd0;
        @(posedge clk);
        start_transaction <= 0;

        // Envia W
        for(k=0; k<((((1<<(`RING_DEPTH-`PE_DEPTH))-1)+`PE_DEPTH)<<`PE_DEPTH); k=k+1) begin
            @(posedge clk); din <= w[k]; valid_in <= 1;
        end
        // Envia W_inv
        for(k=0; k<((((1<<(`RING_DEPTH-`PE_DEPTH))-1)+`PE_DEPTH)<<`PE_DEPTH); k=k+1) begin
            @(posedge clk); din <= winv[k]; valid_in <= 1; 
        end
        @(posedge clk)
        // Envia q (params[1])
        din <= params[1]; valid_in <= 1; @(posedge clk);
        // Envia n_inv*R (params[6])
        din <= params[6]; valid_in <= 1; @(posedge clk);
        
        valid_in <= 0; din <= 0;
        
        $display("TB: Stream de W enviado. Aguardando (busy=0).");
        @(negedge busy);
        $display("TB: MODO 0 Concluído.");
        #(10*FP);

        // --- Carrega Polinômio A (MODO 1) ---
        $display("TB: Iniciando MODO 1 - Carga de A (Bruto) = 1 + 2x + 3x^2.");
        @(posedge clk);
        start_transaction <= 1; mode <= 3'd1;
        @(posedge clk);
        start_transaction <= 0;

        for(k=0; k<(`RING_SIZE); k=k+1) begin
            if(k == 0)      din <= 1;
            else if(k == 1) din <= 2;
            else if(k == 2) din <= 3;
            else            din <= 0;
            valid_in <= 1;
            @(posedge clk);
        end
        valid_in <= 0; din <= 0;

        $display("TB: Polinômio A enviado. Aguardando (busy=0).");
        @(negedge busy);
        $display("TB: MODO 1 Concluído (A está na RAM interna).");
        #(10*FP);

        // --- Carregaa Polinômio B (MODO 2) ---
        $display("TB: Iniciando MODO 2 - Carga de B (Bruto) = 2 + 2x");
        @(posedge clk);
        start_transaction <= 1; mode <= 3'd2;
        @(posedge clk);
        start_transaction <= 0;

        for(k=0; k<(`RING_SIZE); k=k+1) begin
            if(k == 0) din <= 2;
            else if(k==1) din = 2;
            else       din <= 0;
            valid_in <= 1;
            @(posedge clk);
        end
        valid_in <= 0; din <= 0;

        $display("TB: Polinômio B enviado. Aguardando (busy=0).");
        @(negedge busy);
        $display("TB: MODO 2 Concluído (B está na RAM interna).");
        #(10*FP);
        
        // ---  "GO!" (MODO 3) ---
        $display("TB: Iniciando MODO 3 - GO! (Executando A * B).");
        @(posedge clk);
        start_transaction <= 1; mode <= 3'd3;
        @(posedge clk);
        start_transaction <= 0;
        
        $display("TB: Comando 'GO' enviado. Aguardando processamento completo (done_all=1)...");
        @(posedge done_all);
        
        $display("TB: Sinal 'done_all' recebido!");
        #(10*FP); 

        // --- Verificação dos Resultados ---
        $display("--- VERIFICAÇÃO DO RESULTADO C = A * B ---");
        // A = 1 + 2x + 3x^2
        // B = 2 + 2x
        // C = 2 + 6x + 10x^2 + 6xˆ3

        for(k = 0; k < (`RING_SIZE); k = k +1) begin
            $display("C[0] = %0d", final_c_output[k]);
        end
        
        if (final_c_output[0] == 2) $display("C[0] = %0d -> CORRETO", final_c_output[0]);
        else begin $display("C[0] = %0d -> INCORRETO (Esperado: 2)", final_c_output[0]); errors = errors + 1; end

        if (final_c_output[1] == 6) $display("C[1] = %0d -> CORRETO", final_c_output[1]);
        else begin $display("C[1] = %0d -> INCORRETO (Esperado: 6", final_c_output[1]); errors = errors + 1; end
        
        if (final_c_output[2] == 10) $display("C[2] = %0d -> CORRETO", final_c_output[2]);
        else begin $display("C[2] = %0d -> INCORRETO (Esperado: 10)", final_c_output[2]); errors = errors + 1; end

        if (final_c_output[3] == 6) $display("C[2] = %0d -> CORRETO", final_c_output[3]);
        else begin $display("C[2] = %0d -> INCORRETO (Esperado: 6)", final_c_output[3]); errors = errors + 1; end

        for (k = 4; k < `RING_SIZE; k = k + 1) begin
            if (final_c_output[k] != 0) begin
                $display("C[%0d] = %0d -> INCORRETO (Esperado: 0)", k, final_c_output[k]); errors = errors + 1;
                if (errors > 10) k = `RING_SIZE; // Sai do loop
            end
        end

        if (errors == 0) $display("==================== TESTE PASSOU! ====================");
        else $display("==================== TESTE FALHOU! (%0d erros) ====================", errors);

        $stop();
    end


    // =========================================================================
    // CAPTURA DE SAÍDA
    // =========================================================================
    function [`RING_DEPTH-1:0] bit_reverse_index;
        input [`RING_DEPTH-1:0] x;
        integer i;
        begin
            for (i = 0; i < (`RING_DEPTH); i = i + 1)
                bit_reverse_index[i] = x[`RING_DEPTH-1-i];
        end
    endfunction
    
    always @(posedge clk or posedge reset) begin
        if (reset) begin
            out_idx <= 0;
        end 
        else if (done_all) begin
            out_idx <= 0;
        end
        // Captura o dado APENAS quando a FSM der a permissão de escrita (e a FIFO não estiver cheia).
        else if (fifo_wr_enable) begin 
            final_c_output[bit_reverse_index(out_idx)] <= dout;
            out_idx <= out_idx + 1;
        end
    end

endmodule