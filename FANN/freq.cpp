#include <ctype.h>
#include <fann.h>

#include <fstream>
#include <iostream>
/* #include <vector> */

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

int main(int argc, char *argv[]) {
  /* if (argc != 2) printf("Remember to specify an input file"); */
  float frequencies[26];
  generate_frequencies("text_EN.txt", frequencies);

  FILE *fp = fopen("frequencies.txt", "w");

  fprintf(fp, "4 2 1\n");

  for (unsigned int i = 0; i != 26; i++) {
    fprintf(fp, "%f ", frequencies[i]);
  }
  fprintf(fp, "\n");
  fprintf(fp, "0");
  fprintf(fp, "\n\n");

  generate_frequencies("text_DE.txt", frequencies);
  for (unsigned int i = 0; i != 26; i++) {
    fprintf(fp, "%f ", frequencies[i]);
  }
  fprintf(fp, "\n");
  fprintf(fp, "1");
  fprintf(fp, "\n\n");

  fclose(fp);

  return 0;
}
