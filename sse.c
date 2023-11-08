#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <winsock2.h>
#include <ws2tcpip.h> 

#pragma comment(lib,"ws2_32.lib") //Winsock Library



int main() {
    struct sockaddr_in serverAddress;
    struct hostent *serverInfo;
    const char *SERVER_HOSTNAME = "pxgot1-onprem.srv.volvo.com";
    int SERVER_PORT = 8080;  // Replace with your destination port

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
