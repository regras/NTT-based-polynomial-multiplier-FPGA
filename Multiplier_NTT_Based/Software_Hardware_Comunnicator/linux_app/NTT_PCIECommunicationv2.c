#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "PCIE.h"
#include <time.h>
#include "NTT_Software/Generator_Params/generate_params.h"

// =========================================================================
// DEFINIÇÕES DE HARDWARE
// =========================================================================

// IDs das FIFOs (conforme definido no Platform Designer / QSys)
#define FIFO_IN_ID  0x40
#define FIFO_OUT_ID 0x80

// Endereços dos Registradores de Controle/Status (na BAR0)
#define ADDR_CONTROL 0x00 
#define ADDR_STATUS  0x20 

// Máscaras de bits para os registradores
#define CONTROL_START_BIT (1 << 0)
#define CONTROL_MODE_SHIFT 1

#define STATUS_BUSY_MASK  (1 << 0)
#define STATUS_DONE_MASK  (1 << 1)

// Parâmetros do 'defines.v'
#define N 256
#define RING_DEPTH 8 // ===> LOG(N)b'
#define PE_DEPTH 3 // ====> LOG(PE_NUMBER)b2 , PE_NUMBER = 8


#define W_COUNT ((((1<<(RING_DEPTH-PE_DEPTH))-1)+PE_DEPTH)<<PE_DEPTH)

#define DEMO_PCIE_USER_BAR			PCIE_BAR0
// =========================================================================
// FUNÇÕES HELPER (Abstração do Protocolo)
// =========================================================================

/**
 * @brief Envia um comando (pulsa 'start' com um 'mode') para a FSM.
 */
void SendCommand(PCIE_HANDLE hPCIe, int mode) {
    // Liga 'start' (bit 0) e 'mode' (bits [3:1])
    DWORD command = (mode << CONTROL_MODE_SHIFT) | CONTROL_START_BIT;
    PCIE_Write32(hPCIe, DEMO_PCIE_USER_BAR, ADDR_CONTROL, command);

    // Baixa 'start' (bit 0) no ciclo seguinte
    command = (mode << CONTROL_MODE_SHIFT);
    PCIE_Write32(hPCIe, DEMO_PCIE_USER_BAR, ADDR_CONTROL, command);
}

/**
 * @brief Fica em loop (polling) até o bit 'busy' da FSM cair para 0.
 */
BOOL WaitForBusyClear(PCIE_HANDLE hPCIe) {
    DWORD status = 0;

    long long timeout = 50 * 1000 * 1000; 
    
    printf("...aguardando 'busy' (FSM) ficar livre...\n");

    while (1) {
        PCIE_Read32(hPCIe, DEMO_PCIE_USER_BAR, ADDR_STATUS, &status);

        // Se busy == 0 → pronto
        if (!(status & STATUS_BUSY_MASK)) {
            return TRUE;
        }

        // Timeout para evitar loop infinito
        if (--timeout <= 0) {
            printf("ERRO: Timeout esperando 'busy' ficar baixo!\n");
            return FALSE;
        }
    }
}


/**
 * @brief Fica em loop (polling) até o bit 'done_all' da FSM subir para 1.
 */
BOOL WaitForDoneAll(PCIE_HANDLE hPCIe) {
    DWORD status = 0;

    long long timeout = 2500000;  

    printf("...aguardando 'done_all' (FSM) ficar alto...\n");

    while (1) {
        PCIE_Read32(hPCIe, DEMO_PCIE_USER_BAR, ADDR_STATUS, &status);

        // Se o bit subiu, retorna
        if (status & STATUS_DONE_MASK)
            return TRUE;

        // Evita loop infinito caso a FPGA trave
        if (--timeout <= 0) {
            printf("ERRO: Timeout esperando 'done_all'!\n");
            return FALSE;
        }
    }
}


// =========================================================================
// FUNÇÃO PRINCIPAL DE EXECUÇÃO
// =========================================================================
BOOL NTT_HARDWARE_EXE(PCIE_HANDLE hPCIe) {
    BOOL bPass = TRUE;
    char szError[256];

    // --- 1. Alocação de Memória ---

    uint32_t* params_stream = NULL;
    uint32_t* polyA = NULL;
    uint32_t* polyB = NULL;
    uint32_t* polyC = NULL;

    // Buffer único para o stream do Modo 0 (W + W_INV + q + n_inv)
    int stream0_count = (W_COUNT * 2) + 2;
    params_stream = (uint32_t*)malloc(stream0_count * sizeof(uint32_t));
    
    polyA = (uint32_t*)calloc(N, sizeof(uint32_t));
    polyB = (uint32_t*)calloc(N, sizeof(uint32_t));
    polyC = (uint32_t*)calloc(N, sizeof(uint32_t));

    if (!params_stream || !polyA || !polyB || !polyC) {
        fprintf(stderr, "Erro: falha na alocação de memória\n");
        bPass = FALSE;
        goto cleanup;
    }
    printf("Buffers de memória alocados.\n");

    // --- 2. Geração de Dados ---
    int psi=0, psi_inv=0, w=0, w_inv=0, R=0, n_inv=0, PE=0, q=0;
    generate_params(&psi, &psi_inv, &w, &w_inv, &R, &n_inv, &PE, &q);

    // Ponteiros para W e W_INV dentro do buffer de stream
    uint32_t* pW = params_stream;
    uint32_t* pW_INV = params_stream + W_COUNT;

    // printf("Aviso: Verifique se generate_twiddles() está preenchendo pW e pW_INV como uint32_t!\n");
    generate_twiddles(pW, pW_INV, w, w_inv, q, R);

    // Adiciona 'q' e 'n_inv' no final do stream
    params_stream[W_COUNT * 2]     = (uint32_t)q;
    params_stream[W_COUNT * 2 + 1] = (uint32_t)n_inv;

    // Gere polinômios A e B aleatórios
    // for(int i = 0; i < N; i++){
    //     polyA[i] = (uint32_t)(rand() % q);
    //     polyB[i] = (uint32_t)(rand() % q);
    // }
    
    // Para depuração, use valores conhecidos:
    polyA[0] = 1; polyA[1] = 2; polyA[2] = 3;
    polyB[0] = 2;
    
    printf("Dados de entrada gerados.\n");

    // --- Inicia Medição de Tempo ---
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // --- PASSO 1: Carregar W, W_inv e Params (MODO 0) ---
    printf("TB: Iniciando MODO 0 - Carga de W/Params.\n");
    SendCommand(hPCIe, 0); // Pulsa 'start' com 'mode=0'
    
    // Envia o stream inteiro (W, W_INV, q, n_inv) de uma vez via DMA
    bPass = PCIE_DmaFifoWrite(hPCIe, 
                              FIFO_IN_ID, 
                              params_stream, 
                              stream0_count * sizeof(uint32_t));
    if (!bPass) { sprintf(szError, "DMA Write (Modo 0) falhou\n"); goto cleanup; }
    
    bPass = WaitForBusyClear(hPCIe);
    if (!bPass) goto cleanup;
    printf("TB: MODO 0 Concluído.\n");

    // --- PASSO 2: Carregar Polinômio A (MODO 1) ---
    printf("TB: Iniciando MODO 1 - Carga de A (Bruto).\n");
    SendCommand(hPCIe, 1); // Pulsa 'start' com 'mode=1'
    
    bPass = PCIE_DmaFifoWrite(hPCIe, 
                              FIFO_IN_ID, 
                              polyA, 
                              N * sizeof(uint32_t));
    if (!bPass) { sprintf(szError, "DMA Write (Modo 1) falhou\n"); goto cleanup; }

    bPass = WaitForBusyClear(hPCIe);
    if (!bPass) goto cleanup;
    printf("TB: MODO 1 Concluído (A está na RAM interna).\n");

    // --- PASSO 3: Carregar Polinômio B (MODO 2) ---
    printf("TB: Iniciando MODO 2 - Carga de B (Bruto).\n");
    SendCommand(hPCIe, 2); // Pulsa 'start' com 'mode=2'
    
    bPass = PCIE_DmaFifoWrite(hPCIe, 
                              FIFO_IN_ID, 
                              polyB, 
                              N * sizeof(uint32_t));
    if (!bPass) { sprintf(szError, "DMA Write (Modo 2) falhou\n"); goto cleanup; }
                              
    bPass = WaitForBusyClear(hPCIe);
    if (!bPass) goto cleanup;
    printf("TB: MODO 2 Concluído (B está na RAM interna).\n");
    
    // --- PASSO 4: "GO!" (MODO 3) ---
    printf("TB: Iniciando MODO 3 - GO! (Executando A * B).\n");
    SendCommand(hPCIe, 3); // Pulsa 'start' com 'mode=3'
    
    // Espera a FSM sinalizar que a computação inteira terminou
    bPass = WaitForDoneAll(hPCIe);
    if (!bPass) goto cleanup;
    printf("TB: Sinal 'done_all' recebido!\n");
    
    // --- PASSO 5: Ler o Resultado ---
    printf("Lendo resultados da FIFO de Saída...\n");
    bPass = PCIE_DmaFifoRead(hPCIe,
                             FIFO_OUT_ID,
                             polyC,
                             N * sizeof(uint32_t));
    if (!bPass) { sprintf(szError, "DMA Read (polyC) falhou\n"); goto cleanup; }

    // --- Finaliza Medição de Tempo ---
    clock_gettime(CLOCK_MONOTONIC, &end);
    double durationNTT = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec)/1e9;
    printf("Tempo total: %.3f ms\n", durationNTT * 1000);

    // --- Verificação (Opcional, mas recomendado) ---
    printf("--- VERIFICAÇÃO DO RESULTADO C = A * B ---\n");
    // (A = 1+2x+3x^2, B = 2)
    int errors = 0;
    if (polyC[0] != 2) errors++;
    if (polyC[1] != 4) errors++;
    if (polyC[2] != 6) errors++;
    printf("Verificação: %d erros encontrados.\n", errors);

    // salvar_dados_txt("resultado_NTT.txt", (unsigned char*)polyC, N * sizeof(uint32_t));

cleanup:
    if (params_stream) free(params_stream);
    if (polyA) free(polyA);
    if (polyB) free(polyB);
    if (polyC) free(polyC);

    if (!bPass)
        printf("%s", szError);

    return bPass;
}

// =========================================================================
// MAIN
// =========================================================================
int main(void)
{
    
	void *lib_handle;
	PCIE_HANDLE hPCIE;
	BOOL bQuit = FALSE;
	int nSel;
    srand(time(NULL));

	printf("== Terasic: PCIe Demo Program (FIFO DMA) ==\r\n");

	lib_handle = PCIE_Load();
	if (!lib_handle){
		printf("PCIE_Load failed!\r\n");
		return 0;
	}

	hPCIE = PCIE_Open(0,0,0);
	if (!hPCIE){
		printf("PCIE_Open failed\r\n");
	} else {
        // Executa a NTT apenas 1 vez
    if(!NTT_HARDWARE_EXE(hPCIE)) { 
        printf("Execução NTT falhou\n");
     }
	}

    PCIE_Close(hPCIE);
	PCIE_Unload(lib_handle);
	return 0;
}