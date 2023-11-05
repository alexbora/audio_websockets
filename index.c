/**
 * @author      : alex (alexbora@gmail.com)
 * @file        : index
 * @created     : Duminică Oct 08, 2023 09:38:43 EEST
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __APPLE__
#include <sys/wait.h>
#endif
#include <unistd.h>

size_t indx;

typedef struct {
  char title[256];
  size_t len;
} Buffer;

Buffer playing(char *arr) {
  Buffer buffer;
  char t[256];

  if (arr) {
    int i = 0;
    char *p = arr + strlen("StreamTitle='");
    while (*p != ';')
      t[i++] = *p++;
    printf("Now playing: %s\n", t   );
  }

  return buffer;
}

char title[256];

size_t jump(char *arr, size_t interval, size_t size, char **icy) {
  size_t over = 0;

  int tmp = indx;
  indx += size;
  if (indx >= 16000) {
    /* printf("1: %d %s\n", arr[size - indx + 16000], &arr[size - indx +
     * 16000]); */
    /* printf("indx %d size %ld size - indx %ld diff %ld\n", indx, size, */
    /* size - indx, 16000 + (size - indx)); */
    /* printf("%d\n", 16000 - tmp); */
    /* printf("%d\n", 16000 - indx); */
    /* indx = -(16000 - indx - arr[16000 - indx] * 16 - 1); */

    indx = size - (16000 - tmp) - arr[16000 - tmp] * 16 - 1;
    /* printf("%s\n", &arr[16000 - tmp]); */

    /* printf("icy %d tmp %d diff %d over %d %s\n", arr[16000 - tmp], tmp, */
    /* 16000 - tmp, arr[16000 - tmp] * 16 + 1, &arr[16000 - tmp]); */

// if(arr[16000-tmp]>0)
     printf("icy %d %s\n", arr[16000 - tmp], &arr[16000 - tmp] + strlen("StreamTitle = ")); 

// if(arr[16000-tmp]>0)
// playing(&arr[16000-tmp]);
    memset(title, 0, sizeof(title));
#if 0
    if (arr[16000 - tmp] + '0' != '0') {
      int i = 0;
      char *p = &arr[16000 - tmp] + strlen("StreamTitle='");
      while (*p != ';')
        title[i++] = *p++;
      printf("Now playing: %s\n", title);
    }
    #endif
    /* printf("index: %ld  === %ld\n", size - indx + 16000, size); */
    /* printf("xxx: %ld  === %d\n", 16000 - size + indx, indx); */
    return (arr[16000 - tmp] * 16 + 1);
  }

  return 0;

  indx += size;
  if (indx < interval)
    return 0;

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
