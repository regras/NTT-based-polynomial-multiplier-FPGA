#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

/* ---------- Função modular: (base^exp) % mod ---------- */
uint32_t modexp(uint32_t base, uint32_t exp, uint32_t mod) {
    uint32_t result = 1;
    base %= mod;

    while (exp > 0) {
        if (exp & 1)
            result = (__uint64_t)result * base % mod;
        base = (__uint64_t)base * base % mod;
        exp >>= 1;
    }
    return result;
}

/* ---------- Miller-Rabin primality test ---------- */
bool miller_rabin(uint32_t p, int s) {
    if (p < 4)
        return p == 2 || p == 3;

    // Decompose p-1 = 2^u * r
    uint32_t r = p - 1;
    int u = 0;
    while ((r & 1) == 0) {
        u++;
        r >>= 1;
    }

    for (int i = 0; i < s; i++) {
        uint32_t a = 2 + rand() % (p - 3);
        uint32_t z = modexp(a, r, p);

        if (z != 1 && z != p - 1) {
            int j;
            for (j = 0; j < u - 1 && z != p - 1; j++) {
                z = (__uint64_t)z * z % p;
                if (z == 1)
                    return false;
            }
            if (z != p - 1)
                return false;
        }
    }
    return true;
}

/* ---------- Pequena lista de primos para filtragem rápida ---------- */
int lowPrimes[] = {
    3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97,
    101,103,107,109,113,127,131,137,139,149,151,157,163,167,173,179,181,
    191,193,197,199,211,223,227,229,233,239,241,251,257,263,269,271,277,
    281,283,293,307,311,313,317,331,337,347,349,353,359,367,373,379,383,
    389,397,401,409,419,421,431,433,439,443,449,457,461,463,467,479,487,
    491,499,503,509,521,523,541,547,557,563,569,571,577,587,593,599,601,
    607,613,617,619,631,641,643,647,653,659,661,673,677,683,691,701,709,
    719,727,733,739,743,751,757,761,769,773,787,797,809,811,821,823,827,
    829,839,853,857,859,863,877,881,883,887,907,911,919,929,937,941,947,
    953,967,971,977,983,991,997
};
int lowPrimeCount = sizeof(lowPrimes)/sizeof(lowPrimes[0]);

/* ---------- Verificação de primalidade ---------- */
bool is_prime(uint32_t n, int s) {
    if (n < 2)
        return false;
    if (n == 2)
        return true;
    if (n % 2 == 0)
        return false;

    for (int i = 0; i < lowPrimeCount; i++) {
        if (n == (uint32_t)lowPrimes[i])
            return true;
        if (n % lowPrimes[i] == 0)
            return false;
    }
    return miller_rabin(n, s);
}

/* ---------- Geração de número primo de k bits ---------- */
uint32_t generate_large_prime(int k, int s) {
    if (k >= 63) {
        fprintf(stderr, "Erro: limite máximo suportado é 63 bits.\n");
        exit(1);
    }

    uint32_t min = (1ULL << (k - 1));
    uint32_t max = (1ULL << k) - 1;

    int tries = (int)(100 * (log2(k) + 1));

    while (tries-- > 0) {
        uint32_t n = ((uint64_t)rand() << 32 | rand()) % (max - min) + min;
        n |= 1ULL;  // força ímpar

        if (is_prime(n, s))
            return n;
    }
    fprintf(stderr, "Falha após tentativas.\n");
    exit(1);
}
