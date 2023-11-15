#include <curl/curl.h>
#include <mpg123.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Struct to hold received data
struct ResponseData {
  unsigned char *data;
  size_t size;
  mpg123_handle *handle;
};

// Callback function to receive data
size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
  size_t total_size = size * nmemb;
  struct ResponseData *response = (struct ResponseData *)userp;

  // Allocate memory for data
  response->data = realloc(response->data, response->size + total_size);

  // Copy received data to the struct
  memcpy(&(response->data[response->size]), contents, total_size);
  response->size += total_size;

  if (response->size > 16384) {
    response->size = 0;
  }  // return 0;
  printf("\n-----\n: %s\n%ld\n", &response->data[response->size - 2],
         response->size);

  size_t decode_size, done;
  unsigned char decode_buffer[8192];  // Adjust buffer size as needed

  mpg123_decode(response->handle, response->data, response->size, decode_buffer,
                sizeof(decode_buffer), &done);
  printf("??????????: %.4s %ld\n", decode_buffer, done);
  // puts("Process and play the decoded audio or perform other
  // actions");

  return total_size;
}

int main(void) {
  CURL *curl;
  CURLcode res;

  // Initialize cURL and MPG123
  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();

  mpg123_init();
  mpg123_handle *mpg123_handle = mpg123_new(NULL, NULL);

  if (!curl || !mpg123_handle) return -1;

  struct ResponseData response = {.data = NULL, .size = 0, mpg123_handle};

  mpg123_open_feed(response.handle);
  /* mpg123_feed(response.handle, response.data, response.size); */

  curl_easy_setopt(curl, CURLOPT_URL,
                   "https://s1-webradio.antenne.de/chillout/stream/mp3");
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

  // curl_easy_setopt(curl, CURLOPT_URL, "https://example.com/song.mp3");
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

  res = curl_easy_perform(curl);

  if (res != CURLE_OK) return -1;

  return 0;

  int err = 0;
  size_t done;

  // Decode the MP3 data using libmpg123
  err = mpg123_open_feed(mpg123_handle);

  if (err != MPG123_OK) return -1;

  err = mpg123_feed(mpg123_handle, response.data, response.size);

  if (err != MPG123_OK) return -1;

  size_t decode_size;
  unsigned char decode_buffer[8192];  // Adjust buffer size as needed

  while ((decode_size = mpg123_read(mpg123_handle, decode_buffer,
                                    sizeof(decode_buffer), &done)) > 0) {
    printf("%.4s %ld\n", decode_buffer, done);
    // puts("Process and play the decoded audio or perform other
    // actions");
  }

  // Clean up MPG123
  mpg123_close(mpg123_handle);
  mpg123_delete(mpg123_handle);

  // Clean up
  free(response.data);
  curl_easy_cleanup(curl);

  mpg123_exit();
  curl_global_cleanup();

  return 0;
}

