/**
 * @author      : alex (alexbora@gmail.com)
 * @file        : test_main
 * @created     : Miercuri Noi 01, 2023 16:16:13 EET
 */

#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

int main(int argc, char *argv[]) {
  int arr[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};

  int x = binary_search(arr, 3, 0, 7);

  printf("%d\n", x);

  return 0;
}

