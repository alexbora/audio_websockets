/**
 * @author      : alex (alex@T400)
 * @file        : ssl_read
 * @created     : Sunday Nov 12, 2023 14:36:36 UTC
 */

#ifndef _WIN32
#define nix
#endif

#ifdef WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <winsock2.h>
#if _WIN32_WINNT == 0x0601  // Windows 7
#define WINDOWS_CPU_GROUPS_ENABLED 1
#endif
#define sleep(secs) Sleep((secs)*1000)
#include <sys/resource.h>
#ifdef _MSC_VER
#define snprintf(...) _snprintf(__VA_ARGS__)
#define strdup(...) _strdup(__VA_ARGS__)
#define strncasecmp(x, y, z) _strnicmp(x, y, z)
#define strcasecmp(x, y) _stricmp(x, y)
#define __func__ __FUNCTION__
#define __thread __declspec(thread)
#define _ALIGN(x) __declspec(align(x))
typedef int ssize_t;
#include <Mmsystem.h>
#pragma comment(lib, "winmm.lib")
#endif
#endif

#ifndef _MSC_VER
#define _ALIGN(x) __attribute__((aligned(x)))
#endif

#undef unlikely
#undef likely
#if defined(__GNUC__) && (__GNUC__ > 2) && defined(__OPTIMIZE__)
#define unlikely(expr) (__builtin_expect(!!(expr), 0))
#define likely(expr) (__builtin_expect(!!(expr), 1))
#else
#define unlikely(expr) (expr)
#define likely(expr) (expr)
#endif

#ifdef nix
#include <arpa/inet.h>
#include <netdb.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#ifdef _MSC_VER
#define inline __inline  // for MSVCi
#elif defined(__GNUC__)
#define inline __attribute__((always_inline)) inline
#endif

#define swap_vars(a, b) \
  a ^= b;               \
  b ^= a;               \
  a ^= b;

#define HOST "antenne.de"
#define PORT "443"
#define PATH "/api/metadata/now/chillout"

struct thr_info {
  int id;
  pthread_t pth;
  pthread_attr_t attr;
};

typedef struct {
  unsigned char *data;
  size_t len;
} Buffer;

typedef struct {
  Buffer artist;
  Buffer title;
} Metadata;

static void init_openssl();
static void cleanup_openssl();
static SSL_CTX *create_context();
static int create_socket(const char *host, const char *port);
static SSL *create_ssl(SSL_CTX *ctx, int sockfd);
static void ssl_handshake(SSL *ssl);
static void send_request(SSL *ssl, const char *path, const char *host);
static void read_response(SSL *ssl, Metadata *);
static int boyerMooreSearch(unsigned char *, int, char *, int);

static void flushSocket(int sockfd) {
  char flushBuffer[1024];
  while (recv(sockfd, flushBuffer, sizeof(flushBuffer), MSG_DONTWAIT) > 0)
    ;
}

static int thread_create(struct thr_info *, void *);

static void get_meta(void *arg) {
  struct thr_info *thr = (struct thr_info *)arg;
  Metadata metadata;

  pthread_detach(pthread_self());
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
    /* printf("Now playing...\t%s\t%s\n", metadata.artist.data, */
    /* metadata.title.data); */
    sleep(1);
  }

  // Clean up
  SSL_shutdown(ssl);
  close(sockfd);
  SSL_free(ssl);
  SSL_CTX_free(ctx);
  cleanup_openssl();
};

static inline int meta_() {
  struct thr_info thr;
  thr.id = 2;
  thr.pth = 0;
  pthread_attr_init(&thr.attr);
  pthread_attr_setdetachstate(&thr.attr, PTHREAD_CREATE_DETACHED);

  thread_create(&thr, get_meta);

  while (1)
    ;

  pthread_cancel(thr.pth);

  return 0;
}

int meta(void) { return meta_(); };

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
  if (unlikely(ctx == NULL)) {
    fprintf(stderr, "Error creating SSL context\n");
    exit(EXIT_FAILURE);
  }
  return ctx;
}

int create_socket(const char *host, const char *port) {
  int sockfd = -1;
  struct addrinfo hints = {0}, *result = NULL, *rp = NULL;
  /* memset(&hints, 0, sizeof(struct addrinfo)); */
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

Metadata get_name(int *index, unsigned char *buf) {
  Metadata metadata;
  int i = 0;

  unsigned char *p = buf + *index;
  while (*p++ != ':')
    ;
  metadata.artist.data = p + 1;
  unsigned char *r = p + 1;
  while (*r++ != '"') i++;
  metadata.artist.data[i] = '\0';
  metadata.artist.len = i;
  *index = i;
  return metadata;
}

Metadata get_title(int *index, unsigned char *buf) {
  Metadata metadata;
  int i = 0;

  unsigned char *p = buf + *index;
  while (*p++ != ':')
    ;
  metadata.title.data = p + 1;
  unsigned char *r = p + 1;
  while (*r++ != '"') i++;
  metadata.title.data[i] = '\0';
  metadata.title.len = i;
  *index = i;
  return metadata;
}

static inline void split_string(unsigned char *in) {
  unsigned char *start = in;
  unsigned char *end = in;

  Metadata metadata;

  size_t count = 0;

  // Count the number of substrings
  while (*end) {
    while (*end && *end != '"') end++;
    count++;
    end = *end ? end + 1 : end;
  }

  start = end = in;

  printf("Number of substrings: %zu\n", count);

  count = count > 2 ? 2 : count;

  unsigned char **substrings = malloc(sizeof(unsigned char *) * count);
  for (size_t i = 0; i < count; i++) {
    while (*end && *end != '"') end++;

    substrings[i] = start;
    start = end + (*end != '\0');
    end = start;
#if 0
    while (*end) {
      while (*end && *end != '"') end++;
      /* printf("Substring:"); */
      while (start < end) putchar(*start++);
      printf("\n");
      /* start[end - start] = '\0'; */

      /* printf("string:%s\n", (const char *)start); */

      /* metadata.artist.data = start; */
      /* metadata.artist.len = end - start; */
      /* metadata.artist.data[end - start] = '\0'; */

      /* printf("ARTIST: %s\n", metadata.artist.data); */

      start = end + (*end != '\0');
      end = start;
#endif
  }
  printf("Substring: %s\n", (const char *)substrings[0]);
}

void read_response(SSL *ssl, Metadata *metadata) {
  int i = 0, j = i;
  unsigned char buffer[1024 * 2] = {0};

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
      /* *metadata = get_name(&artistIndex, buffer); */
      /* *metadata = get_title(&artistIndex, buffer + artistIndex); */

      /* printf("Received response:\n%s\n", buffer + artistIndex); */
      unsigned char *p = buffer + artistIndex;
      split_string(p);

      while (*p++ != ':')
        ;
      metadata->artist.data = p + 1;
      unsigned char *r = p + 1;
      while (*r++ != '"') i++;
      metadata->artist.data[i] = '\0';
      metadata->artist.len = i;

      r = p + i;

      while (*r++ != ':')
        ;
      metadata->title.data = r + 1;
      unsigned char *q = r + 1;
      while (*q++ != '"') j++;
      metadata->title.data[j - 1] = '\0';
      metadata->title.len = j;
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

    // Keep reducing the index j of the pattern while characters of the
    // pattern and text are matching
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

static int thread_create(struct thr_info *thr, void *func) {
  int err = 0;
  pthread_attr_init(&thr->attr);
  err = pthread_create(&thr->pth, &thr->attr, func, thr);
  pthread_attr_destroy(&thr->attr);
  return err;
}
