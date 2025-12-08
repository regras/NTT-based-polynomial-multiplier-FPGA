`include "defines.v"

module PolyPointwiseMult(
    input wire clk,
    input wire reset,

    // Controle de operação
    input wire start,
    output reg done,

    // Interface para escrita dos vetores A e B
    input wire wr_en,                                 // habilita escrita
    input wire [`RING_DEPTH-1:0] wr_addr,              // endereço do coeficiente
    input wire [`DATA_SIZE-1:0] wr_A, wr_B,            // valores para armazenar

    // Parâmetro de módulo
    input wire [`DATA_SIZE-1:0] q,
    input  wire rd_en,
    input  wire [`RING_DEPTH-1:0] rd_addr,
    output reg  [`DATA_SIZE-1:0] rd_data

);

    // Saída dos coeficientes resultante`
    reg [`DATA_SIZE-1:0] C [0:`RING_SIZE-1];

    // Memórias internas
    reg [`DATA_SIZE-1:0] A [0:`RING_SIZE-1];
    reg [`DATA_SIZE-1:0] B [0:`RING_SIZE-1];

    // Controle da FSM
    reg [`RING_DEPTH-1:0] idx;
    reg [2:0] state;
    reg busy;

    // Instância do multiplicador modular
    reg [`DATA_SIZE-1:0] modmult_A_in, modmult_B_in;
    wire [`DATA_SIZE-1:0] modmult_C_out;

    reg [`DATA_SIZE-1:0] result_reg;
    reg [2 * (`DATA_SIZE) - 1:0] product_reg;
    reg result_valid;

     // Definição simbólica dos estados
    localparam IDLE = 3'd0,
               WAIT = 3'd1,
               MULT = 3'd2,
               MODULO = 3'd3,
               SAVE = 3'd4,
               DONE = 3'd5;

    // ModMult uut_modmult (
    //     .clk(clk),
    //     .reset(reset),
    //     .A(modmult_A_in),
    //     .B(modmult_B_in),
    //     .q(q),
    //     .C(modmult_C_out)
    // );

    reg start_d;  // start atrasado 1 ciclo

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            start_d <= 0;
        end else begin
            start_d <= start;
        end
    end


    // Escrita de dados nas memórias A e B
    always @(posedge clk) begin
        if (wr_en) begin
            A[wr_addr] <= wr_A;
            B[wr_addr] <= wr_B;
        end
    end

    // FSM de multiplicação ponto a ponto
    always @(posedge clk or posedge reset) begin
        if (reset) begin
            idx <= 0;
            done <= 0;
            state <= IDLE;
            result_reg <= 0;
        end else begin
            case (state)
                IDLE: begin
                    done <= 0;
                    if (start) begin
                        idx <= 0;
                        state <= WAIT;
                    end // espera 1 ciclo após start
                end

                WAIT: begin
                    state <= MULT;
                end

                MULT: begin
                    // Multiplicação ponto a ponto
                    product_reg <= A[idx] * B[idx]; // O Verilog infere 32 bits
                    state <= MODULO;
                
                end

                MODULO: begin
                    if (q != 0)
                        result_reg <= product_reg % q;
                    else
                        result_reg <= product_reg;

                    $display("Ciclo %0t [MULT]: Lendo idx=%d, A=%h, B=%h, q=%h, result=%0d", $time, idx, A[idx], B[idx], q, (A[idx] * B[idx]) % q);
                    state <= SAVE;
                end
                
                SAVE: begin
                    C[idx] <= result_reg;

                    // Controle do índice
                    if (idx == (`RING_SIZE - 1)) begin
                        state <= DONE;
                    end else begin
                        idx <= idx + 1;
                        state <= MULT;
                    end
                end

                DONE: begin
                    done <= 1;
                    state <= IDLE; // volta a ficar pronto para próxima operação
                end

                default: begin
                    state <= IDLE;
                end
            endcase
        end
    end

    always @(posedge clk) begin
        // $display("rd_en", rd_en);
        if (rd_en)
            rd_data <= C[rd_addr];
    end

endmodule
