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

  char arr2[] = "test;tests";
  int n = sizeof(arr2) / sizeof(arr2[0]);

  char target = ';';
  int result = binary_search_gpt(arr2, 0, n - 1, target);

  if (result != -1)
    printf("%c found at index %d\n", target, result);
  else
    printf("%c not found in the array.\n", target);

  char *m = malloc(10);      // Allocate memory for 10 characters
  const char *txt = "test";  // Removed the single quotation mark

  if (m != NULL) {
    char *m_start = m;  // Store the start of the allocated memory

    while (*txt != '\0' &&
           m - m_start <
               10 - 1) {  // Ensure not to overrun the allocated memory
      *m++ = *txt++;
    }
    *m = '\0';  // Null-terminate the copied string

    printf("Copied string: %s\n",
           m_start);  // Print the copied string from the start

    free(m_start);  // Free the allocated memory

  } else {
    printf("Memory allocation failed.\n");
  }

  return 0;
}
