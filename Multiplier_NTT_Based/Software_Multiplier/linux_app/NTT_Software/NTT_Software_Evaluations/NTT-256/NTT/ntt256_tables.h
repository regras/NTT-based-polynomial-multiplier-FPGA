/*
 * Parameters:
 * - q = 12289
 * - n = 256
 * - psi = 1002
 * - omega = psi^2 = 8595
 * - inverse of psi = 10805
 * - inverse of omega = 2525
 * - inverse of n = 12241
 */

#ifndef __NTT256_TABLES_H
#define __NTT256_TABLES_H

#include <stdint.h>

/*
 * PARAMETERS
 */
static const int32_t ntt256_psi = 1002;
static const int32_t ntt256_omega = 8595;
static const int32_t ntt256_inv_psi = 10805;
static const int32_t ntt256_inv_omega = 2525;
static const int32_t ntt256_inv_n = 12241;

/*
 * POWERS OF PSI
 */
extern const uint16_t ntt256_psi_powers[256];
extern const uint16_t ntt256_inv_psi_powers[256];
extern const uint16_t ntt256_scaled_inv_psi_powers[256];

/*
 * TABLES FOR NTT COMPUTATION
 */
extern const uint16_t ntt256_omega_powers[256];
extern const uint16_t ntt256_omega_powers_rev[256];
extern const uint16_t ntt256_inv_omega_powers[256];
extern const uint16_t ntt256_inv_omega_powers_rev[256];
extern const uint16_t ntt256_mixed_powers[256];
extern const uint16_t ntt256_mixed_powers_rev[256];
extern const uint16_t ntt256_inv_mixed_powers[256];
extern const uint16_t ntt256_inv_mixed_powers_rev[256];

#endif /* __NTT256_TABLES_H */
