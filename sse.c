#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <winsock2.h>
#include <ws2tcpip.h> 

#pragma comment(lib,"ws2_32.lib") //Winsock Library

#define SERVER_HOSTNAME "webradio.antenne.de"
#define SERVER_PORT "80"



int main() {
   struct addrinfo hints, *serverInfo, *p;
    int clientSocket;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // Resolve the server's hostname to an IP address using getaddrinfo
    int status = getaddrinfo(SERVER_HOSTNAME, SERVER_PORT, &hints, &serverInfo);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        return 1;
    }

    // Iterate through the list of addresses and attempt to create a socket and connect
    for (p = serverInfo; p != NULL; p = p->ai_next) {
        clientSocket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (clientSocket == -1) {
            perror("Socket creation failed");
            continue;
        }

        if (connect(clientSocket, p->ai_addr, p->ai_addrlen) == -1) {
            close(clientSocket);
            perror("Connection failed");
            continue;
        }

        break; // Successfully created a socket and connected
    }

    if (p == NULL) {
        fprintf(stderr, "Failed to connect to the server\n");
        return 2;
    }

    // Send the SSE request
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
 freeaddrinfo(serverInfo); // Clean up the linked list
    return 0;
}
