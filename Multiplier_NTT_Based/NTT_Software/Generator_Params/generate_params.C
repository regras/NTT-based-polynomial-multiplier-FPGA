#include "prime_generate.h"
#include "helper.h"
#include <math.h> 
#include <stdlib.h>  // para rand()
#include <time.h>    // para time()

#define N 256
#define K 13
#define P 8

// Gera parâmetros NTT
void generate_params(int *psi, int *psi_inv, int *w, int *w_inv, int *R, int *n_inv, int *PE, int *q) {
    // Inicializa semente aleatória (caso ainda não esteja)
    srand((unsigned)time(NULL));

    // === 1. Gera primo aleatório q ≡ 1 (mod 2N) ===
    *q = generate_large_prime(K, 11);
    while ((*q % (2 * N)) != 1) {
        *q = generate_large_prime(K, 11);
    }

    *q = 12289;

    // === 2. Encontra raiz primitiva psi ===
    for (int i = 2; i < *q - 1; i++) {
        if (modexp(i, 2 * N, *q) == 1) {             // condição: i^(2N) ≡ 1 mod q
            if (modexp(i, N, *q) == (*q - 1)) {      // condição: i^N ≡ -1 mod q
                int found_one = 0;
                for (int x = 1; x < 2 * N; x++) {    // verifica se i é de ordem exata 2N
                    if (modexp(i, x, *q) == 1) {
                        found_one = 1;
                        break;
                    }
                }
                if (!found_one) {
                    *psi = i;
                    *psi_inv = modinv(i, *q);
                    *w = modexp(*psi, 2, *q);
                    *w_inv = modinv(*w, *q);
                    break;
                }
            }
        }
    }

    // === 3. Calcula parâmetros auxiliares ===
    int logN = (int)log2((double)N);
    int fator = (int)ceil((double)K / (double)(logN + 1));
    *R = (int)pow(2.0, (double)((logN + 1) * fator));
    *n_inv = modinv(N, *q);
    *PE = P * 2;
}

void generate_twiddles(uint32_t W[], uint32_t W_INV[], uint32_t w, uint32_t w_inv, uint32_t q, uint32_t R) {
    size_t idx = 0;
    int PE = P * 2;
    for (size_t j = 0; j < (size_t)log2((double)N); j++) {
        size_t limit = ((N / PE) >> j) < 1 ? 1 : ((N / PE) >> j);
        for (size_t k = 0; k < limit; k++) {
            for (size_t i = 0; i < P; i++) {
                size_t w_pow = (((P << j) * k + (i << j)) % (N / 2));

                // gera os fatores
                uint64_t w_val     = (modexp(w,     w_pow, q) * R) % q;
                uint64_t w_inv_val = (modexp(w_inv, w_pow, q) * R) % q;

                W[idx]     = w_val;
                W_INV[idx] = w_inv_val;
                idx++;
            }
        }
    }
}

