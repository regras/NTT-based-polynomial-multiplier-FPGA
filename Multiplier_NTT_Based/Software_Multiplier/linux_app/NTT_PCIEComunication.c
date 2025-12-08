#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "PCIE.h"
#include <time.h>
#include <generate_params.h>


#define DEMO_PCIE_USER_BAR			PCIE_BAR0
#define DEMO_PCIE_IO_LED_ADDR		0x00
#define DEMO_PCIE_IO_BUTTON_ADDR	0x20
#define DEMO_PCIE_FIFO_WRITE_ADDR	0x40
#define DEMO_PCIE_FIFO_STATUS_ADDR	0x60
#define DEMO_PCIE_FIFO_READ_ADDR	0x80
#define DEMO_PCIE_MEM_ADDR			0x20000

#define MEM_SIZE			(128*1024) //128KB
#define FIFO_SIZE			(16*1024) // 2KBx8

// O endereco da nossa RAM On-Chip, baseado no mapa de enderecos do QSys (BAR 1)
#define TEST_RAM_ADDR	0x10000

// NTT Parameters
#define PCIE_BAR0		PCIE_BAR0
#define PCIE_BAR1		PCIE_BAR1

#define RAM_ADDR_POLYA	0x00000
#define RAM_ADDR_POLYB	0x10000
#define RAM_ADDR_POLYC	0x20000

#define N 256 // Mesmo valor RING_SIZE em defines.v
#define K 13 // Quantida de bits do primo 'q'

#define PARAM_COUNT 8 // Quantidade de parâmetros a ser transferido para o barramento e ser utilizada pelo verilog



void gerador_rand_Poly(const char *nome_arquivo){
    FILE *fp = fopen("POLY_A_HEX.txt", "w");
    if (!fp) {
        perror("Erro ao abrir o arquivo");
        return;
    }

    for (int i = 0; i < N; i++) {
        A[i] = rand() % q;  // gera número aleatório entre 0 e q-1
        fprintf(fp, "%x\n", A[i]);  // escreve em hexadecimal sem 0x
    }

    fclose(fp);
    printf("Arquivo gerado com sucesso!\n");
    return;   
}


void salvar_dados_txt(const char *nome_arquivo, unsigned char *dados, size_t tamanho_bytes) {
    FILE *fp = fopen(nome_arquivo, "w");
    if (!fp) {
        perror("Erro ao abrir arquivo");
        return;
    }

    // grava cada byte em hexadecimal
    for (size_t i = 0; i < tamanho_bytes; i++) {
        fprintf(fp, "%02X\n", dados[i]);  // ou "%d\n" para decimal
    }

    fclose(fp);
    printf("Arquivo '%s' salvo com sucesso (%zu bytes)\n", nome_arquivo, tamanho_bytes);
}


const PCIE_LOCAL_ADDRESS LocalAddrParams = 0x40000;
const PCIE_LOCAL_ADDRESS LocalAddrW = 0x60000;
const PCIE_LOCAL_ADDRESS LocalAddrW_INV = 0x80000;
const PCIE_LOCAL_ADDRESS LocalAddrA = 0x100000;
const PCIE_LOCAL_ADDRESS LocalAddrB = 0x102000;
const PCIE_LOCAL_ADDRESS LocalAddrC = 0x104000;


BOOL NTT_HARDWARE_EXE(PCIE_HANDLE hPCIe){
    BOOL bPass = TRUE;
    char szError[256];

    // Inicializa valores
    uint32_t* params = NULL;
    uint64_t* W = NULL;
    uint64_t* W_INV = NULL;
    uint32_t* polyA = NULL;
    uint32_t* polyB = NULL;
    uint32_t* polyC = NULL;


    // Aloca memória para os vetores
    params =    (uint32_t*)malloc(PARAM_COUNT * sizeof(uint32_t));
    W      =    (uint64_t*)malloc(2 * N * sizeof(uint64_t));
    W_INV  =    (uint64_t*)malloc(2 * N * sizeof(uint64_t));
    polyA  =    (uint32_t*)malloc(N * sizeof(uint32_t));
    polyB  =    (uint32_t*)malloc(N * sizeof(uint32_t));
    polyC  =    (uint32_t*)malloc(N * sizeof(uint32_t));

	
    if (!params || !W || !W_INV || !polyA || !polyB || !polyC) {
        fprintf(stderr, "Erro: falha na alocação de memória\n");
        bPass = FALSE;
        goto cleanup;
    }

    // Inicializa os parâmetros
    int psi=0, psi_inv=0, w=0, w_inv=0, R=0, n_inv=0, PE=0, q=0;
    generate_params(&psi, &psi_inv, &w, &w_inv, &R, &n_inv, &PE, &q);

    //Inicializa um array com os valores de parâmetros
    params[0] = (uint32_t) N;
    params[1] = (uint32_t) q;
    params[2] = (uint32_t) w;
    params[3] = (uint32_t) w_inv;
    params[4] = (uint32_t) psi;
    params[5] = (uint32_t) psi_inv;
    params[6] = (uint32_t) n_inv;
    params[7] = (uint32_t) R;
    
    //Inicializa os valores de twiddles factors
    generate_twiddles((uint64_t*)W, (uint64_t*)W_INV, (uint64_t)w, (uint64_t)w_inv, (uint64_t)q, (uint64_t)R);

    // Inicializa vetores com coeficientes aleatórios
    for(int i = 0; i < N; i++){
        polyA[i] = (uint32_t)rand() % q;
        polyB[i] = (uint32_t)rand() % q;
    }

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start); // Inicia o contador de tempo

    //write W
    bPass = PCIE_DmaWrite(hPCIe, LocalAddrW, W, 2 * N * sizeof(uint64_t));
    if (!bPass) {
        sprintf(szError, "DMA Write W failed\r\n");
        goto cleanup;
    }

    //write W_INV
    bPass = PCIE_DmaWrite(hPCIe, LocalAddrW_INV, W_INV, 2 * N * sizeof(uint64_t));
    if (!bPass) {
        sprintf(szError, "DMA Write W_INV failed\r\n");
        goto cleanup;
    }
    
    //write params
    bPass = PCIE_DmaWrite(hPCIe, LocalAddrParams, params, PARAM_COUNT * sizeof(uint32_t));
    if (!bPass) {
        sprintf(szError, "DMA Write params failed\r\n");
        goto cleanup;
    }

    // Write poly A
    bPass = PCIE_DmaWrite(hPCIe, LocalAddrA, polyA, N * sizeof(uint32_t));
    if (!bPass) {
        sprintf(szError, "DMA Write PolyA failed\r\n");
        goto cleanup;
    }

    // Write poly B
    bPass = PCIE_DmaWrite(hPCIe, LocalAddrB, polyB, N * sizeof(uint32_t));
    if (!bPass) {
        sprintf(szError, "DMA Write PolyB failed\r\n");
        goto cleanup;
    }

    // Inicia NTT no hardware
    DWORD initNTT = 0x1;
    DWORD doneNTT = 0;

    bPass = PCIE_Write32(hPCIe, DEMO_PCIE_USER_BAR, TEST_RAM_ADDR, initNTT);
    if (!bPass){
        sprintf(szError, "Erro: PCIE_Write32 falhou!\n");
        goto cleanup;
    }

    // Espera o hardware terminar
    int timeout = 1000000;
    printf("Executando NTT no hardware...\n");
    do {
        PCIE_Read32(hPCIe, DEMO_PCIE_USER_BAR, TEST_RAM_ADDR, &doneNTT);
        timeout--;
        if(timeout <= 0){
            printf("TIMEOUT: NTT não terminou!\n");
            break;
        }
    } while (doneNTT == 0);

    printf("NTT concluída!\n");

    // Lê o resultado
    bPass = PCIE_DmaRead(hPCIe, LocalAddrC, polyC, N * sizeof(uint32_t));
    if (!bPass){
        sprintf(szError, "DMA Read PolyC failed\r\n");
        goto cleanup;
    }

	clock_gettime(CLOCK_MONOTONIC, &end); // termina o contador de tempo
	double durationNTT = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec)/1e9; //medição da duração
	printf("Tempo total: %.3f ms\n", durationNTT * 1000);


    // Salva resultado
    salvar_dados_txt("resultado_NTT.txt", (unsigned char*)polyC, N);

cleanup:
    if (params) free(params);
    if (W) free(W);
    if (W_INV) free(W_INV);
    if (polyA) free(polyA);
    if (polyB) free(polyB);
    if (polyC) free(polyC);

    if (!bPass)
        printf("%s", szError);

    return bPass;
}



int main(void)
{
	void *lib_handle;
	PCIE_HANDLE hPCIE;
	BOOL bQuit = FALSE;
	int nSel;
    srand(time(NULL));  // inicializa gerador de números aleatórios

	printf("== Terasic: PCIe Demo Program ==\r\n");

	lib_handle = PCIE_Load();
	if (!lib_handle){
		printf("PCIE_Load failed!\r\n");
		return 0;
	}

	hPCIE = PCIE_Open(0,0,0);
	if (!hPCIE) {
		printf("PCIE_Open failed\r\n");
	} else {
		// while(!bQuit){ // Executa a NTT várias vezes(definido no loop)
		// 	NTT_HARDWARE_EXE(hPCIE);
		// }

        if(!NTT_HARDWARE_EXE(hPCIE)) { //Executa a NTT apenas 1 vez
            printf("Execução NTT falhou\n");
        }
	}

    PCIE_Close(hPCIE);
	PCIE_Unload(lib_handle);
	return 0;
}
