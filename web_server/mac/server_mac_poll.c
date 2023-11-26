/**
 * @author      : alex (alexbora@gmail.com)
 * @file        : server_mac_poll
 * @created     : Duminică Noi 26, 2023 10:15:24 EET
 */

#include <arpa/inet.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "../log.h"

#define PORT 8080
#define HTML_FILE "../index.html"
#define MAX_CLIENTS 10
#define INITIAL_POLL_INTERVAL 1000  // Initial poll timeout in milliseconds
#define SLIDING_WINDOW 60           // Time window in seconds
#define FILENAME "polling_interval.txt"

// Global variable to store HTML content
char *htmlContent = NULL;

struct sigaction act;

void read_html_file() {
  FILE *htmlFile = fopen(HTML_FILE, "r++");

  if (htmlFile == NULL) {
    log_message(WARN, "HTML file not found");
    return;
  }

  // Read the HTML content from the file
  fseek(htmlFile, 0, SEEK_END);
  long fileSize = ftell(htmlFile);
  fseek(htmlFile, 0, SEEK_SET);

  htmlContent = (char *)malloc(fileSize + 1);
  if (htmlContent == NULL) {
    log_message(WARN, "Memory allocation failed");
    return;
  }

  fread(htmlContent, 1, fileSize, htmlFile);
  htmlContent[fileSize] = '\0';

  fclose(htmlFile);
}

void free_html_content() {
  // Free the allocated memory for HTML content
  if (htmlContent != NULL) {
    free(htmlContent);
    htmlContent = NULL;
  }
}

typedef struct {
  int clientSocket;
  int serverSocket;
  char *htmlContent;
} Runtime_data;

void __attribute__((destructor)) clean_exit(Runtime_data *data) {
  // Close the server socket and free HTML content
  int serverSocket = data->serverSocket;
  int clientSocket = data->clientSocket;
  close(serverSocket);
  close(data->clientSocket);
  shutdown(clientSocket, SHUT_RDWR);
  shutdown(serverSocket, SHUT_RDWR);
  free_html_content();
}

void handle_sigint(int sig) {
  log_message(WARN, "Caught signal");
  printf("Caught signal %d\n", sig);
  free_html_content();

  exit(0);
}
void handle_client(int clientSocket) {
  const char *ok_response =
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: text/html\r\n"
      "Connection: close\r\n"
      "\r\n";
  const char *error_response =
      "HTTP/1.1 500 Internal Server Error\r\n"
      "Content-Type: text/plain\r\n"
      "Connection: close\r\n"
      "\r\nError serving HTML content";

  if (htmlContent != NULL) {
    // Respond with the stored HTML content
    write(clientSocket,
          "HTTP/1.1 200 OK\r\n"
          "Content-Type: text/html\r\n"
          "Connection: close\r\n"
          "\r\n",
          strlen("HTTP/1.1 200 OK\r\n"
                 "Content-Type: text/html\r\n"
                 "Connection: close\r\n"
                 "\r\n"));

    write(clientSocket, htmlContent, strlen(htmlContent));
  } else {
    // Respond with an error message
    write(clientSocket,
          "HTTP/1.1 500 Internal Server Error\r\n"
          "Content-Type: text/plain\r\n"
          "Connection: close\r\n"
          "\r\nError serving HTML content",
          strlen("HTTP/1.1 500 Internal Server Error\r\n"
                 "Content-Type: text/plain\r\n"
                 "Connection: close\r\n"
                 "\r\nError serving HTML content"));
  }

  // Close the client socket
  close(clientSocket);
}

void accept_new_connection(int serverSocket, struct pollfd *pollfds,
                           int *numClients) {
  struct sockaddr_in clientAddr;
  socklen_t addrSize = sizeof(struct sockaddr_in);

  int newClient =
      accept(serverSocket, (struct sockaddr *)&clientAddr, &addrSize);
  if (newClient == -1) {
    log_message(ERROR, "Accept failed");
    return;
  }

  log_message(INFO, "Connection accepted");
  printf("Connection accepted from %s:%d\n", inet_ntoa(clientAddr.sin_addr),
         ntohs(clientAddr.sin_port));

  // Add the new client socket to the array
  if (*numClients < MAX_CLIENTS) {
    pollfds[*numClients + 1].fd = newClient;
    pollfds[*numClients + 1].events = POLLIN;
    pollfds[*numClients + 1].revents = 0;
    (*numClients)++;
  } else {
    log_message(WARN, "Max clients reached. Connection rejected.");
    close(newClient);
  }
}

void handle_client_data(int clientSocket, struct pollfd *pollfds,
                        int *numClients) {
  handle_client(clientSocket);

  // Remove the client socket from the array
  for (int i = 1; i <= *numClients; i++) {
    if (pollfds[i].fd == clientSocket) {
      pollfds[i] = pollfds[*numClients];
      (*numClients)--;
      break;
    }
  }
}

void update_sliding_window(int *eventsCount, time_t *windowStart,
                           int *pollTimeout) {
  (*eventsCount)++;
  time_t currentTime = time(NULL);
  double elapsedSeconds = difftime(currentTime, *windowStart);

  if (elapsedSeconds >= SLIDING_WINDOW) {
    // Adjust poll timeout based on events per second
    double eventsPerSecond = *eventsCount / elapsedSeconds;
    *pollTimeout = eventsPerSecond > 0 ? (int)(1000 / eventsPerSecond)
                                       : INITIAL_POLL_INTERVAL;

    // Reset counters
    *windowStart = currentTime;
    *eventsCount = 0;
  }
}

int main() {
  int serverSocket, clientSockets[MAX_CLIENTS];
  struct sockaddr_in serverAddr;
  socklen_t addrSize = sizeof(struct sockaddr_in);
  struct pollfd pollfds[MAX_CLIENTS + 1];  // +1 for server socket
  int numClients = 0;
  int pollTimeout = INITIAL_POLL_INTERVAL;
  time_t windowStart = time(NULL);
  int eventsCount = 0;

  signal(SIGINT, handle_sigint);

  // Read HTML content into the global pointer
  read_html_file();

  // Create socket
  if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    log_message(ERROR, "Socket creation failed");
    free_html_content();
    exit(EXIT_FAILURE);
  }

  // Prepare server address structure
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  serverAddr.sin_port = htons(PORT);

  // Bind the socket
  if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) ==
      -1) {
    free_html_content();
    log_message(ERROR, "Bind failed");
  }

  // Listen for incoming connections
  if (listen(serverSocket, SOMAXCONN) == -1) {
    free_html_content();
    log_message(ERROR, "Listen failed");
  }

  log_message(INFO, "Server listening ...");

  // Initialize client sockets array
  for (int i = 0; i < MAX_CLIENTS; i++) clientSockets[i] = -1;

  // Initialize poll structure for the server socket
  pollfds[0].fd = serverSocket;
  pollfds[0].events = POLLIN;
  pollfds[0].revents = 0;

  while (1) {
    // Poll for events
    int numReady = poll(pollfds, numClients + 1, pollTimeout);
    if (numReady == -1) log_message(ERROR, "Poll error");
    if (numReady == 0) log_message(INFO, "No events within the interval");

    // Handle different types of events
    for (int i = 0; i <= numClients; i++) {
      if (pollfds[i].revents) {
        switch (i) {
          case 0:
            // Server socket event (new connection)
            if (pollfds[i].revents & POLLIN) {
              accept_new_connection(serverSocket, pollfds, &numClients);
            }
            break;
          default:
            // Client socket event (data available)
            if (pollfds[i].revents & POLLIN) {
              handle_client_data(pollfds[i].fd, pollfds, &numClients);
            }
            break;
        }
      }
    }

    // Update sliding window counters
    update_sliding_window(&eventsCount, &windowStart, &pollTimeout);
  }

  // Close the server socket and free HTML content (this part is unreachable in
  // this example)
  close(serverSocket);
  free_html_content();

  return 0;
}

