
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h> 
#include <inttypes.h>
#include <string.h> 

#include "ntt256.h" 
#define Q 12289
#define N 256 


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


int main(void) {

    // Alocaa memória para os polinômios na heap com valores 0(calloc)
    int32_t *polyA = (int32_t *)calloc(N, sizeof(int32_t));
    int32_t *polyB = (int32_t *)calloc(N, sizeof(int32_t));
    int32_t *polyC = (int32_t *)calloc(N, sizeof(int32_t));

    if (polyA == NULL || polyB == NULL || polyC == NULL) {
        fprintf(stderr, "Falha na alocação de memória\n");
        return 1;
    }

    // Define os polinomios A e B
    polyA[0] = 1;
    polyA[1] = 2;

    polyB[0] = 3;
    polyB[1] = 0;
    

    // Executa o produto polinomial
    printf("Executando ntt_red256_product1(C, A, B)...\n\n");
    ntt256_product1(polyC, polyA, polyB);

    //Mostra o resultado
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
    1. /Users/ronaldgabriel/Documents/Projeto_NTT/linux_app/NTT_Software/NTT_Software_Evaluations/NTT-256/NTT
    2. gcc test_prod_ntt256.c ntt256.c ntt256_tables.c ntt.c -o test_prod_ntt
    3. ./test_prod_ntt 
*/
  