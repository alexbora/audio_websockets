#include <stdio.h>
#include <curl/curl.h>


#define SSE_CLIENT_USERAGENT     ("sse/" SSE_CLIENT_VERSION)

// This callback function is called by libcurl when SSE data is received.
size_t sseCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    // Process the SSE data received here as needed.
    // In this example, we print it to the console.
    printf("%.*s", (int)totalSize, (char*)contents);


    return totalSize;
}

int debug_callback(CURL *handle, curl_infotype type, char *data, size_t size, void *userptr) {
    // Print only header information
    if (type == CURLINFO_HEADER_OUT) {
        fwrite(data, 1, size, stderr);
    }
    return 0;
}

int main() {
    CURL* curl;
    CURLcode res;

    // Initialize libcurl
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize libcurl\n");
        return 1;
    }

    // Set the URL to your SSE endpoint
    curl_easy_setopt(curl, CURLOPT_URL, "https://www.antenne.de/api/metadata/now/chillout");
curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    // Set the callback function to process SSE data
    // curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, sseCallback);


  curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, debug_callback);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);


    // curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    // curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

curl_easy_setopt(curl, CURLOPT_PROXY, "pxgot1-onprem.srv.volvo.com:8080");
  curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, "a049689:SummicronSummilux-50");
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L); 
curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 0L);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 0L);
curl_easy_setopt(curl, CURLOPT_PIPEWAIT, 1L);

// curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
// curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);  // Adjust as needed
// curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);  // Adjust as needed

const char**  http_headers = (const char*[]){"Connection: keep-alive", "Cache-Control: no-cache", "Content-Type: text/event-stream", NULL};
  struct curl_slist *headers = NULL;
  while(http_headers && *http_headers)
    headers = curl_slist_append(headers, *http_headers++);
curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers); 

// struct curl_slist *header1 = NULL, *header2 = NULL;
  // header1 = curl_slist_append(header1, "Cache-Control: no-cache");
  // header2 = curl_slist_append(header2, "Upgrade: websocket"); 
  // curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header1);
  // curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header2);

 CURLM *multi_handle = curl_multi_init();
        curl_multi_add_handle(multi_handle, curl);

        int still_running;
        do {
            // Perform the request and handle the SSE stream
            curl_multi_perform(multi_handle, &still_running);
            if (still_running) {
                // You can add other processing or events here
            }
        } while (still_running);


    // Perform the SSE request
    // res = curl_easy_perform(curl);

    // if (res != CURLE_OK) {
    //     fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    // }

    // Cleanup and close libcurl
    curl_easy_cleanup(curl);

curl_multi_remove_handle(multi_handle, curl);
 curl_multi_cleanup(multi_handle);

        // Cleanup and close the curl handle
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);


    return 0;
}
