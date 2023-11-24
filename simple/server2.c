/**
 * @author      : alex (alexbora@gmail.com)
 * @file        : server2
 * @created     : Vineri Noi 24, 2023 18:05:54 EET
 */

#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 8080
#define MAX_CONNECTIONS 5
#define THREAD_POOL_SIZE 3

pthread_mutex_t mutex;
pthread_t threadPool[THREAD_POOL_SIZE];
int clientSockets[MAX_CONNECTIONS];

void *handleClient(void *socketDesc) {
  int clientSocket = *(int *)socketDesc;
  char buffer[1024] = {0};

  // Read client request
  read(clientSocket, buffer, sizeof(buffer));
  printf("Received from client: %s\n", buffer);

  // Read the content of index.html file
  FILE *htmlFile = fopen("index.html", "r");
  if (htmlFile == NULL) {
    perror("Error opening index.html");
    exit(EXIT_FAILURE);
  }

  char htmlBuffer[4096];
  size_t bytesRead = fread(htmlBuffer, 1, sizeof(htmlBuffer), htmlFile);

  // Send HTML response
  if (bytesRead > 0) {
    write(clientSocket, htmlBuffer, bytesRead);
  } else {
    const char *response =
        "HTTP/1.1 500 Internal Server Error\nContent-Type: "
        "text/plain\nContent-Length: 29\n\nError reading index.html file";
    write(clientSocket, response, strlen(response));
  }

  // Close the client socket and file
  close(clientSocket);
  fclose(htmlFile);

  free(socketDesc);  // Free memory allocated for socket descriptor
  pthread_exit(NULL);
}

void *workerThread(void *arg) {
  while (1) {
    pthread_mutex_lock(&mutex);

    // Get the next available client socket
    int clientSocket = -1;
    for (int i = 0; i < MAX_CONNECTIONS; ++i) {
      if (clientSockets[i] != -1) {
        clientSocket = clientSockets[i];
        clientSockets[i] = -1;
        break;
      }
    }

    pthread_mutex_unlock(&mutex);

    if (clientSocket != -1) {
      // Handle the client in a separate thread
      handleClient(&clientSocket);
    } else {
      // Sleep if no task is available
      usleep(1000);
    }
  }
}

int main() {
  // ... (unchanged part of the code)

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
    int clientSocket =
        accept(serverSocket, (struct sockaddr *)&clientAddr, &addrSize);
    if (clientSocket == -1) {
      perror("Accept failed");
      exit(EXIT_FAILURE);
    }

    printf("Connection accepted from %s:%d\n", inet_ntoa(clientAddr.sin_addr),
           ntohs(clientAddr.sin_port));

    pthread_mutex_lock(&mutex);

    // Find an available slot in the clientSockets array
    int index = -1;
    for (int i = 0; i < MAX_CONNECTIONS; ++i) {
      if (clientSockets[i] == -1) {
        index = i;
        break;
      }
    }

    if (index != -1) {
      // Assign the client socket to a worker thread
      clientSockets[index] = clientSocket;
      int *newSocket = malloc(sizeof(int));
      *newSocket = clientSocket;
      if (pthread_create(&thread, NULL, handleClient, (void *)newSocket) != 0) {
        perror("Thread creation failed");
        exit(EXIT_FAILURE);
      }

      // Detach the thread to allow it to run independently
      pthread_detach(thread);
    } else {
      // No available slot, reject the connection
      close(clientSocket);
    }

    pthread_mutex_unlock(&mutex);
  }
}

// ... (unchanged part of the code)

