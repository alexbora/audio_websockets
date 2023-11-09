/*
* @Author: Alex Bora
* @Date:   2023-11-09 19:43:16
* @Last Modified by:   a049689
* @Last Modified time: 2023-11-09 21:11:19
*/
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <winsock2.h>
#include <ws2tcpip.h> 
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/pem.h>

#pragma comment(lib,"ws2_32.lib") //Winsock Library

int setSocketNonBlocking(SOCKET sockfd) {
    unsigned long nonBlocking = 1;
    return ioctlsocket(sockfd, FIONBIO, &nonBlocking);
}


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

void cleanupOpenSSL() {
    EVP_cleanup();
}
void printOpenSSLError() {
    ERR_print_errors_fp(stderr);
}


int main() {
	 initOpenSSL();

    // Initialize Winsock
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        fprintf(stderr, "Failed to initialize Winsock\n");
        return 1;
    }

    // Create a socket
    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET) {
        perror("Error in socket");
        WSACleanup();
        return 1;
    }

    // Set up proxy server address
    struct sockaddr_in proxyAddress;
    proxyAddress.sin_family = AF_INET;
    proxyAddress.sin_port = htons(8080);  // Proxy port (replace with your proxy port)

    // Resolve hostname to IP address
    struct hostent *he = gethostbyname("pxgot1-onprem.srv.volvo.com");  // Replace with your proxy hostname
    if (he == NULL) {
        fprintf(stderr, "Failed to resolve proxy hostname\n");
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }
    memcpy(&proxyAddress.sin_addr.s_addr, he->h_addr, he->h_length);

    // Connect to the proxy server
    if (connect(sockfd, (struct sockaddr *)&proxyAddress, sizeof(proxyAddress)) == SOCKET_ERROR) {
        fprintf(stderr, "Error in connection to the proxy: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }



	const char* username = "a049689", *password = "SummicronSummilux-50";
    char auth_string[256], auth_header[1024], *base64_auth;
    snprintf(auth_string, sizeof(auth_string), "%s:%s", username, password);
    base64_auth = base64_encode(auth_string, strlen(auth_string));
    snprintf(auth_header, sizeof(auth_header), "CONNECT HTTP/1.1\r\n");
    sprintf(auth_header+strlen(auth_header), "Proxy-Authorization: Basic %s\r\n%s\r\n\r\n", base64_auth, "Proxy-Connection: Keep-Alive");



 // const char *request = "GET /api/metadata/now/chillout HTTP/1.1\r\n"
 //                          "Host: www.antenne.de\r\n"
 //                          "Accept: text/event-stream\r\n"
 //                          "\r\n";



 const char *request = "GET / HTTP/1.1\r\nHost: www.example.com\r\n\r\n";

    // Send the request
    int bytesSent = send(sockfd, request, strlen(request), 0);
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


    if (write(sockfd, auth_header, strlen(auth_header)) == -1) {
        fprintf(stderr, "SSE request failed: %d\n", WSAGetLastError());
        // return 1;
    }
     
     const char* sseRequest = "GET /api/metadata/now/chillout HTTP/1.1\r\n"
                             "Host: antenne.de\r\n"
                             "Accept: text/event-stream\r\n"
                             "Cache-Control: no-cache\r\n"
                             "Connection: keep-alive\r\n"
                             "Content-Type: text/event-stream\r\n\r\n";
    
if (write(sockfd, sseRequest, strlen(sseRequest)) == -1) {
        perror("SSE request failed");
        // return 1;
    }
    // Perform other operations with the socket

    // Close the socket and cleanup when done
    closesocket(sockfd);
    WSACleanup();

    return 0;
}
