#ifndef PARAMS_GENERATOR_H
#define PARAMS_GENERATOR_H

#include <math.h>
#include "prime_generate.h"
#include "helper.h"

// === Constantes do sistema ===
#define N 256
#define K 13
#define P 8

// === Protótipo ===
// Gera parâmetros NTT (q, psi, psi_inv, w, w_inv, R, n_inv, PE)
void generate_params(int *psi, int *psi_inv, int *w, int *w_inv, int *R, int *n_inv, int *PE, int *q);

void generate_twiddles(uint32_t W[], uint32_t W_INV[], uint32_t w, uint32_t w_inv, uint32_t q, uint32_t R);

#endif // PARAMS_GENERATOR_H
