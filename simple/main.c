/**
 * @author      : alex (alexbora@gmail.com)
 * @file        : main
 * @created     : Duminică Noi 19, 2023 16:49:06 EET
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

struct thr_info {
  int id;
  pthread_t pth;
  pthread_attr_t attr;
};

void get_meta(void *);
int thread_create(struct thr_info *, void *);

int main(int argc, char *argv[]) {
  struct thr_info thr;
  thr.id = 2;
  thr.pth = 0;
  pthread_attr_init(&thr.attr);
  pthread_attr_setdetachstate(&thr.attr, PTHREAD_CREATE_DETACHED);

  thread_create(&thr, get_meta);

  while (1)
    ;

  pthread_cancel(thr.pth);

  return EXIT_SUCCESS;
}

