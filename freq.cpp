#include <ctype.h>

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
  if (argc != 2) printf("Remember to specify an input file");
  float frequencies[26];
  generate_frequencies(argv[1], frequencies);
  for (unsigned int i = 0; i != 26; i++) {
    std::cout << frequencies[i] << ' ';
  }
  std::cout << std::endl;
  return 0;
}
