/**
 * @author      : alex (alexbora@gmail.com)
 * @file        : fann_execute
 * @created     : Marţi Noi 14, 2023 17:12:43 EET
 */

#include <fann.h>
#include <floatfann.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
  struct fann *ann;
  ann = fann_create_from_file("xor_float.net");

  fann_type input[2][26] = {
      {0.080795, 0.016998, 0.032617, 0.034025, 0.118796, 0.019005, 0.022829,
       0.043613, 0.070941, 0.004223, 0.011614, 0.047893, 0.027252, 0.073062,
       0.078912, 0.017074, 0.001227, 0.063369, 0.056597, 0.082897, 0.033245,
       0.009826, 0.020070, 0.001845, 0.031161, 0.000114},
      {0.051676,    0.020249,   0.032921,    0.054266, 3779.661377, 0.047347,
       1501.974731, 0.077085,   0.087081,    0.001996, 2441.851318, 0.069850,
       3779.506592, 0.150882,   1500.811523, 0.038883, 2441.822266, 0.101588,
       1500.843628, 0.054730,   0.042959,    0.009463, 2441.855713, 0.032975,
       19.255980,   2380.679688}};

  fann_type *calc_out;

  /* input[1] = 1; */
  calc_out = fann_run(ann, input[0]);
  printf("xor test: %f\n", calc_out[0]);
  calc_out = fann_run(ann, input[1]);
  printf("xor test: %f\n", calc_out[0]);

  fann_destroy(ann);
  return 0;
}
