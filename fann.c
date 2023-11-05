

#include <fann.h>

#include "fann_data.h"
#include "fann_io.h"
#include "fann_train.h"

int main(int argc, char const *argv[]) {
  struct fann *ann = fann_create_standard(3, 2, 3, 1);

  fann_set_activation_function_hidden(ann, FANN_SIGMOID);
  fann_set_activation_function_output(ann, FANN_SIGMOID);

  struct fann_train_data *data = fann_create_train(4, 2, 1);

  data->input[0][0] = 0;
  data->input[0][1] = 1;
  /* data->output[0][0] = 2; */

  data->input[1][0] = 2;
  data->input[1][1] = 3;
  /* data->output[1][0] = 5; */

  fann_train_on_data(ann, data, 1000, 100, 0.01);

  fann_save(ann, "xor_float.net");

  fann_create_from_file("xor_float.net");
  float input_data[3];
  /* input_data[0] = 1; */
  /* input_data[1] = 2; */
  /* input_data[2] = 3; */

  fann_type *calc_out = fann_run(ann, input_data);
  printf("Prediction: %f\n", calc_out[0]);

  fann_destroy_train(data);
  fann_destroy(ann);

  return 0;
}
