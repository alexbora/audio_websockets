/**
 * @author      : alex (alex@T400)
 * @file        : ep
 * @created     : Sunday Nov 12, 2023 09:54:45 UTC
 */
#include <curl/curl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>
#define SSE_ENDPOINT "https://antenne.de/api/metadata/now/chillout"

// Callback to handle incoming data
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  size_t realsize = size * nmemb;
  printf("Received data: %.*s", (int)realsize, (char *)contents);
  return realsize;
}

int main() {
  CURL *curl;
  CURLcode res;

  // Initialize libcurl
  curl_global_init(CURL_GLOBAL_DEFAULT);

  // Create a libcurl handle
  curl = curl_easy_init();
  // Cleanup
  if (curl) {
    // Set the SSE endpoint URL
    curl_easy_setopt(curl, CURLOPT_URL, SSE_ENDPOINT);

    // Set the write callback to handle incoming data
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

    // Set libcurl to use a separate file descriptor for the connection
    curl_easy_setopt(curl, CURLOPT_PIPEWAIT, 1L);
    struct pollfd pfd;
    pfd.fd = curl_easy_getinfo(curl, CURLINFO_ACTIVESOCKET, &pfd.events);
    pfd.events |= POLLIN;

    while (1) {
      poll(&pfd, 1, -1);
      // Perform the SSE request
      res = curl_easy_perform(curl);

      if (res != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));

      // Cleanup
      curl_easy_cleanup(curl);
    }
  }
  // Cleanup libcurl
  curl_global_cleanup();

  return 0;
}
