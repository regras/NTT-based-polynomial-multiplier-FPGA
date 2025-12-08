/*
 * Parameters:
 * - q = 12289
 * - k = 3
 * - n = 256
 * - psi = 1002
 * - omega = psi^2 = 8595
 * - inverse of psi = 10805
 * - inverse of omega = 2525
 * - inverse of n = 12241
 * - inverse of k = 8193
 */

#ifndef __NTT_RED256_TABLES_H
#define __NTT_RED256_TABLES_H

#include <stdint.h>

/*
 * PARAMETERS
 */
static const int32_t ntt_red256_psi = 1002;
static const int32_t ntt_red256_omega = 8595;
static const int32_t ntt_red256_inv_psi = 10805;
static const int32_t ntt_red256_inv_omega = 2525;
static const int32_t ntt_red256_inv_n = 12241;
static const int32_t ntt_red256_inv_k = 8193;
static const int32_t ntt_red256_rescale8 = 8822;
static const int32_t ntt_red256_rescale6 = 5664;

/*
 * POWERS OF PSI
 */
extern const int16_t ntt_red256_psi_powers[256];
extern const int16_t ntt_red256_inv_psi_powers[256];
extern const int16_t ntt_red256_scaled_inv_psi_powers[256];
extern const int16_t ntt_red256_scaled_inv_psi_powers_var[256];

/*
 * TABLES FOR NTT COMPUTATION
 */
extern const int16_t ntt_red256_omega_powers[256];
extern const int16_t ntt_red256_omega_powers_rev[256];
extern const int16_t ntt_red256_inv_omega_powers[256];
extern const int16_t ntt_red256_inv_omega_powers_rev[256];
extern const int16_t ntt_red256_mixed_powers[256];
extern const int16_t ntt_red256_mixed_powers_rev[256];
extern const int16_t ntt_red256_inv_mixed_powers[256];
extern const int16_t ntt_red256_inv_mixed_powers_rev[256];

#endif /* __NTT_RED256_TABLES_H */
