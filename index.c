/**
 * @author      : alex (alexbora@gmail.com)
 * @file        : index
 * @created     : Duminică Oct 08, 2023 09:38:43 EEST
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int indx;

size_t jump(char *arr, size_t interval, size_t size, char **icy) {
  size_t over = 0;

  int tmp = indx;
  indx += size;
  if (indx >= 16000) {
    /* printf("1: %d %s\n", arr[size - indx + 16000], &arr[size - indx +
     * 16000]); */
    /* printf("indx %d size %ld size - indx %ld diff %ld\n", indx, size, */
    /* size - indx, 16000 + (size - indx)); */
    printf("%d\n", 16000 - tmp);
    printf("icy %d tmp %d diff %d over %d\n", arr[16000 - tmp], tmp,
           16000 - tmp, arr[16000 - tmp] * 16 + 1);
    /* printf("index: %ld  === %ld\n", size - indx + 16000, size); */
    /* printf("xxx: %ld  === %d\n", 16000 - size + indx, indx); */
  }

  return 0;

  indx += size;
  if (indx < interval) return 0;

  int o = indx - interval - 1;
  int x = size + o;
  printf("%d\n", arr[x] + '0');

  /* printf("INTERVAL: %d\n", arr[indx - interval] + '0'); */

  /* printf("icy: \n%s\n", &arr[interval]); */
  over = indx - interval - arr[interval] * 1 /*16*/ + '0';
  indx = -over;

  printf("\nicy: %s\n", &arr[size - over]);
  return size - over;
}

