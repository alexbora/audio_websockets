/**
 * @author      : alex (alexbora@gmail.com)
 * @file        : utils
 * @created     : Miercuri Noi 01, 2023 16:10:22 EET
 */

#ifndef UTILS_H

#define UTILS_H

static inline int binary_search(char* array, char x, int low, int high) {
  while (low <= high) {
    int mid = low + (high - low) / 2;

    if (array[mid] == x) return mid;

    if (array[mid] < x)
      low = mid + 1;
    else
      high = mid - 1;
  }
  return -1;
}

static int binarySearch(char* c, char letter, unsigned len) {
  int lo, mid, hi;
  lo = 0;
  hi = len - 1;
  while (lo <= hi) {
    mid = lo + (hi - lo) / 2;
    if (c[mid] == letter)
      return mid;
    else if (c[mid] > letter)
      hi = mid - 1;
    else
      lo = mid + 1;
  }
  return -1;
}

#endif /* end of include guard UTILS_H */

