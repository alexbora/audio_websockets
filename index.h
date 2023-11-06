

#include <stddef.h>
#include <stdint.h>

#ifndef size_t
typedef unsigned long size_t;
#endif

typedef struct {
  uint8_t title[256];
  size_t len;
} Buffer;
