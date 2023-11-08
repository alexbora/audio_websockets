#include <ctype.h>

#include <fstream>
#include <iostream>

/* #include <vector> */
#include <fann.h>
void generate_frequencies(const char *filename, float *frequencies) {
  std::ifstream infile(filename);
  if (!infile) printf("Cannot open input file %s\n", filename);

  unsigned int letter_count[26];
  /* std::vector<unsigned int> letter_count(26, 0); */
  unsigned int num_characters = 0;
  char c;
  while (infile.get(c)) {
    c = tolower(c);
    if (c >= 'a' && c <= 'z') {
      letter_count[c - 'a']++;
      num_characters++;
    }
  }
  if (!infile.eof()) puts("Something strange happened");
  for (unsigned int i = 0; i != 26; i++) {
    frequencies[i] = letter_count[i] / (double)num_characters;
  }
}

void train() {
  struct fann *ann = fann_create_standard(3, 2, 3, 1);

  /* struct fann *ann = fann_create_standard(1, 0.7, 3, 26, 13, 3); */
  fann_set_activation_function_hidden(ann, FANN_SIGMOID_SYMMETRIC);
  fann_set_activation_function_output(ann, FANN_SIGMOID_SYMMETRIC);

  fann_train_on_file(ann, "frequencies.data", 200, 10, 0.0001);

  fann_save(ann, "xxx.net");

  fann_destroy(ann);
}

int main(int argc, char *argv[]) {
  if (argc != 2) printf("Remember to specify an input file");
  float frequencies[26];
  generate_frequencies(argv[1], frequencies);

  for (unsigned int i = 0; i != 26; i++) {
    std::cout << frequencies[i] << ' ';
  }
  std::cout << std::endl;

  FILE *fp = fopen("frequencies.data", "w");
  int arr[] = {12, 26, 3};
  fprintf(fp, "%d %d %d\n\n", arr[0], arr[1], arr[2]);
  for (unsigned int i = 0; i != 26; i++) {
    fprintf(fp, "%f ", frequencies[i]);
  }
  fprintf(fp, "\n");
  fclose(fp);

  return 0;
}
