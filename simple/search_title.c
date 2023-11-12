/**
 * @author      : alex (alex@T400)
 * @file        : search_title
 * @created     : Sunday Nov 12, 2023 14:58:32 UTC
 */

// Function to pre-process the bad character heuristic array
void badCharHeuristic(char *artist, int size, int badChar[256]) {
  int i;

  // Initialize all occurrences as -1
  for (i = 0; i < 256; i++)
    badChar[i] = -1;

  // Fill the actual value of the last occurrence of a character
  for (i = 0; i < size; i++)
    badChar[(unsigned char)artist[i]] = i;
}

// Function to perform the Boyer-Moore search algorithm
int searchArtist(char *text, int n, char *artist) {
  int m = strlen(artist);

  int badChar[256];

  // Preprocess the bad character heuristic array
  badCharHeuristic(artist, m, badChar);

  int s = 0; // Shift of the pattern with respect to the text

  while (s <= (n - m)) {
    int j = m - 1;

    // Keep reducing the index j of the pattern while characters of the pattern
    // and text are matching
    while (j >= 0 && artist[j] == text[s + j])
      j--;

    // If the pattern is present at the current shift, print the index and
    // return
    if (j < 0) {
      printf("Artist found at index %d\n", s);
      return s;
    } else {
      // Shift the pattern based on the bad character heuristic
      s += (j - badChar[(unsigned char)text[s + j]]) > 0
               ? j - badChar[(unsigned char)text[s + j]]
               : 1;
    }
  }

  return -1; // Artist not found
}
