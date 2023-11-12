// main.c

#include <openssl/ssl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#define SERVER_PORT 80
#define SERVER_ADDRESS "antenne.de"
#define REQUEST_TEMPLATE                                                       \
  "GET /api/metadata/now/chillout HTTP/1.1\r\nHost: %s\r\n\r\n"

const char *host;

void parse_sse_message(const char *message) {
  // Simple SSE message parser
  printf("Received SSE message: %s\n", message);
}

void on_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
  if (nread > 0) {
    // SSE messages are typically separated by double newlines
    char *data = (char *)malloc(nread + 1);
    memcpy(data, buf->base, nread);
    data[nread] = '\0'; // Null-terminate the data

    char *message_start = data;
    char *newline = strchr(data, '\n');

    while (newline) {
      *newline = '\0'; // Null-terminate the message
      parse_sse_message(message_start);
      message_start = newline + 1; // Move to the start of the next message
      newline = strchr(message_start, '\n');
    }

    free(data);
  } else if (nread == UV_EOF) {
    printf("Server closed the connection.\n");
    uv_stop(uv_default_loop());
  } else if (nread < 0) {
    fprintf(stderr, "Read error: %s\n", uv_strerror(nread));
    uv_stop(uv_default_loop());
  }
}

void on_alloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
  buf->base = (char *)malloc(suggested_size);
  buf->len = suggested_size;
}

void on_write(uv_write_t *req, int status) {
  if (status < 0) {
    fprintf(stderr, "Write error: %s\n", uv_strerror(status));
  } else {
    printf("GET request sent successfully.\n");
  }

  uv_stream_t *stream = req->handle;

  // Start reading the response
  uv_read_start(stream, on_alloc, on_read);
}

void on_connect(uv_connect_t *req, int status) {
  if (status < 0) {
    fprintf(stderr, "Connection error: %s\n", uv_strerror(status));
    uv_stop(uv_default_loop());
    return;
  }

  uv_stream_t *stream = req->handle;

  /* printf("Connected to %s:%d\n", SERVER_ADDRESS, SERVER_PORT); */

  // Prepare the HTTP GET request
  char request[1024];
  snprintf(request, sizeof(request), REQUEST_TEMPLATE, SERVER_ADDRESS);

  // Write the HTTP GET request to the server
  uv_buf_t write_buf = uv_buf_init(request, strlen(request));
  uv_write_t write_req;
  uv_write(&write_req, stream, &write_buf, 1, on_write);
}

int main() {

  const char *hostname = "antenne.de";
  struct hostent *host_entry;

  // Use gethostbyname to resolve the IP address synchronously
  host_entry = gethostbyname(hostname);

  if (host_entry == NULL) {
    herror("gethostbyname");
    return 1;
  }
  host = malloc(sizeof(char) * 100);
  // Extract and print the resolved IP address
  struct in_addr **addr_list = (struct in_addr **)host_entry->h_addr_list;
  for (int i = 0; addr_list[i] != NULL; ++i) {
    printf("Resolved IP address: %s\n", inet_ntoa(*addr_list[i]));
    strcpy(host, inet_ntoa(*addr_list[i]));
  }

  uv_loop_t loop;
  uv_loop_init(&loop);

  uv_tcp_t socket;
  uv_tcp_init(&loop, &socket);

  struct sockaddr_in dest;
  uv_ip4_addr(host, SERVER_PORT, &dest);

  uv_connect_t connect_req;
  uv_tcp_connect(&connect_req, &socket, (struct sockaddr *)&dest, on_connect);

  uv_run(&loop, UV_RUN_DEFAULT);

  uv_loop_close(&loop);
  return 0;
}
