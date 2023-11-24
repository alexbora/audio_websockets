/**
 * @author      : alex (alexbora@gmail.com)
 * @file        : server
 * @created     : Vineri Noi 24, 2023 18:00:58 EET
 */

#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 8080
#define MAX_CONNECTIONS 5

void *handleClient(void *socketDesc) {
  int clientSocket = *(int *)socketDesc;
  char buffer[1024] = {0};

  // Read client request
  read(clientSocket, buffer, sizeof(buffer));
  printf("Received from client: %s\n", buffer);

  // Send a simple response
  const char *response =
      "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello, "
      "World!";
  send(clientSocket, response, strlen(response), 0);

  // Close the client socket
  close(clientSocket);

  /* free(socketDesc);  // Free memory allocated for socket descriptor */
  pthread_exit(NULL);
}

int main() {
  int serverSocket, clientSocket;
  struct sockaddr_in serverAddr, clientAddr;
  socklen_t addrSize = sizeof(struct sockaddr_in);
  pthread_t thread;

  // Create socket
  if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  // Prepare server address structure
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  serverAddr.sin_port = htons(PORT);

  // Bind the socket
  if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) ==
      -1) {
    perror("Bind failed");
    exit(EXIT_FAILURE);
  }

  // Listen for incoming connections
  if (listen(serverSocket, MAX_CONNECTIONS) == -1) {
    perror("Listen failed");
    exit(EXIT_FAILURE);
  }

  printf("Server listening on port %d...\n", PORT);

  while (1) {
    // Accept a connection
    if ((clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr,
                               &addrSize)) == -1) {
      perror("Accept failed");
      exit(EXIT_FAILURE);
    }

    printf("Connection accepted from %s:%d\n", inet_ntoa(clientAddr.sin_addr),
           ntohs(clientAddr.sin_port));

    // Create a thread to handle the client
    /* int *newSocket = malloc(sizeof(int)); */
    /* *newSocket = clientSocket; */
    if (pthread_create(&thread, NULL, handleClient, &clientSocket) != 0) {
      perror("Thread creation failed");
      exit(EXIT_FAILURE);
    }

    // Detach the thread to allow it to run independently
    pthread_detach(thread);
  }

  // Close the server socket (this part is unreachable in this example)
  close(serverSocket);

  return 0;
}

