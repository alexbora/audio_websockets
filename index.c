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

char p[256];
size_t idx;

char *r;
volatile static int indx;
extern char *icy;

size_t jump(char *arr, size_t interval, size_t size, char **icy) {
  size_t over = 0;

  indx += size;
  if (indx < interval) return 0;

  int o = indx - interval - 1;
  int x = size + o;
  printf("%d\n", arr[x] + '0');

  /* printf("INTERVAL: %d\n", arr[indx - interval] + '0'); */

  *icy = &arr[interval];
  /* printf("icy: \n%s\n", &arr[interval]); */
  over = indx - interval - arr[interval] * 1 /*16*/ + '0';
  indx = -over;

  printf("\nicy: %s\n", &arr[size - over]);
  return size - over;
}

#ifndef NOMAIN
int main(int argc, char *argv[]) {
  r = malloc(256);
  memcpy(r, "0000100000", 10);

  size_t o = jump(r, 4, sizeof("0123456789") - 1, NULL);

  printf("jump: %ld jumped: %s idx: %d\n", o, &r[o], indx);
  memset(r, '\0', 256);
  memcpy(r, "123451", 6);

  printf("jump: %ld jumped: %s idx: %d\n", o, &r[o], indx);
  return 0;
}
#endif
