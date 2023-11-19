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

typedef struct {
  unsigned char *data;
  size_t len;
} Buffer;

typedef struct {
  Buffer artist;
  Buffer title;
} Metadata;

void init_openssl();
void cleanup_openssl();
SSL_CTX *create_context();
int create_socket(const char *host, const char *port);
SSL *create_ssl(SSL_CTX *ctx, int sockfd);
void ssl_handshake(SSL *ssl);
void send_request(SSL *ssl, const char *path, const char *host);
void read_response(SSL *ssl, Metadata *);
int boyerMooreSearch(unsigned char *text, int textLength, char *pattern,
                     int patternLength);

void flushSocket(int sockfd) {
  char flushBuffer[1024];
  while (recv(sockfd, flushBuffer, sizeof(flushBuffer), MSG_DONTWAIT) > 0)
    ;
}

int main() {
  Metadata metadata;

  init_openssl();

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
    read_response(ssl, &metadata);
    printf("Now playing\tArtist: %s\tTitle: %s\n", metadata.artist.data,
           metadata.title.data);
    sleep(1);
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
  int sockfd = -1;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if (getaddrinfo(host, port, &hints, &result) != 0) {
    fprintf(stderr, "Error resolving address\n");
    exit(EXIT_FAILURE);
  }

  for (rp = result; rp != NULL; rp = rp->ai_next) {
    sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sockfd == -1) continue;

    if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) != -1) break;  // Success

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
  char request[1024] = {0};

  snprintf(request, sizeof(request),
           "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: keep-alive\r\n\r\n",
           path, host);

  if (SSL_write(ssl, request, strlen(request)) < 0) {
    fprintf(stderr, "Error sending HTTP request\n");
    exit(EXIT_FAILURE);
  }
}

void read_response(SSL *ssl, Metadata *metadata) {
  unsigned char buffer[1024] = {0};
  memset(buffer, 0, sizeof(buffer));
  int bytes_received;

  // int sockfd = SSL_get_fd(ssl); // Get socket connection
  // flushSocket(sockfd);

  bytes_received = SSL_read(ssl, buffer, sizeof(buffer) - 1);

  if (bytes_received > 0) {
    buffer[bytes_received] = '\0';

    int artistIndex =
        boyerMooreSearch(buffer, bytes_received, "artist", strlen("artist"));

    /* Monolink","title":"Harlem River (Radio Edit)", */

    if (artistIndex != -1) {
      /* printf("Received response:\n%s\n", buffer + artistIndex); */
      unsigned char *p = buffer + artistIndex;
      while (*p++ != ':')
        ;
      metadata->artist.data = p + 1;
      unsigned char *r = p + 1;
      int i = 0;
      while (*r++ != '"') i++;
      metadata->artist.data[i] = '\0';
      metadata->artist.len = i;

      r = p + i;
      i = 0;

      while (*r++ != ':')
        ;
      metadata->title.data = r + 1;
      unsigned char *q = r + 1;
      while (*q++ != '"') i++;
      metadata->title.data[i] = '\0';
      metadata->title.len = i;
    }

  } else if (bytes_received == 0) {
    fprintf(stderr, "Server closed the connection\n");
    exit(EXIT_SUCCESS);
  } else {
    fprintf(stderr, "Error reading response\n");
    exit(EXIT_FAILURE);
  }
}

int boyerMooreSearch(unsigned char *text, int textLength, char *pattern,
                     int patternLength) {
  int badChar[256];

  // Preprocess the bad character heuristic array
  for (int i = 0; i < 256; i++) badChar[i] = patternLength;

  for (int i = 0; i < patternLength - 1; i++)
    badChar[(unsigned char)pattern[i]] = patternLength - 1 - i;

  // Boyer-Moore search algorithm
  int s = 0;  // Shift of the pattern with respect to the text

  while (s <= (textLength - patternLength)) {
    int j = patternLength - 1;

    // Keep reducing the index j of the pattern while characters of the pattern
    // and text are matching
    while (j >= 0 && pattern[j] == text[s + j]) j--;

    // If the pattern is present at the current shift, return the index
    if (j < 0) {
      return s;
    } else {
      // Shift the pattern based on the bad character heuristic
      s += badChar[(unsigned char)text[s + j]] > patternLength - 1 - j
               ? badChar[(unsigned char)text[s + j]]
               : patternLength - 1 - j;
    }
  }

  return -1;  // Pattern not found
}
