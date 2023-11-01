/**
 * @author      : alex (alexbora@gmail.com)
 * @file        : test_main
 * @created     : Miercuri Noi 01, 2023 16:16:13 EET
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

int main(int argc, char *argv[]) {
  char *arr = "testsxx;ssssssss";

  int x = binary_search(arr, ';', 0, strlen(arr));

  printf("%d\n", x);

  x = binarySearch(arr, ';', strlen(arr));

  printf("%d\n", x);
  return 0;
}

