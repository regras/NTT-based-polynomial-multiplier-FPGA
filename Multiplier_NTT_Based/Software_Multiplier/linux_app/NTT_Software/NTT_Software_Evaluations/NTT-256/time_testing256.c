#define _POSIX_C_SOURCE 199309L
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h> 
#include <inttypes.h>
#include <string.h>
#include <time.h>

#include "NTT-RED/ntt_red256.h"
#include "NTT/ntt256.h"

#define Q 12289
#define N 256 


void ler_coeficientes(const char *nome_arquivo, int32_t array_destino[], int tamanho_maximo) {
    FILE *arquivo = fopen(nome_arquivo, "r");
    
    // 1. Verificação de Abertura
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo para leitura"); 
    }

    int elementos_lidos = 0;
    int resultado_leitura;
    printf("Lendo valores a partir do arquivo txt\n");
    // 2. Loop de Leitura Segura
    while (elementos_lidos < tamanho_maximo) {
        resultado_leitura = fscanf(arquivo, "%" SCNd32, &array_destino[elementos_lidos]);

        if (resultado_leitura == 1) {
            elementos_lidos++;
        } else if (resultado_leitura == EOF) {
            break; 
        } else {
            fprintf(stderr, "Aviso: Leitura interrompida por dado invalido no arquivo.\n");
            break;
        }
    }

    // 3. Fecha o Arquivo
    fclose(arquivo);   
}

static void print_array(FILE *f, int32_t *a, int32_t n) {
    uint32_t i, k;

    k = 0;
    for (i=0; i<n; i++) {
        if (k == 0) fprintf(f, "  ");
        fprintf(f, "%5"PRId32, a[i]);
        k ++;
        if (k == 16) {
            fprintf(f, "\n");
            k = 0;
        } else {
            fprintf(f, " ");
        }
    }
    if (k > 0) {
        fprintf(f, "\n");
    }
}



static void init_polyABC(int32_t* A, int32_t* B, int32_t* C) {
    for(int i = 0; i < N; i++){
        C[i] = 0;
    }

    A[0] = 1;
    A[1] = 2;
    A[4] = 2;

    B[0] = 3;
    B[1] = 3;
    B[3] = 1;
}

static void init_polyABC_txt(int32_t* A, int32_t* B, int32_t* C, const char *file_name_A, const char *file_name_B) {
    ler_coeficientes(file_name_A, A, N);
    ler_coeficientes(file_name_B, B, N);
    for(int i = 0; i < N; i++){
        C[i] = 0;
    }
}

static void init_rand_polyABC(int32_t* A, int32_t* B, int32_t* C) {
    const int32_t MAX_COEFF_VALUE = Q - 1;
    for(int i = 0; i < N; i++){
        C[i] = 0;
    }

    for(int i = 0; i < N; i++){
        A[i] = rand() % (MAX_COEFF_VALUE + 1);
        B[i] = rand() % (MAX_COEFF_VALUE + 1);
    }
}

static void init_polyABC_temp(int32_t* A, int32_t* B, int32_t* C, int32_t* A_temp, int32_t* B_temp, int32_t* C_temp) {
    for(int i = 0; i < N; i++){
        A_temp[i] = A[i];
        B_temp[i] = B[i];
        C_temp[i] = C[i];
    }
}

static void reset_polyABC(int32_t* A, int32_t* B, int32_t* C, int32_t* A_temp, int32_t* B_temp, int32_t* C_temp) {
    for(int i = 0; i < N; i++){
        A[i] = A_temp[i];
        B[i] = B_temp[i];
        C[i] = C_temp[i];
    }
}

int main(void) {

    srand(time(NULL));
    
    // Alocaa memória para os polinômios na heap com valores 0(calloc)
    int32_t *polyA = (int32_t *)calloc(N, sizeof(int32_t));
    int32_t *polyB = (int32_t *)calloc(N, sizeof(int32_t));
    int32_t *polyC = (int32_t *)calloc(N, sizeof(int32_t));

    int32_t *polyA_temp = (int32_t *)calloc(N, sizeof(int32_t));
    int32_t *polyB_temp = (int32_t *)calloc(N, sizeof(int32_t));
    int32_t *polyC_temp = (int32_t *)calloc(N, sizeof(int32_t));


    if (polyA == NULL || polyB == NULL || polyC == NULL) {
        fprintf(stderr, "Falha na alocação de memória\n");
        return 1;
    }

    // Define os polinomios A e B
    //init_rand_polyABC(polyA, polyB, polyC);
    init_polyABC_txt(polyA, polyB, polyC, "coeficientes_a.txt", "coeficientes_b.txt");
    // init_polyABC(polyA, polyB, polyC);
    init_polyABC_temp(polyA, polyB, polyC, polyA_temp, polyB_temp, polyC_temp);
    
    
    struct timespec start, end;
    double durationNTTMult = 0;

    int num_inter = 30;
    int count = 0;
    double sum = 0, average = 0;
    
    // TESTE NTT256 - CT -----------------------------------------------------------------------
    // Executar o produto polinomial
    // printf("Executando mult ntt256  CT (C, A, B)...\n\n");  
    // while (count < num_inter)
    // {
    //     // reset_polyABC(polyA, polyB, polyC, polyA_temp, polyB_temp, polyC_temp);
    //     clock_gettime(CLOCK_MONOTONIC, &start); // Inicia o contador de tempo
    //     ntt256_product1(polyC, polyA, polyB);
    //     clock_gettime(CLOCK_MONOTONIC, &end); // termina o contador de tempo
    //     durationNTTMult = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec)/1e9; //medição da duração
        
    //     sum += durationNTTMult;
    //     count++;
    // }
    // average = sum / count; 
    // printf("Tempo total médio ntt256 ct: %.3f ms\n", average * 1000);



	// // TESTE NTT256 - GS -----------------------------------------------------------------------
    printf("Executando mult ntt256 GS(C, A, B)...\n\n");
    reset_polyABC(polyA, polyB, polyC, polyA_temp, polyB_temp, polyC_temp);
    count = 0; sum = 0; average = 0;

    while (count < num_inter)
    {
        reset_polyABC(polyA, polyB, polyC, polyA_temp, polyB_temp, polyC_temp);
        clock_gettime(CLOCK_MONOTONIC, &start);
        ntt256_product4(polyC, polyA, polyB);
        clock_gettime(CLOCK_MONOTONIC, &end); // termina o contador de tempo
        durationNTTMult = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec)/1e9; //medição da duração
            
        sum += durationNTTMult;
        count++;
    }
    average = sum / count; 
    printf("Tempo total médio ntt256 gs: %.3f ms\n", average * 1000);




    // TESTE NTT256 - CT com Redução -----------------------------------------------------------------------
    // printf("Executando mult ntt256 CT com Redução(C, A, B)...\n\n");
    // reset_polyABC(polyA, polyB, polyC, polyA_temp, polyB_temp, polyC_temp);
    // count = 0; sum = 0; average = 0;

    // while (count < num_inter)
    // {
    //     reset_polyABC(polyA, polyB, polyC, polyA_temp, polyB_temp, polyC_temp);
    //     clock_gettime(CLOCK_MONOTONIC, &start);
    //     ntt_red256_product1(polyC, polyA, polyB);
    //     clock_gettime(CLOCK_MONOTONIC, &end); // termina o contador de tempo
	//     durationNTTMult = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec)/1e9; //medição da duração
            
    //     sum += durationNTTMult;
    //     count++;
    // }
    // average = sum / count; 
    // printf("Tempo total médio ntt256 ct com red: %.3f ms\n", average * 1000);


    // // TESTE NTT256 - GS com Redução -----------------------------------------------------------------------
    // printf("Executando mult ntt256 GS com Redução(C, A, B)...\n\n");
    // reset_polyABC(polyA, polyB, polyC, polyA_temp, polyB_temp, polyC_temp);
    // count = 0; sum = 0; average = 0;

    // while (count < num_inter)
    // {
    //     reset_polyABC(polyA, polyB, polyC, polyA_temp, polyB_temp, polyC_temp);
    //     clock_gettime(CLOCK_MONOTONIC, &start);
    //     ntt_red256_product4(polyC, polyA, polyB);
    //     clock_gettime(CLOCK_MONOTONIC, &end); // termina o contador de tempo
	//     durationNTTMult = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec)/1e9; //medição da duração
            
    //     sum += durationNTTMult;
    //     count++;
    // }
    // average = sum / count; 
    // printf("Tempo total médio ntt256 gs com red: %.3f ms\n", average * 1000);


	
    
    // Mostrar o resultado
    printf("Polinômio C (Resultado C = A * B):\n");
    print_array(stdout, polyC, N);
    printf("\n");

    //Libera memória
    free(polyA);
    free(polyB);
    free(polyC);

    return 0;
}

/*
   ---Comandos para executar:
    1. /Users/ronaldgabriel/Documents/Projeto_NTT/linux_app/NTT_Software/NTT_Software_Evaluations/NTT-256
    2. gcc time_testing256.c ntt_red256.c ntt_red256_tables.c ntt_red.c ntt256.c ntt256_tables.c ntt.c -o time_testing256
    3. ./time_testing256
*/
  