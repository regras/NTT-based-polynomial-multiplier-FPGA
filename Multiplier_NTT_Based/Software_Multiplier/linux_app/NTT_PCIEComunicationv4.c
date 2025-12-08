#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "PCIE.h" 
#include <time.h>
#include <unistd.h>
#include <stdint.h>

// === Tipos de Dados e Definições (Mantidas) ===

#ifndef BOOL
#define BOOL int
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef DWORD
typedef unsigned int DWORD;
#endif


extern void generate_params(int *psi, int *psi_inv, int *w, int *w_inv, int *R, int *n_inv, int *PE, int *q);
// extern void generate_twiddles(uint32_t* pW, uint32_t* pW_INV, int w, int w_inv, int q, int R);

// ... (Restante das definições de endereços SGDMA/RAM/Wrapper) ...

// =========================================================================
// DEFINIÇÕES DE HARDWARE (AJUSTAR NO SEU QSYS)
// =========================================================================

// Os endereços abaixo DEVEM ser ajustados conforme o mapa de endereços do seu QSys.
#define DEMO_PCIE_USER_BAR      PCIE_BAR0

// Base do Wrapper (Controle/Status do Verilog)
#define WRAPPER_BASE_ADDR       0x1000 
#define ADDR_CONTROL            (WRAPPER_BASE_ADDR + 0x00) // 4'h0 * 4 bytes/palavra
#define ADDR_STATUS             (WRAPPER_BASE_ADDR + 0x04) // 4'h4 * 4 bytes/palavra

// Offsets dos CSRs da SGDMA (Control and Status Registers)
#define SGDMA_TX_CSR_ADDR       0x4040 // Exemplo: Base do CSR do SGDMA_MEM_TO_STREAM
#define SGDMA_RX_CSR_ADDR       0x4000 // Exemplo: Base do CSR do SGDMA_STREAM_TO_MEM

// Endereços na RAM On-Chip (acessível via PCIe BAR0)
#define ONCHIP_RAM_BASE_ADDR    0x20000 // Exemplo: Base da sua On-Chip RAM
#define ONCHIP_RAM_DESC_BASE_ADDR   0x60000

// Definições de buffers na RAM On-Chip (Offsets a partir da ONCHIP_RAM_BASE_ADDR)
#define SGDMA_NUM_DESCRIPTORS   1
#define SGDMA_DESC_SIZE         32 
#define DATA_WORD_SIZE          4 // sizeof(uint32_t)

#define TX_DESC_ADDR            0x00060000U  // descritor TX
#define RX_DESC_ADDR            0x00060020U  // descritor RX

#define TX_TEST_BUFFER_ADDR  0x00060100U  // buffer TX (memória -> stream)
#define RX_TEST_BUFFER_ADDR  0x00060200U 

#define TEST_LENGTH_BYTES    (4 * sizeof(DWORD))

#define PARAMS_STREAM_ADDR      (ONCHIP_RAM_BASE_ADDR + 0x1000) 
#define POLY_A_ADDR             (ONCHIP_RAM_BASE_ADDR + 0x4000) 
#define POLY_B_ADDR             (ONCHIP_RAM_BASE_ADDR + 0x8000) 
#define POLY_C_ADDR             (ONCHIP_RAM_BASE_ADDR + 0xC000) 


// Máscaras e Offsets SGDMA
#define SGDMA_CSR_CONTROL       0x00 
#define SGDMA_CSR_STATUS        0x04 
#define SGDMA_CSR_NEXT_DESC     0x08 

#define SGDMA_CONTROL_RUN_MASK  0x00000001
#define SGDMA_STATUS_DONE_MASK  0x00000002
#define SGDMA_DESCRIPTOR_EOP_MASK 0x80000000




// ---- BUFFERS ----
#define TX_TEST_BUFFER_ADDR   0x20000
#define RX_TEST_BUFFER_ADDR   0x20020   // 32 bytes after TX

// ---- DESCRIPTORS ----
#define TEST_DESC_TX_ADDR     0x60000
#define TEST_DESC_RX_ADDR     0x60020

// ---- CSR ADDRESSES (FROM YOUR QSYS) ----
#define SGDMA_TX_CSR_ADDR     0x4000
#define SGDMA_RX_CSR_ADDR     0x4040

// ---- SGDMA REG OFFSETS ----
#define SGDMA_CSR_STATUS      0x00
#define SGDMA_CSR_CONTROL     0x04
#define SGDMA_CSR_NEXT_DESC   0x08

#define SGDMA_CONTROL_RESET_MASK   0x8
#define SGDMA_CONTROL_RUN_MASK     0x1
#define SGDMA_CONTROL_STOP_MASK    0x2

// ---- DESCRIPTOR FLAGS ----
#define SGDMA_TX_FLAGS 0x00000000
#define SGDMA_RX_FLAGS 0x00000000

#define TEST_LENGTH_BYTES 16



// =========================================================================
// DEFINIÇÕES DE HARDWARE (CONTROLE E STATUS DO WRAPPER)
// Estas macros definem os bits que você usa nas funções SendCommand e WaitFor*
// =========================================================================

// Máscaras de bits para os registradores de Controle (ADDR_CONTROL)
#define CONTROL_START_BIT   (1 << 0)    // Bit 0: Pulso para iniciar a transação
#define CONTROL_MODE_SHIFT  1           // Bit 1: Posição de início do campo 'mode' (3 bits)

// Máscaras de bits para os registradores de Status (ADDR_STATUS)
#define STATUS_BUSY_MASK    (1 << 0)    // Bit 0: O sistema está ocupado
#define STATUS_DONE_MASK    (1 << 1)    // Bit 1: O processamento completo terminou

// Parâmetros do seu 'defines.v'
#define N 256
#define RING_DEPTH 8 
#define PE_DEPTH 3 
#define W_COUNT ((((1<<(RING_DEPTH-PE_DEPTH))-1)+PE_DEPTH)<<PE_DEPTH)


// =========================================================================
// ENDEREÇOS PARA TESTE DE LOOPBACK (Assumindo ONCHIP_RAM_BASE_ADDR = 0x20000)
// =========================================================================
#define TX_TEST_BUFFER_ADDR     (ONCHIP_RAM_BASE_ADDR + 0x100) // 0x20100
#define RX_TEST_BUFFER_ADDR     (ONCHIP_RAM_BASE_ADDR + 0x200) // 0x20200
#define TEST_LENGTH_BYTES       16                              // 4 palavras de 32 bits
#define TEST_DESC_TX_ADDR       (ONCHIP_RAM_BASE_ADDR + 0x0)  // Descritor TX (MMIO)
#define TEST_DESC_RX_ADDR       (ONCHIP_RAM_BASE_ADDR + 0x20) // Descritor RX (MMIO)

// Use as suas mascaras de controle
#define SGDMA_TX_FLAGS          (SGDMA_CONTROL_RUN_MASK | SGDMA_DESCRIPTOR_EOP_MASK)
#define SGDMA_RX_FLAGS          (SGDMA_CONTROL_RUN_MASK | SGDMA_DESCRIPTOR_EOP_MASK)
#define SGDMA_CONTROL_RESET_MASK  0x00000002


// =========================================================================
// NOVAS FUNÇÕES: TRANSFERÊNCIA DE BUFFER VIA MMIO (PCIE_Read32/Write32)
// =========================================================================

/**
 * @brief Transfere um buffer grande do Host para o Target (FPGA) usando PCIE_Write32.
 * @param hPCIe Handle do PCIe.
 * @param bar_index O BAR index (PCIE_BAR0).
 * @param local_addr Endereço de destino na memória local da FPGA.
 * @param buffer Buffer de dados do Host (uint32_t*).
 * @param byte_count Número total de bytes a transferir.
 * @return TRUE se sucesso, FALSE caso contrário.
 */
BOOL pcie_write_buffer(PCIE_HANDLE hPCIe, int bar_index, DWORD local_addr, const void *buffer, size_t byte_count) {
    const DWORD *data_ptr = (const DWORD *)buffer;
    size_t dword_count = byte_count / DATA_WORD_SIZE;
    DWORD current_addr = local_addr;
    
    if (byte_count % DATA_WORD_SIZE != 0) {
        printf("ERRO: O tamanho do buffer (%zu bytes) deve ser múltiplo de 4.\n", byte_count);
        return FALSE;
    }
    
    for (size_t i = 0; i < dword_count; ++i) {
        if (!PCIE_Write32(hPCIe, bar_index, current_addr, data_ptr[i])) {
            printf("ERRO: Falha ao escrever na pos %zu (addr 0x%X).\n", i, current_addr);
            return FALSE;
        }
        current_addr += DATA_WORD_SIZE;
    }
    return TRUE;
}

/**
 * @brief Transfere um buffer grande do Target (FPGA) para o Host usando PCIE_Read32.
 * @param hPCIe Handle do PCIe.
 * @param bar_index O BAR index (PCIE_BAR0).
 * @param local_addr Endereço de origem na memória local da FPGA.
 * @param buffer Buffer de destino no Host (uint32_t*).
 * @param byte_count Número total de bytes a transferir.
 * @return TRUE se sucesso, FALSE caso contrário.
 */
BOOL pcie_read_buffer(PCIE_HANDLE hPCIe, int bar_index, DWORD local_addr, void *buffer, size_t byte_count) {
    DWORD *data_ptr = (DWORD *)buffer;
    size_t dword_count = byte_count / DATA_WORD_SIZE;
    DWORD current_addr = local_addr;
    
    if (byte_count % DATA_WORD_SIZE != 0) {
        printf("ERRO: O tamanho do buffer (%zu bytes) deve ser múltiplo de 4.\n", byte_count);
        return FALSE;
    }

    for (size_t i = 0; i < dword_count; ++i) {
        if (!PCIE_Read32(hPCIe, bar_index, current_addr, &data_ptr[i])) {
            printf("ERRO: Falha ao ler na pos %zu (addr 0x%X).\n", i, current_addr);
            return FALSE;
        }
        current_addr += DATA_WORD_SIZE;
    }
    return TRUE;
}

// ... (Restante das funções auxiliares SendCommand, WaitForBusyClear, WaitForDoneAll, setup_sgdma_descriptor, WaitForSgDmaDone) ...
// (Para fins de concisão, estas funções são mantidas como na resposta anterior, mas DEVE-SE incluí-las no código final)

// =========================================================================
// FUNÇÕES HELPER (Controle do Wrapper Verilog) - INÍCIO
// =========================================================================

void SendCommand(PCIE_HANDLE hPCIe, int mode) {
    DWORD command = (mode << CONTROL_MODE_SHIFT) | CONTROL_START_BIT;
    PCIE_Write32(hPCIe, DEMO_PCIE_USER_BAR, ADDR_CONTROL, command);
    command = (mode << CONTROL_MODE_SHIFT);
    PCIE_Write32(hPCIe, DEMO_PCIE_USER_BAR, ADDR_CONTROL, command);
}

BOOL WaitForBusyClear(PCIE_HANDLE hPCIe) {
    DWORD status = 0;
    long long timeout = 50000000;
    printf("...aguardando 'busy' (FSM) ficar livre...\n");
    while (timeout-- > 0) {
        if (!PCIE_Read32(hPCIe, DEMO_PCIE_USER_BAR, ADDR_STATUS, &status)) {
            printf("ERRO: Falha ao ler status da FSM.\n");
            return FALSE;
        }
        if (!(status & STATUS_BUSY_MASK)) {
            return TRUE;
        }
    }
    printf("ERRO: Timeout esperando 'busy' ficar baixo! (Status: 0x%X)\n", status);
    return FALSE;
}

BOOL WaitForDoneAll(PCIE_HANDLE hPCIe) {
    DWORD status = 0;
    long long timeout = 250000000;
    printf("...aguardando 'done_all' (FSM) ficar alto...\n");
    while (timeout-- > 0) {
        if (!PCIE_Read32(hPCIe, DEMO_PCIE_USER_BAR, ADDR_STATUS, &status)) {
            printf("ERRO: Falha ao ler status da FSM.\n");
            return FALSE;
        }
        if (status & STATUS_DONE_MASK)
            return TRUE;
    }
    printf("ERRO: Timeout esperando 'done_all'! (Status: 0x%X)\n", status);
    return FALSE;
}

// =========================================================================
// FUNÇÕES HELPER (Controle do SGDMA) - INÍCIO
// =========================================================================

// --- (As seguintes macros DEVERIAM estar definidas no seu arquivo) ---
// #define ONCHIP_RAM_BASE_ADDR    0x20000 // Endereço de base da RAM para o HOST
// #define DEMO_PCIE_USER_BAR      PCIE_BAR0
// typedef unsigned int DWORD;

// =========================================================================
// FUNÇÃO CORRIGIDA: SEM TRADUÇÃO DE ENDEREÇO
// =========================================================================
BOOL setup_sgdma_descriptor(PCIE_HANDLE hPCIe, DWORD desc_addr_mmio, 
                            DWORD source_addr_mmio, 
                            DWORD dest_addr_mmio, 
                            DWORD length, DWORD control_flags)
{
    // NENHUMA TRADUÇÃO DE ENDEREÇO É NECESSÁRIA. 
    // O SGDMA VÊ A RAM NO MESMO ENDEREÇO QUE O HOST (0x20000 + offset).
    uint32_t sgdma_source_addr = source_addr_mmio;
    uint32_t sgdma_dest_addr = dest_addr_mmio;
    uint32_t next_desc_sgdma = 0x00000000; 

    // O Host escreve nos endereços desc_addr_mmio + offset
    
    // 0x00: Source Address (Endereço MMIO COMPLETO)
    if (!PCIE_Write32(hPCIe, DEMO_PCIE_USER_BAR, desc_addr_mmio + 0x00, sgdma_source_addr)) return FALSE;
    
    // 0x04: Destination Address (Endereço MMIO COMPLETO)
    if (!PCIE_Write32(hPCIe, DEMO_PCIE_USER_BAR, desc_addr_mmio + 0x04, sgdma_dest_addr)) return FALSE;
    
    // 0x08: Next Descriptor (0x0 para um único descritor)
    if (!PCIE_Write32(hPCIe, DEMO_PCIE_USER_BAR, desc_addr_mmio + 0x08, next_desc_sgdma)) return FALSE;
    
    // 0x0C: Bytes To Transfer
    if (!PCIE_Write32(hPCIe, DEMO_PCIE_USER_BAR, desc_addr_mmio + 0x0C, length)) return FALSE;
    
    // 0x10: Control/Status (Flags EOP, RUN, etc.)
    if (!PCIE_Write32(hPCIe, DEMO_PCIE_USER_BAR, desc_addr_mmio + 0x10, control_flags)) return FALSE;

    // 0x14 - 0x1C (Reservados, Zera)
    if (!PCIE_Write32(hPCIe, DEMO_PCIE_USER_BAR, desc_addr_mmio + 0x14, 0)) return FALSE;
    if (!PCIE_Write32(hPCIe, DEMO_PCIE_USER_BAR, desc_addr_mmio + 0x18, 0)) return FALSE;
    if (!PCIE_Write32(hPCIe, DEMO_PCIE_USER_BAR, desc_addr_mmio + 0x1C, 0)) return FALSE;
    
    return TRUE;
}

BOOL WaitForSgDmaDone(PCIE_HANDLE hPCIe, DWORD sgdma_csr_base) {
    DWORD status = 0;
    long long timeout = 5000000;
    while (timeout-- > 0) {
        if (!PCIE_Read32(hPCIe, DEMO_PCIE_USER_BAR, sgdma_csr_base + SGDMA_CSR_STATUS, &status)) return FALSE;
        if (status & SGDMA_STATUS_DONE_MASK) {
            if (!PCIE_Write32(hPCIe, DEMO_PCIE_USER_BAR, sgdma_csr_base + SGDMA_CSR_STATUS, SGDMA_STATUS_DONE_MASK)) return FALSE;
            return TRUE;
        }
    }
    printf("ERRO: Timeout esperando SGDMA Done! (CSR 0x%X, Status: 0x%X)\n", sgdma_csr_base, status);
    return FALSE;
}



// =========================================================================
// FUNÇÃO HELPER: TESTE DE ACESSO À RAM VIA MMIO
// =========================================================================

/**
 * @brief Testa a capacidade do Host de escrever e ler um valor de 32 bits
 * no endereço do descritor na On-Chip RAM via PCIE_Read32/Write32.
 * @param hPCIe Handle do PCIe.
 * @return TRUE se o teste de leitura/escrita passar, FALSE caso contrário.
 */
BOOL Test_RAM_Access(PCIE_HANDLE hPCIe) {
    DWORD write_data = 0xDEADBEEF; // Padrão de teste conhecido
    DWORD read_data = 0;
    DWORD test_addr = TX_DESC_ADDR; // Usamos o endereço do descritor como ponto de teste

    printf("\n--- Teste de Acesso Básico à RAM (MMIO) ---\n");
    printf("Tentando escrever 0x%X no endereco 0x%X...\n", write_data, test_addr);

    // 1. ESCREVE o valor de teste
    if (!PCIE_Write32(hPCIe, DEMO_PCIE_USER_BAR, test_addr, write_data)) {
        printf("ERRO: Falha na PCIE_Write32 no endereco 0x%X.\n", test_addr);
        return FALSE;
    }

    // 2. LÊ o valor de volta
    if (!PCIE_Read32(hPCIe, DEMO_PCIE_USER_BAR, test_addr, &read_data)) {
        printf("ERRO: Falha na PCIE_Read32 no endereco 0x%X.\n", test_addr);
        return FALSE;
    }

    printf("Valor Lido: 0x%X\n", read_data);

    // 3. COMPARA os valores
    if (read_data == write_data) {
        printf("SUCESSO: O acesso a RAM via MMIO esta funcionando corretamente.\n");
        return TRUE;
    } else {
        printf("ERRO: Mismatch (valor lido nao coincide com o valor escrito).\n");
        printf("Esperado: 0x%X, Lido: 0x%X\n", write_data, read_data);
        return FALSE;
    }
}

void run_sgdma_loopback_test(PCIE_HANDLE hPCIe) {
    DWORD tx_data[4] = {0xAAAAAAAA, 0xBBBBBBBB, 0xCCCCCCCC, 0xDDDDDDDD};
    DWORD rx_data[4] = {0};
    BOOL bPass = TRUE;
    DWORD i;

    printf("\n=== TESTE SGDMA LOOPBACK ===\n");

    // ------------------------------
    // 1. WRITE TEST DATA IN RAM (s1)
    // ------------------------------
    printf("1. Gravando dados TX em 0x%X\n", TX_TEST_BUFFER_ADDR);
    if (!pcie_write_buffer(hPCIe, DEMO_PCIE_USER_BAR, TX_TEST_BUFFER_ADDR, tx_data, TEST_LENGTH_BYTES)) {
        printf("ERRO ao gravar buffer TX.\n");
        return;
    }

    // ------------------------------
    // 2. CONFIGURE TX DESCRIPTOR (s2)
    // ------------------------------
    printf("2. Configurando descritor TX em 0x%X\n", TEST_DESC_TX_ADDR);

    if (!setup_sgdma_descriptor(
            hPCIe,
            TEST_DESC_TX_ADDR,             // descriptor location in s2
            TX_TEST_BUFFER_ADDR,           // source
            0x0,                           // stream
            TEST_LENGTH_BYTES,
            SGDMA_TX_FLAGS)) {
        printf("ERRO ao configurar descritor TX.\n");
        return;
    }

    // ------------------------------
    // 3. CONFIGURE RX DESCRIPTOR (s2)
    // ------------------------------
    printf("3. Configurando descritor RX em 0x%X\n", TEST_DESC_RX_ADDR);

    if (!setup_sgdma_descriptor(
            hPCIe,
            TEST_DESC_RX_ADDR,
            0x0,                           // stream
            RX_TEST_BUFFER_ADDR,           // destination buffer in s1
            TEST_LENGTH_BYTES,
            SGDMA_RX_FLAGS)) {
        printf("ERRO ao configurar descritor RX.\n");
        return;
    }

    // ------------------------------
    // 4. RESET + RUN BOTH SGDMA
    // ------------------------------
    printf("4. Resetando SGDMA...\n");

    // TX
    PCIE_Write32(hPCIe, DEMO_PCIE_USER_BAR, SGDMA_TX_CSR_ADDR + SGDMA_CSR_CONTROL, SGDMA_CONTROL_RESET_MASK);
    usleep(50);
    PCIE_Write32(hPCIe, DEMO_PCIE_USER_BAR, SGDMA_TX_CSR_ADDR + SGDMA_CSR_CONTROL, SGDMA_CONTROL_RUN_MASK);

    // RX
    PCIE_Write32(hPCIe, DEMO_PCIE_USER_BAR, SGDMA_RX_CSR_ADDR + SGDMA_CSR_CONTROL, SGDMA_CONTROL_RESET_MASK);
    usleep(50);
    PCIE_Write32(hPCIe, DEMO_PCIE_USER_BAR, SGDMA_RX_CSR_ADDR + SGDMA_CSR_CONTROL, SGDMA_CONTROL_RUN_MASK);

    // ------------------------------
    // 5. START (RX FIRST)
    // ------------------------------
    printf("5. Iniciando RX e depois TX\n");

    // FULL ADDRESS OF DESCRIPTORS (still on BAR0)
    uint32_t desc_rx_full = TEST_DESC_RX_ADDR;
    uint32_t desc_tx_full = TEST_DESC_TX_ADDR;

    // RX FIRST
    PCIE_Write32(hPCIe, DEMO_PCIE_USER_BAR, SGDMA_RX_CSR_ADDR + SGDMA_CSR_NEXT_DESC, desc_rx_full);

    // THEN TX
    PCIE_Write32(hPCIe, DEMO_PCIE_USER_BAR, SGDMA_TX_CSR_ADDR + SGDMA_CSR_NEXT_DESC, desc_tx_full);

    // ------------------------------
    // 6. WAIT FOR RX DONE
    // ------------------------------
    printf("6. Aguardando RX concluir...\n");

    if (!WaitForSgDmaDone(hPCIe, SGDMA_RX_CSR_ADDR)) {
        printf("ERRO: RX Timeout!\n");
        return;
    }

    printf("RX concluído.\n");

    // ------------------------------
    // 7. READ BACK RESULT BUFFER
    // ------------------------------
    printf("7. Lendo buffer RX de 0x%X\n", RX_TEST_BUFFER_ADDR);

    if (!pcie_read_buffer(hPCIe, DEMO_PCIE_USER_BAR, RX_TEST_BUFFER_ADDR, rx_data, TEST_LENGTH_BYTES)) {
        printf("ERRO ao ler buffer RX.\n");
        return;
    }

    // ------------------------------
    // 8. COMPARE TX vs RX
    // ------------------------------
    printf("8. Verificando dados:\n");

    for (i = 0; i < 4; i++) {
        printf("   Palavra %d: TX=0x%08X  RX=0x%08X\n", i, tx_data[i], rx_data[i]);
        if (tx_data[i] != rx_data[i])
            bPass = FALSE;
    }

    if (bPass)
        printf("\nSUCESSO: Loopback SGDMA funcionando!\n");
    else
        printf("\nERRO: Dados nao batem!\n");
}



// =========================================================================
// FUNÇÃO PRINCIPAL DE EXECUÇÃO (Fluxo SGDMA)
// =========================================================================

BOOL NTT_HARDWARE_EXE(PCIE_HANDLE hPCIe) {
    BOOL bPass = TRUE;
    char szError[256] = "";
    
    // --- 1. Alocação de Memória (Buffers do Host) ---
    uint32_t* params_stream = NULL;
    uint32_t* polyA_host = NULL;
    uint32_t* polyB_host = NULL;
    uint32_t* polyC_host = NULL;

    int stream0_count = (W_COUNT * 2) + 2;
    params_stream = (uint32_t*)malloc(stream0_count * DATA_WORD_SIZE);
    polyA_host = (uint32_t*)calloc(N, DATA_WORD_SIZE);
    polyB_host = (uint32_t*)calloc(N, DATA_WORD_SIZE);
    polyC_host = (uint32_t*)calloc(N, DATA_WORD_SIZE);

    if (!params_stream || !polyA_host || !polyB_host || !polyC_host) {
        fprintf(stderr, "Erro: falha na alocação de memória do Host.\n");
        bPass = FALSE;
        goto cleanup;
    }
    printf("Buffers de memória do Host alocados.\n");
    
    // --- 2. Geração de Dados (Simplificada) ---
    int psi=0, psi_inv=0, w=0, w_inv=0, R=0, n_inv=0, PE=0, q=0;
    generate_params(&psi, &psi_inv, &w, &w_inv, &R, &n_inv, &PE, &q);

    polyA_host[0] = 1; polyA_host[1] = 2; polyA_host[2] = 3;
    polyB_host[0] = 2;
    params_stream[W_COUNT * 2] = (uint32_t)q;
    params_stream[W_COUNT * 2 + 1] = (uint32_t)n_inv;
    printf("Dados de entrada gerados.\n");

    if (!Test_RAM_Access(hPCIe)) {
        sprintf(szError, "Teste de RAM MMIO falhou. Verifique o Qsys e enderecos.");
        bPass = FALSE;
        goto cleanup;
    }
    
    // --- 3. Carregar Dados na RAM On-Chip (FPGA) USANDO A NOVA FUNÇÃO ---
    printf("\nCopiando dados para a RAM On-Chip via PCIe Write (MMIO loop)...\n");
    
    // Escreve polyA, polyB e Params usando pcie_write_buffer
    if (!pcie_write_buffer(hPCIe, DEMO_PCIE_USER_BAR, POLY_A_ADDR, polyA_host, N * DATA_WORD_SIZE)) { sprintf(szError, "pcie_write_buffer (polyA) falhou\n"); goto cleanup; }
    if (!pcie_write_buffer(hPCIe, DEMO_PCIE_USER_BAR, POLY_B_ADDR, polyB_host, N * DATA_WORD_SIZE)) { sprintf(szError, "pcie_write_buffer (polyB) falhou\n"); goto cleanup; }
    if (!pcie_write_buffer(hPCIe, DEMO_PCIE_USER_BAR, PARAMS_STREAM_ADDR, params_stream, stream0_count * DATA_WORD_SIZE)) { sprintf(szError, "pcie_write_buffer (Params) falhou\n"); goto cleanup; }
    
    // --- Inicia Medição de Tempo ---
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    // --- 4. Configuração Inicial do SGDMA RX ---
    printf("Configurando SGDMA RX (Receber C, Destino RAM: 0x%X)...\n", POLY_C_ADDR);
    
    DWORD rx_flags = SGDMA_CONTROL_RUN_MASK; 
    bPass = setup_sgdma_descriptor(hPCIe, RX_DESC_ADDR, 
                                   0x00000000, 
                                   POLY_C_ADDR, 
                                   N * DATA_WORD_SIZE, 
                                   rx_flags);
    if (!bPass) { sprintf(szError, "Config Desc RX falhou\n"); goto cleanup; }
    
    if (!PCIE_Write32(hPCIe, DEMO_PCIE_USER_BAR, SGDMA_RX_CSR_ADDR + SGDMA_CSR_NEXT_DESC, RX_DESC_ADDR)) { sprintf(szError, "Init SGDMA RX falhou\n"); goto cleanup; }
    
    // --- 5. Execução do Fluxo de Transferência (MODO 0, 1, 2) ---

    printf("Forcando o bit RUN no SGDMA TX (CSR 0x%X + 0x00)...\n", SGDMA_TX_CSR_ADDR);

    // Escreve a mascara RUN no registrador de controle (Offset 0x00).
    // Isso coloca o SGDMA no modo operacional.
    if (!PCIE_Write32(hPCIe, DEMO_PCIE_USER_BAR, 
                    SGDMA_TX_CSR_ADDR + SGDMA_CSR_CONTROL, 
                    SGDMA_CONTROL_RUN_MASK)) 
    {
        printf("ERRO: Falha ao escrever o bit RUN no SGDMA TX.\n");
        // Tratar erro (goto cleanup)
    }

    uint32_t desc_addr_para_sgdma_csr = TX_DESC_ADDR - ONCHIP_RAM_BASE_ADDR;
    
    // *** 5a. PASSO MODO 0 (Parâmetros) ***
    printf("\nTB: Iniciando MODO 0 - Carga de W/Params.\n");
    DWORD tx_flags_0 = SGDMA_CONTROL_RUN_MASK | SGDMA_DESCRIPTOR_EOP_MASK; 
    bPass = setup_sgdma_descriptor(hPCIe, TX_DESC_ADDR, PARAMS_STREAM_ADDR, 0x00000000, stream0_count * DATA_WORD_SIZE, tx_flags_0);
    if (!bPass) { sprintf(szError, "Config Desc TX 0 falhou\n"); goto cleanup; }
    SendCommand(hPCIe, 0); 
    if (!PCIE_Write32(hPCIe, DEMO_PCIE_USER_BAR, SGDMA_TX_CSR_ADDR + SGDMA_CSR_NEXT_DESC, desc_addr_para_sgdma_csr)) { sprintf(szError, "Init SGDMA TX 0 falhou\n"); goto cleanup; }
    
    // DEBUG: Status do SGDMA CSR
    DWORD status_csr;
    PCIE_Read32(hPCIe, DEMO_PCIE_USER_BAR, SGDMA_TX_CSR_ADDR + SGDMA_CSR_STATUS, &status_csr);
    printf("DEBUG 1: Status do SGDMA CSR (0x%X) apos iniciar: 0x%X\n", SGDMA_TX_CSR_ADDR + SGDMA_CSR_STATUS, status_csr);

    // DEBUG: Leitura do campo de controle/status do DESCRITOR na RAM (Offset 0x10)
    DWORD desc_control_field;
    PCIE_Read32(hPCIe, DEMO_PCIE_USER_BAR, TX_DESC_ADDR + 0x10, &desc_control_field);
    printf("DEBUG 2: Campo Control/Status do Descritor (0x%X) na RAM: 0x%X\n", TX_DESC_ADDR + 0x10, desc_control_field);
    
    if (!WaitForSgDmaDone(hPCIe, SGDMA_TX_CSR_ADDR)) goto cleanup;
    if (!WaitForBusyClear(hPCIe)) goto cleanup;
    printf("TB: MODO 0 Concluído.\n");

    // *** 5b. PASSO MODO 1 (Polinômio A) ***
    printf("\nTB: Iniciando MODO 1 - Carga de A.\n");
    DWORD tx_flags_1 = SGDMA_CONTROL_RUN_MASK | SGDMA_DESCRIPTOR_EOP_MASK; 
    bPass = setup_sgdma_descriptor(hPCIe, TX_DESC_ADDR, POLY_A_ADDR, 0x00000000, N * DATA_WORD_SIZE, tx_flags_1);
    if (!bPass) { sprintf(szError, "Config Desc TX 1 falhou\n"); goto cleanup; }
    SendCommand(hPCIe, 1); 
    if (!PCIE_Write32(hPCIe, DEMO_PCIE_USER_BAR, SGDMA_TX_CSR_ADDR + SGDMA_CSR_NEXT_DESC, TX_DESC_ADDR)) { sprintf(szError, "Init SGDMA TX 1 falhou\n"); goto cleanup; }
    if (!WaitForSgDmaDone(hPCIe, SGDMA_TX_CSR_ADDR)) goto cleanup;
    if (!WaitForBusyClear(hPCIe)) goto cleanup;
    printf("TB: MODO 1 Concluído.\n");

    // *** 5c. PASSO MODO 2 (Polinômio B) ***
    printf("\nTB: Iniciando MODO 2 - Carga de B.\n");
    DWORD tx_flags_2 = SGDMA_CONTROL_RUN_MASK | SGDMA_DESCRIPTOR_EOP_MASK;
    bPass = setup_sgdma_descriptor(hPCIe, TX_DESC_ADDR, POLY_B_ADDR, 0x00000000, N * DATA_WORD_SIZE, tx_flags_2);
    if (!bPass) { sprintf(szError, "Config Desc TX 2 falhou\n"); goto cleanup; }
    SendCommand(hPCIe, 2); 
    if (!PCIE_Write32(hPCIe, DEMO_PCIE_USER_BAR, SGDMA_TX_CSR_ADDR + SGDMA_CSR_NEXT_DESC, TX_DESC_ADDR)) { sprintf(szError, "Init SGDMA TX 2 falhou\n"); goto cleanup; }
    if (!WaitForSgDmaDone(hPCIe, SGDMA_TX_CSR_ADDR)) goto cleanup;
    if (!WaitForBusyClear(hPCIe)) goto cleanup;
    printf("TB: MODO 2 Concluído.\n");
    
    // --- 6. Execução e Coleta (MODO 3 - GO!) ---
    printf("\nTB: Iniciando MODO 3 - GO! (Execução A * B).\n");
    SendCommand(hPCIe, 3); 
    
    printf("Aguardando o SGDMA RX concluir a leitura do resultado C...\n");
    if (!WaitForSgDmaDone(hPCIe, SGDMA_RX_CSR_ADDR)) goto cleanup;
    if (!WaitForDoneAll(hPCIe)) goto cleanup;
    printf("TB: Sinal 'done_all' recebido!\n");
    
    // --- 7. Ler o Resultado da RAM On-Chip para o Host USANDO A NOVA FUNÇÃO ---
    printf("Lendo resultados da RAM On-Chip (0x%X) para o Host (MMIO loop)...\n", POLY_C_ADDR);
    if (!pcie_read_buffer(hPCIe, DEMO_PCIE_USER_BAR, POLY_C_ADDR, polyC_host, N * DATA_WORD_SIZE)) { sprintf(szError, "pcie_read_buffer (polyC) falhou\n"); goto cleanup; }

    // --- Finaliza Medição de Tempo ---
    clock_gettime(CLOCK_MONOTONIC, &end);
    double durationNTT = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec)/1e9;
    printf("Tempo total de hardware: %.3f ms\n", durationNTT * 1000);

    // --- 8. Verificação ---
    printf("\n--- RESULTADO DE SAÍDA ---\n");
    printf("C[0] = %u (Esperado: 2)\n", polyC_host[0]);
    printf("C[1] = %u (Esperado: 4)\n", polyC_host[1]);
    printf("C[2] = %u (Esperado: 6)\n", polyC_host[2]);
    
    int errors = 0;
    if (polyC_host[0] != 2) errors++;
    if (polyC_host[1] != 4) errors++;
    if (polyC_host[2] != 6) errors++;
    
    if (errors == 0) {
        printf("==================== TESTE PASSOU! ====================\n");
    } else {
        printf("==================== TESTE FALHOU! (%d erros) ====================\n", errors);
    }
    
cleanup:
    // Libera a memória alocada no Host
    if (params_stream) free(params_stream);
    if (polyA_host) free(polyA_host);
    if (polyB_host) free(polyB_host);
    if (polyC_host) free(polyC_host);

    if (!bPass)
        printf("ERRO FATAL: %s\n", szError);

    return bPass;
}

// =========================================================================
// MAIN
// =========================================================================

int main(void)
{
    void *lib_handle;
    PCIE_HANDLE hPCIE;
    
    printf("== Terasic: PolyMult-NTT SGDMA PCIe Demo Program ==\r\n");

    lib_handle = PCIE_Load();
    if (!lib_handle){
        printf("PCIE_Load failed! Verifique a DLL/driver.\r\n");
        return 0;
    }

    hPCIE = PCIE_Open(0,0,0);
    if (!hPCIE){
        printf("PCIE_Open failed. Verifique se a placa esta plugada e o driver carregado.\r\n");
    } else {
        printf("PCIe Device aberto com sucesso.\n");
        run_sgdma_loopback_test(hPCIE);
        // if(!NTT_HARDWARE_EXE(hPCIE)) { 
        //     printf("Execução de Hardware (NTT_HARDWARE_EXE) falhou.\n");
        // } else {
        //     printf("Execucao do fluxo SGDMA concluida.\n");
        // }
        
        PCIE_Close(hPCIE);
    }

    PCIE_Unload(lib_handle);
    return 0;
}