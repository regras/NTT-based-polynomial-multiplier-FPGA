#ifndef PRIME_UTILS_H
#define PRIME_UTILS_H

#include <stdint.h>
#include <stdbool.h>

/* ---------- Funções principais ---------- */

// Cálculo modular rápido: (base^exp) % mod
uint32_t modexp(uint32_t base, uint32_t exp, uint32_t mod);

// Teste de primalidade probabilístico (Miller–Rabin)
bool miller_rabin(uint32_t p, int s);

// Verificação completa de primalidade (usa lowPrimes + Miller–Rabin)
bool is_prime(uint32_t n, int s);

// Geração de número primo aleatório de k bits (testes Miller–Rabin)
uint32_t generate_large_prime(int k, int s);

/* ---------- Dados globais ---------- */

// Pequena lista de primos para filtragem rápida
extern int lowPrimes[];
extern int lowPrimeCount;

#endif // PRIME_UTILS_H
