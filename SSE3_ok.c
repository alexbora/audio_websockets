/*
 * @Author: Alex Bora
 * @Date:   2023-11-09 19:43:16
 * @Last Modified by:   a049689
 * @Last Modified time: 2023-11-11 19:04:18
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <io.h>
#include <winsock2.h>
#define close closesocket
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <unistd.h>
#define SOCKET_ERROR -1
#define closesocket close
#define WSAGetLastError() errno
#define WSACleanup() __asm("nop");
#endif

#include <fcntl.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
// #pragma comment(lib,"ws2_32.lib") //Winsock Library

char *base64_encode(const char *input, int length) {
  BIO *bio, *b64;
  BUF_MEM *bptr;

  b64 = BIO_new(BIO_f_base64());
  bio = BIO_new(BIO_s_mem());
  bio = BIO_push(b64, bio);

  BIO_write(bio, input, length);
  BIO_flush(bio);

  BIO_get_mem_ptr(bio, &bptr);

  char *buff = (char *)malloc(bptr->length);
  if (buff == NULL) {
    perror("Memory allocation error");
    exit(EXIT_FAILURE);
  }

  memcpy(buff, bptr->data, bptr->length - 1);
  buff[bptr->length - 1] = 0;

  BIO_free_all(bio);

  return buff;
}
void initOpenSSL() {
  SSL_library_init();
  SSL_load_error_strings();
  OpenSSL_add_all_algorithms();
}

void cleanupOpenSSL() { EVP_cleanup(); }
void printOpenSSLError() { ERR_print_errors_fp(stderr); }

int sniCallback(SSL *ssl, int *ad, void *arg) {
  const char *hostname = SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name);
  if (hostname) {
    printf("SNI Hostname: %s\n", hostname);
  }
  return SSL_TLSEXT_ERR_OK;
}
int setSocketNonBlocking(int sockfd) {
#ifdef _WIN32
  unsigned long nonBlocking = 1;
  return ioctlsocket(sockfd, FIONBIO, &nonBlocking);
#else
  return fcntl(sockfd, F_SETFL, O_NONBLOCK);
#endif
}

int getHostName(char *buffer, size_t size) {
#ifdef _WIN32
  // For Windows, use the GetComputerName function
  DWORD bufferSize = (DWORD)size;
  if (!GetComputerName(buffer, &bufferSize)) {
    fprintf(stderr, "Error getting host machine name on Windows\n");
    return 0;
  }
  return 1;
#else
  // For Unix-like systems, use the gethostname function
  if (gethostname(buffer, size) != 0) {
    perror("Error getting host machine name on Unix-like system");
    return 0;
  }
  return 1;
#endif
}

void info_callback(const SSL *ssl, int type, int val) {
  (void)ssl;  // Suppress unused parameter warning

  const char *type_str;
  const char *val_str;

  switch (type) {
    case SSL_CB_LOOP:
      type_str = "LOOP";
      break;
    case SSL_CB_EXIT:
      type_str = "EXIT";
      break;
    case SSL_CB_READ:
      type_str = "READ";
      break;
    case SSL_CB_WRITE:
      type_str = "WRITE";
      break;
    case SSL_CB_ALERT:
      type_str = "ALERT";
      break;
    case SSL_CB_HANDSHAKE_START:
      type_str = "HANDSHAKE START";
      break;
    case SSL_CB_HANDSHAKE_DONE:
      type_str = "HANDSHAKE DONE";
      break;
    default:
      type_str = "UNKNOWN";
      break;
  }

  switch (val) {
    case SSL_CB_LOOP:
      val_str = "LOOP";
      break;
    case SSL_CB_EXIT:
      val_str = "EXIT";
      break;
    case SSL_CB_READ:
      val_str = "READ";
      break;
    case SSL_CB_WRITE:
      val_str = "WRITE";
      break;
    case SSL_CB_ALERT:
      val_str = "ALERT";
      break;
    case SSL_CB_HANDSHAKE_START:
      val_str = "HANDSHAKE START";
      break;
    case SSL_CB_HANDSHAKE_DONE:
      val_str = "HANDSHAKE DONE";
      break;
    default:
      val_str = "UNKNOWN";
      break;
  }

  printf("SSL Info Callback - Type: %s, Val: %s\n", type_str, val_str);
}

typedef struct {
  int sock;
  SSL *ssl;
} Connection;

Connection init(int sock, SSL *ssl) {
  Connection conn = {sock, ssl};
  return conn;
}

void handleErrors() {
  ERR_print_errors_fp(stderr);
  exit(EXIT_FAILURE);
}

#ifndef _WIN32
int waitForData(int sockfd) {
  fd_set readfds;
  struct timeval timeout;

  FD_ZERO(&readfds);
  FD_SET(sockfd, &readfds);

  timeout.tv_sec = 5;  // Set the timeout (5 seconds in this example)
  timeout.tv_usec = 0;

  return select(sockfd + 1, &readfds, NULL, NULL, &timeout);
}
#endif

#ifdef _WIN32
int waitForDataWindows(SOCKET sockfd, WSAEVENT event) {
  DWORD result = WSAWaitForMultipleEvents(1, &event, FALSE, INFINITE, FALSE);
  if (result == WSA_WAIT_FAILED) {
    handleErrors();
  }
  return (result == WSA_WAIT_TIMEOUT) ? 0 : 1;
}
#endif

int boyerMooreSearch(const char *text, const char *pattern) {
  int textLength = strlen(text);
  int patternLength = strlen(pattern);

  // Initialize bad character heuristic array
  int badChar[256];
  for (int i = 0; i < 256; i++) {
    badChar[i] = patternLength;
  }
  for (int i = 0; i < patternLength - 1; i++) {
    badChar[(unsigned char)pattern[i]] = patternLength - 1 - i;
  }

  // Search using Boyer-Moore
  int i = patternLength - 1;
  while (i < textLength) {
    int j = patternLength - 1;
    while (j >= 0 && text[i] == pattern[j]) {
      i--;
      j--;
    }
    if (j < 0) {
      // Pattern found
      return i + 1;
    } else {
      // Shift based on bad character heuristic
      i += badChar[(unsigned char)text[i]];
    }
  }

  // Pattern not found
  return -1;
}

int main() {
  Connection conn = init(1, NULL);

  char hostName[256];  // Adjust the size as needed
  if (getHostName(hostName, sizeof(hostName))) {
    printf("Host machine name: %s\n", hostName);
  }

  initOpenSSL();
  // Initialize Winsock for Windows
#ifdef _WIN32
  WSADATA wsa;
  if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
    fprintf(stderr, "Failed to initialize Winsock\n");
    return 1;
  }
#endif

  // Create a socket
  int sockfd;
#ifdef _WIN32
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
#else
  sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#endif

  if (sockfd < 0) {
    perror("Error in socket");
    cleanupOpenSSL();
    WSACleanup();
    return 1;
  }
  // Set up proxy server address
  struct sockaddr_in proxyAddress;
  proxyAddress.sin_family = AF_INET;
  proxyAddress.sin_port =
      htons(8080);  // Proxy port (replace with your proxy port)

  // Resolve hostname to IP address
  struct hostent *he = gethostbyname(
      "pxgot1-onprem.srv.volvo.com");  // Replace with your proxy hostname
  if (he == NULL) {
    fprintf(stderr, "Failed to resolve proxy hostname\n");
    closesocket(sockfd);
    WSACleanup();
    return 1;
  }
  memcpy(&proxyAddress.sin_addr.s_addr, he->h_addr, he->h_length);

  if (connect(sockfd, (struct sockaddr *)&proxyAddress, sizeof(proxyAddress)) ==
      SOCKET_ERROR) {
    fprintf(stderr, "Error in connection to the proxy: %d\n",
            WSAGetLastError());
    closesocket(sockfd);
    WSACleanup();
    return 1;
  }

  // Create a BIO for the socket
  BIO *bio = BIO_new_socket(sockfd, BIO_NOCLOSE);

  // Create an SSL context
  SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
  if (!ctx) {
    fprintf(stderr, "Error creating SSL context\n");
    printOpenSSLError();
    close(sockfd);
    BIO_free_all(bio);
    cleanupOpenSSL();
    WSACleanup();
    return 1;
  }

  SSL_CTX_set_cipher_list(ctx, "ALL");
  // Set SNI callback
  // SSL_CTX_set_tlsext_servername_callback(ctx, sniCallback);

  SSL_CTX_set_info_callback(ctx, info_callback);

  // Create an SSL structure
  SSL *ssl = SSL_new(ctx);
  if (!ssl) {
    fprintf(stderr, "Error creating SSL structure\n");
    printOpenSSLError();
    SSL_CTX_free(ctx);
    close(sockfd);
    BIO_free_all(bio);
    cleanupOpenSSL();
    WSACleanup();
    return 1;
  }
  SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

  // Set the file descriptor for the SSL connection
  if (SSL_set_fd(ssl, sockfd) != 1) {
    fprintf(stderr, "Error setting file descriptor for SSL connection\n");
    printOpenSSLError();
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    close(sockfd);
    BIO_free_all(bio);
    cleanupOpenSSL();
#ifdef _WIN32
    WSACleanup();
#endif
    return 1;
  }

#ifdef USE_PROXY
  const char *username = "a049689", *password = "SummicronSummilux-50";
  char auth_string[256], auth_header[1024], *base64_auth;
  snprintf(auth_string, sizeof(auth_string), "%s:%s", username, password);
  base64_auth = base64_encode(auth_string, strlen(auth_string));
  snprintf(auth_header, sizeof(auth_header),
           "CONNECT antenne.de:443 HTTP/1.1\r\nHost: antenne.de\r\n");
  // snprintf(auth_header, sizeof(auth_header), "GET / HTTP/1.1\r\nHost:
  // example.org\r\n");
  sprintf(auth_header + strlen(auth_header),
          "Proxy-Authorization: Basic %s\r\n%s\r\n%s\r\n\r\n", base64_auth,
          "Proxy-Connection: Keep-Alive", "Content-Type: text/event-stream");
  send(sockfd, auth_header, strlen(auth_header), 0);
  char rsp[1024];
  recv(sockfd, rsp, sizeof(rsp) - 1, 0);

  puts(auth_header);
#endif
  const char *request =
      "GET /api/metadata/now/chillout HTTP/1.1\r\n"
      "Host: www.antenne.de\r\n"
      "Accept: text/event-stream\r\n"
      "\r\n";

  int sslStatus;
  while ((sslStatus = SSL_connect(ssl)) != 1) {
    int sslError = SSL_get_error(ssl, sslStatus);
    if (sslError == SSL_ERROR_WANT_READ || sslError == SSL_ERROR_WANT_WRITE) {
      // The SSL handshake is still in progress
      continue;
    } else {
      fprintf(stderr, "SSL handshake failed: %d\n", sslError);
      printOpenSSLError();
      SSL_free(ssl);
      SSL_CTX_free(ctx);
      close(sockfd);
      BIO_free_all(bio);
      cleanupOpenSSL();
      WSACleanup();
      return 1;
    }
  }

  SSL_write(ssl, request, strlen(request));
  memset(rsp, '\0', sizeof(rsp));
  SSL_read(ssl, rsp, sizeof(rsp));
  // puts(rsp);

  int res = boyerMooreSearch(rsp, "artist");
  if (res != -1) printf("Pattern found at position: %d\n%s\n", res, rsp + res);

  int bytes;
  //   while ((bytes = SSL_read(ssl, rsp, sizeof(rsp) - 1)) > 0) {
  //         rsp[bytes] = '\0';
  //         int sslError = SSL_get_error(ssl, bytes);
  //         printf("%d\n",sslError );
  //         printf("Received: %s\n", rsp);
  // }

#ifdef _WIN32
  // Create an event for waiting on the socket
  WSAEVENT event = WSACreateEvent();
  if (event == WSA_INVALID_EVENT) {
    handleErrors();
  }
  // Associate the event with the socket
  if (WSAEventSelect(SSL_get_fd(ssl), event, FD_READ) == SOCKET_ERROR) {
    handleErrors();
  }
#endif

#if 0
  while (1) {
#ifdef _WIN32
    int result = waitForDataWindows(SSL_get_fd(ssl), event);
#else
    int result = waitForData(SSL_get_fd(ssl));
#endif
    if (result) SSL_read(ssl, rsp, sizeof(rsp) - 1);
  }

  return 0;
#endif
  // const char *request = "GET / HTTP/1.1\r\nHost: www.example.com\r\n\r\n";

  // Send the request
  int bytesSent = send(sockfd, auth_header, strlen(auth_header), 0);
  if (bytesSent == SOCKET_ERROR) {
    fprintf(stderr, "Error sending request: %d\n", WSAGetLastError());
  } else if (bytesSent == 0) {
    fprintf(stderr, "Connection closed by the peer\n");
  } else {
    printf("Request sent successfully (%d bytes)\n", bytesSent);
  }

  char buffer[1024];
  int bytesRead;
  while ((bytesRead = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) > 0) {
    buffer[bytesRead] = '\0';
    printf("%s", buffer);
  }

  if (bytesRead == SOCKET_ERROR) {
    fprintf(stderr, "Error receiving response: %d\n", WSAGetLastError());
  }

  //     if (write(sockfd, auth_header, strlen(auth_header)) == -1) {
  //         fprintf(stderr, "SSE request failed: %d\n", WSAGetLastError());
  //         // return 1;
  //     }

  //      const char* sseRequest = "GET /api/metadata/now/chillout HTTP/1.1\r\n"
  //                              "Host: antenne.de\r\n"
  //                              "Accept: text/event-stream\r\n"
  //                              "Cache-Control: no-cache\r\n"
  //                              "Connection: keep-alive\r\n"
  //                              "Content-Type: text/event-stream\r\n\r\n";

  // if (write(sockfd, sseRequest, strlen(sseRequest)) == -1) {
  //         perror("SSE request failed");
  //         // return 1;
  //     }
  // Perform other operations with the socket

  // Close the socket and cleanup when done
  close(sockfd);
  WSACleanup();

  return 0;
}
