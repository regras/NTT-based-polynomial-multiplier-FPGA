#include "ntt256.h"


//Cooley-Tukey
void ntt256_product1(int32_t *c, int32_t *a, int32_t *b) {
  mul_array16(a, 256, ntt256_psi_powers);
  ntt256_ct_std2rev(a);
  mul_array16(b, 256, ntt256_psi_powers);
  ntt256_ct_std2rev(b);
  mul_array(c, 256, a, b);
  intt256_ct_rev2std(c);
  mul_array16(c, 256, ntt256_scaled_inv_psi_powers);
}

//Gentleman-Sande
void ntt256_product4(int32_t *c, int32_t *a, int32_t *b) {
  mul_array16(a, 256, ntt256_psi_powers);
  ntt256_gs_std2rev(a);
  mul_array16(b, 256, ntt256_psi_powers);
  ntt256_gs_std2rev(b);
  mul_array(c, 256, a, b);
  intt256_gs_rev2std(c);
  mul_array16(c, 256, ntt256_scaled_inv_psi_powers);
}