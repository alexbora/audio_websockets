/**
 * @author      : alex (alex@T400)
 * @file        : ssl_read
 * @created     : Sunday Nov 12, 2023 14:36:36 UTC
 */

#include <arpa/inet.h>
#include <netdb.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define HOST "antenne.de"
#define PORT "443"
#define PATH "/api/metadata/now/chillout"

void init_openssl();
void cleanup_openssl();
SSL_CTX *create_context();
int create_socket(const char *host, const char *port);
SSL *create_ssl(SSL_CTX *ctx, int sockfd);
void ssl_handshake(SSL *ssl);
void send_request(SSL *ssl, const char *path, const char *host);
void read_response(SSL *ssl);

int main() {
  /* init_openssl(); */

  // Create SSL context and set up the socket
  SSL_CTX *ctx = create_context();
  int sockfd = create_socket(HOST, PORT);
  SSL *ssl = create_ssl(ctx, sockfd);

  // Perform SSL/TLS handshake
  ssl_handshake(ssl);

  // Send an HTTP GET request
  send_request(ssl, PATH, HOST);

  // Read the response every 5 seconds
  while (1) {
    send_request(ssl, PATH, HOST);
    read_response(ssl);
    sleep(5);
  }

  // Clean up
  SSL_shutdown(ssl);
  close(sockfd);
  SSL_free(ssl);
  SSL_CTX_free(ctx);
  cleanup_openssl();

  return 0;
}

void init_openssl() {
  SSL_library_init();
  OpenSSL_add_all_algorithms();
  SSL_load_error_strings();
}

void cleanup_openssl() {
  ERR_free_strings();
  EVP_cleanup();
}

SSL_CTX *create_context() {
  SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
  if (!ctx) {
    fprintf(stderr, "Error creating SSL context\n");
    exit(EXIT_FAILURE);
  }
  return ctx;
}

int create_socket(const char *host, const char *port) {
  struct addrinfo hints, *result, *rp;
  int sockfd;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if (getaddrinfo(host, port, &hints, &result) != 0) {
    fprintf(stderr, "Error resolving address\n");
    exit(EXIT_FAILURE);
  }

  for (rp = result; rp != NULL; rp = rp->ai_next) {
    sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sockfd == -1)
      continue;

    if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) != -1)
      break; // Success

    close(sockfd);
  }

  if (rp == NULL) {
    fprintf(stderr, "Error creating or connecting socket\n");
    exit(EXIT_FAILURE);
  }

  freeaddrinfo(result);

  return sockfd;
}

SSL *create_ssl(SSL_CTX *ctx, int sockfd) {
  SSL *ssl = SSL_new(ctx);
  if (!ssl) {
    fprintf(stderr, "Error creating SSL structure\n");
    exit(EXIT_FAILURE);
  }

  SSL_set_fd(ssl, sockfd);

  return ssl;
}

void ssl_handshake(SSL *ssl) {
  if (SSL_connect(ssl) != 1) {
    fprintf(stderr, "SSL/TLS handshake failed\n");
    exit(EXIT_FAILURE);
  }
}

void send_request(SSL *ssl, const char *path, const char *host) {
  char request[1024];
  snprintf(request, sizeof(request),
           "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: keep-alive\r\n\r\n",
           path, host);

  if (SSL_write(ssl, request, strlen(request)) < 0) {
    fprintf(stderr, "Error sending HTTP request\n");
    exit(EXIT_FAILURE);
  }
}

void read_response(SSL *ssl) {
  char buffer[1024];
  int bytes_received;

  bytes_received = SSL_read(ssl, buffer, sizeof(buffer) - 1);

  if (bytes_received > 0) {
    buffer[bytes_received] = '\0';
    printf("Received response:\n%s\n", buffer);
  } else if (bytes_received == 0) {
    fprintf(stderr, "Server closed the connection\n");
    exit(EXIT_SUCCESS);
  } else {
    fprintf(stderr, "Error reading response\n");
    exit(EXIT_FAILURE);
  }
}
