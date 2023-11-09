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


int main() {
    struct sockaddr_in serverAddress;
    struct hostent *serverInfo;
    const char *SERVER_HOSTNAME = "pxgot1-onprem.srv.volvo.com";
    int SERVER_PORT = 8080;  // Replace with your destination port

const char* username = "a049689";
const char* password = "SummicronSummilux-50";
char auth_string[256];
    snprintf(auth_string, sizeof(auth_string), "%s:%s", username, password);
    const char *base64_auth = base64_encode(auth_string, strlen(auth_string));
// const char* base64Credentials = base64Encode(auth_string); // Base64 encoding function is not included in this code
char auth_header[256];
    snprintf(auth_header, sizeof(auth_header), "Proxy-Authorization: Basic %s\r\n", base64_auth);

printf("%s\n", auth_header);

const char* request_proxy = "GET http://example.com/ HTTP/1.1\r\n"
                     "Host: example.com\r\n"
                     "Proxy-Authorization: Basic base64Credentials\r\n\r\n";


#ifdef _WIN32
WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        perror("WSAStartup failed");
        return 1;
    }

    // Create a socket
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        perror("Socket creation failed");
        WSACleanup(); // Clean up Winsock
        return 1;
    }
#elif

     int clientSocket;
    // Create a socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        perror("Socket creation failed");
        return 1;
    }
#endif
    // Resolve the server's IP address using gethostbyname
    serverInfo = gethostbyname(SERVER_HOSTNAME);
    if (serverInfo == NULL) {
        perror("Failed to resolve server host");
        close(clientSocket);
        return 1;
    } 


      int i = 0;
    while (serverInfo->h_addr_list[i] != NULL) {
        struct in_addr addr;
        memcpy(&addr, serverInfo->h_addr_list[i], sizeof(struct in_addr));

        printf("IP Address %d: %s\n", i + 1, inet_ntoa(addr));
        i++;
    }


    // Configure the server address
    memset((char *)&serverAddress,'\0', sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    memcpy((char *)serverInfo->h_addr, (char *)&serverAddress.sin_addr.s_addr, serverInfo->h_length);
    serverAddress.sin_port = htons(SERVER_PORT);

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("Server connection failed");
        close(clientSocket);
        return 1;
    }

    // Your code to send and receive data here

    const char* sseRequest = "GET /api/metadata/now/chillout HTTP/1.1\r\n"
                             "Host: antenne.de\r\n"
                             "Accept: text/event-stream\r\n"
                             "Cache-Control: no-cache\r\n"
                             "Connection: keep-alive\r\n"
                             "Content-Type: text/event-stream\r\n\r\n";
    
    if (write(clientSocket, sseRequest, strlen(sseRequest)) == -1) {
        perror("SSE request failed");
        return 1;
    }

    // Read and process SSE responses
    char buffer[1024];
    while (1) {
        int bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
        if (bytesRead <= 0) {
            perror("Connection closed or read error");
            break;
        }
        buffer[bytesRead] = '\0';

        // Process the SSE data (parse and handle events)
        printf("Received SSE data: %s\n", buffer);
    }

    // Close the connection
    close(clientSocket);
    return 0;
}
