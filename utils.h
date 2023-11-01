/**
 * @author      : alex (alexbora@gmail.com)
 * @file        : utils
 * @created     : Miercuri Noi 01, 2023 16:10:22 EET
 */

#ifndef UTILS_H

#define UTILS_H

static int binary_search(int array[], int x, int low, int high) {
  // Repeat until the pointers low and high meet each other
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
#endif /* end of include guard UTILS_H */

