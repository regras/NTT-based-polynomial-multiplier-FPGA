#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include "PCIE.h"

// --- Endereços Baseados no Exemplo da Terasic ---
// NOTA: Verifique no QSys se os componentes estão no BAR0 ou BAR1.
// Geralmente na DE2i-150, PIOs e FIFOs ficam no BAR1.
// Se não funcionar, mude para PCIE_BAR0.
#define DEMO_PCIE_USER_BAR			PCIE_BAR0 

#define DEMO_PCIE_IO_LED_ADDR		0x00
#define DEMO_PCIE_IO_BUTTON_ADDR	0x20
#define DEMO_PCIE_FIFO_WRITE_ADDR	0x40
#define DEMO_PCIE_FIFO_STATUS_ADDR	0x60
#define DEMO_PCIE_FIFO_READ_ADDR	0x80

#define FIFO_SIZE			(4*1024) // 4KB para teste rápido

typedef enum{
	MENU_LED = 0,
	MENU_BUTTON,
	MENU_DMA_MEMORY,
	MENU_DMA_FIFO,
	MENU_LOOPBACK_PIO, 
	MENU_LOOPBACK_DMA,
    MENU_LOOPBACK_PIO_FLOW,  // Teste de Streaming (FIFO In -> FIFO Out)
	MENU_QUIT = 99
} MENU_ID;

// Define o numero de palavras (32-bit) a serem transferidas no teste PIO
#define PIO_TEST_WORDS (16) 
#define PIO_TEST_SIZE (PIO_TEST_WORDS * 4) // Tamanho em bytes

BOOL TEST_PIO_FIFO_FLOW(PCIE_HANDLE hPCIe) {
    BOOL bPass = TRUE;
    int i;
    DWORD val_to_write;
    DWORD val_read = 0;
    char szError[256];

    printf("\n--- Iniciando Teste PIO de Fluxo de Dados (Word-by-Word) ---\r\n");
    printf("Transferindo %d palavras (32-bit) via PIO...\r\n", PIO_TEST_WORDS);

    // O teste PIO nao usa os enderecos de controle (0x40/0x80)
    // Este programa é um teste de comunicação com o SGDMA  inativo
    const PCIE_LOCAL_ADDRESS FIFO_WRITE_DATA_ADDR = DEMO_PCIE_FIFO_WRITE_ADDR; 
    const PCIE_LOCAL_ADDRESS FIFO_READ_DATA_ADDR = DEMO_PCIE_FIFO_READ_ADDR; 

    // Buffer para armazenar os dados lidos (para facilitar a verificacao)
    DWORD *pReadBack = (DWORD *)malloc(PIO_TEST_SIZE);
    if (!pReadBack) {
        printf("ERRO: Falha na alocacao de memoria.\r\n");
        return FALSE;
    }
    
    // 1. ESCREVER (Host -> FIFO 0)
    for (i = 0; i < PIO_TEST_WORDS; i++) {
        val_to_write = (DWORD)i; // Padrão simples: 0, 1, 2, 3...
        
        // Escreve uma palavra no endereco de dados da FIFO de Escrita
        if (!PCIE_Write32(hPCIe, DEMO_PCIE_USER_BAR, FIFO_WRITE_DATA_ADDR, val_to_write)) {
            sprintf(szError, "ERRO: Falha na escrita PIO na palavra %d.\r\n", i);
            bPass = FALSE;
            break;
        }
    }
    
    // Se a escrita falhou, sai
    if (!bPass) {
        printf("%s", szError);
        free(pReadBack);
        return FALSE;
    }

    // 2. LER (FIFO 1 -> Host)
    bPass = TRUE; // Reseta a flag de sucesso
    for (i = 0; i < PIO_TEST_WORDS; i++) {
        val_read = 0;
        
        // Lê uma palavra do endereco de dados da FIFO de Leitura
        if (!PCIE_Read32(hPCIe, DEMO_PCIE_USER_BAR, FIFO_READ_DATA_ADDR, &val_read)) {
            sprintf(szError, "ERRO: Falha na leitura PIO na palavra %d.\r\n", i);
            bPass = FALSE;
            break;
        }
        pReadBack[i] = val_read;
    }
    
    // 3. VERIFICAR
    if (!bPass) {
        printf("%s", szError);
    } else {
        int errors = 0;
        for (i = 0; i < PIO_TEST_WORDS; i++) {
            DWORD expected = (DWORD)i;
            if (pReadBack[i] != expected) {
                if (errors == 0) printf("\nDetalhes dos Erros:\r\n");
                printf("  ERRO: Palavra %d: Esperado 0x%08X, Lido 0x%08X\r\n", i, expected, pReadBack[i]);
                errors++;
            }
        }
        
        if (errors == 0) {
            printf("SUCESSO! Loopback PIO de fluxo de dados verificado.\r\n");
        } else {
            printf("\nFALHA! Total de %d palavras incorretas.\r\n", errors);
            bPass = FALSE;
        }
    }

    free(pReadBack);
    return bPass;
}

void UI_ShowMenu(void){
	printf("==============================\r\n");
	printf("[%d]: Led control (Original)\r\n", MENU_LED);
	printf("[%d]: Button Status Read (Original)\r\n", MENU_BUTTON);
	printf("[%d]: DMA Memory Test (Original)\r\n", MENU_DMA_MEMORY);
	printf("[%d]: DMA Fifo Test (Original)\r\n", MENU_DMA_FIFO);
    printf("--- Testes de Loopback (Hardware Modificado) ---\r\n");
	printf("[%d]: Loopback PIO (Escreve LED -> Le Button)\r\n", MENU_LOOPBACK_PIO);
	printf("[%d]: Loopback DMA (Escreve FIFO -> Le FIFO)\r\n", MENU_LOOPBACK_DMA);
    printf("[%d]: Loopback PIO Flow (Word-by-Word)\r\n", MENU_LOOPBACK_PIO_FLOW);
	printf("[%d]: Quit\r\n", MENU_QUIT);
	printf("Please input your selection:");
}

int UI_UserSelect(void){
	int nSel;
	scanf("%d",&nSel);
	return nSel;
}

// Funções Originais 
BOOL TEST_LED(PCIE_HANDLE hPCIe){
	BOOL bPass;
	int	Mask;
	printf("Please input led control mask:");
	scanf("%d", &Mask);
	bPass = PCIE_Write32(hPCIe, DEMO_PCIE_USER_BAR, DEMO_PCIE_IO_LED_ADDR,(DWORD)Mask);
	if (bPass) printf("Led control success, mask=%xh\r\n", Mask);
	else printf("Led control failed\r\n");
	return bPass;
}

BOOL TEST_BUTTON(PCIE_HANDLE hPCIe){
	BOOL bPass = TRUE;
	DWORD Status;
	bPass = PCIE_Read32(hPCIe, DEMO_PCIE_USER_BAR, DEMO_PCIE_IO_BUTTON_ADDR,&Status);
	if (bPass) printf("Button status mask=%xh\r\n", Status);
	else printf("Failed to read button status\r\n");
	return bPass;
}

char PAT_GEN(int nIndex){
	char Data;
	Data = nIndex & 0xFF;
	return Data;
}

// =================================================================
// ============== FUNÇÕES DE TESTE LOOPBACK ================
// =================================================================

BOOL TEST_LOOPBACK_PIO(PCIE_HANDLE hPCIe){
    BOOL bPass = TRUE;
    DWORD val_to_write = 0x03; // Bits 0 e 1 ligados (Done e Busy no Verilog)
    DWORD val_read = 0;

    printf("\n--- Iniciando Teste Loopback PIO ---\n");
    printf("Escrevendo 0x%02X no endereco LED (0x%X)...\n", val_to_write, DEMO_PCIE_IO_LED_ADDR);

    // 1. Escreve no Registrador de Controle (LED)
    if (!PCIE_Write32(hPCIe, DEMO_PCIE_USER_BAR, DEMO_PCIE_IO_LED_ADDR, val_to_write)){
        printf("ERRO: Falha na escrita PIO.\n");
        return FALSE;
    }

    // Pequeno delay para garantir propagação no hardware
    usleep(1000);

    // 2. Lê do Registrador de Status (Button)
    if (!PCIE_Read32(hPCIe, DEMO_PCIE_USER_BAR, DEMO_PCIE_IO_BUTTON_ADDR, &val_read)){
        printf("ERRO: Falha na leitura PIO.\n");
        return FALSE;
    }

    // Mascara para pegar apenas os bits que nos interessam (bits 0 a 3)
    val_read &= 0xF; 

    printf("Lido do endereco BUTTON: 0x%02X\n", val_read);

    if (val_read == val_to_write) {
        printf("SUCESSO: O valor lido e igual ao escrito.\n");
    } else {
        printf("FALHA: Esperado 0x%02X, Recebido 0x%02X\n", val_to_write, val_read);
        bPass = FALSE;
    }
    return bPass;
}

BOOL TEST_LOOPBACK_DMA(PCIE_HANDLE hPCIe){
    BOOL bPass = TRUE;
    int i;
    const int nTestSize = FIFO_SIZE;
    char *pBuffWrite;
    char *pBuffRead;
    char szError[256];

    printf("\n--- Iniciando Teste Loopback DMA (FIFO Echo) ---\n");

    pBuffWrite = (char *)malloc(nTestSize);
    pBuffRead  = (char *)malloc(nTestSize);

    if (!pBuffWrite || !pBuffRead){
        printf("Erro de alocacao de memoria.\n");
        return FALSE;
    }

    // 1. Gerar Padrão de Dados (0, 1, 2, 3...)
    for(i=0; i<nTestSize; i++) {
        *(pBuffWrite+i) = PAT_GEN(i);
        *(pBuffRead+i)  = 0; // Limpa buffer de leitura
    }

    // 2. Escrever na FIFO de Entrada (Host -> FPGA)
    printf("Enviando %d bytes para FIFO Write (0x%X)...\n", nTestSize, DEMO_PCIE_FIFO_WRITE_ADDR);
    bPass = PCIE_DmaFifoWrite(hPCIe, DEMO_PCIE_FIFO_WRITE_ADDR, pBuffWrite, nTestSize);
    
    if (!bPass) {
        sprintf(szError, "ERRO: PCIE_DmaFifoWrite falhou.\n");
    } else {
        // 3. Ler da FIFO de Saída (FPGA -> Host)
        // Com o Verilog em loopback, o dado esta lá imediatamente.
        printf("Lendo %d bytes da FIFO Read (0x%X)...\n", nTestSize, DEMO_PCIE_FIFO_READ_ADDR);
        bPass = PCIE_DmaFifoRead(hPCIe, DEMO_PCIE_FIFO_READ_ADDR, pBuffRead, nTestSize);

        if (!bPass) {
            sprintf(szError, "ERRO: PCIE_DmaFifoRead falhou (Timeout ou FIFO vazia).\n");
        } else {
            // 4. Comparar Buffers
            int err_count = 0;
            for(i=0; i<nTestSize && bPass; i++){
                if (*(pBuffRead+i) != *(pBuffWrite+i)){
                    bPass = FALSE;
                    if(err_count < 5) { // Mostra apenas os primeiros erros
                        sprintf(szError, "Erro de Verificacao! Index=%d, Escrito=%02Xh, Lido=%02Xh\r\n", 
                                i, (unsigned char)*(pBuffWrite+i), (unsigned char)*(pBuffRead+i));
                        printf("%s", szError);
                    }
                    err_count++;
                }
            }
            if(err_count > 0) printf("Total de erros: %d\n", err_count);
        }
    }

    if (pBuffWrite) free(pBuffWrite);
    if (pBuffRead)  free(pBuffRead);

    if (bPass) printf("SUCESSO: Loopback DMA verificado.\n");
    else printf("FALHA no teste DMA.\n");

    return bPass;
}


BOOL TEST_DMA_MEMORY(PCIE_HANDLE hPCIe){
    return TRUE; 
}

BOOL TEST_DMA_FIFO(PCIE_HANDLE hPCIe){
    return TRUE;
}

int main(void)
{
	void *lib_handle;
	PCIE_HANDLE hPCIE;
	BOOL bQuit = FALSE;
	int nSel;

	printf("== Terasic: PCIe Demo Program (Modified for Loopback) ==\r\n");

	lib_handle = PCIE_Load();
	if (!lib_handle){
		printf("PCIE_Load failed!\r\n");
		return 0;
	}

	// Tenta abrir o dispositivo 0
	hPCIE = PCIE_Open(0,0,0);
	if (!hPCIE){
		printf("PCIE_Open failed\r\n");
	}else{
		while(!bQuit){
			UI_ShowMenu();
			nSel = UI_UserSelect();
			switch(nSel){	
				case MENU_LED:
					TEST_LED(hPCIE);
					break;
				case MENU_BUTTON:
					TEST_BUTTON(hPCIE);
					break;
				case MENU_DMA_MEMORY:
					printf("Skipping original memory test.\n");
					break;
				case MENU_DMA_FIFO:
					printf("Skipping original FIFO test. Use Loopback DMA.\n");
					break;
				
				case MENU_LOOPBACK_PIO:
					TEST_LOOPBACK_PIO(hPCIE);
					break;
				case MENU_LOOPBACK_DMA:
					TEST_LOOPBACK_DMA(hPCIE);
					break;
                case MENU_LOOPBACK_PIO_FLOW:
                    TEST_PIO_FIFO_FLOW(hPCIE);
                    break;

				case MENU_QUIT:
					bQuit = TRUE;
					printf("Bye!\r\n");
					break;
				default:
					printf("Invalid selection\r\n");
			} 
		}
		PCIE_Close(hPCIE);
	}

	PCIE_Unload(lib_handle);
	return 0;
}