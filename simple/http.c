#include <arpa/inet.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define REQUEST_TEMPLATE "GET / HTTP/1.1\r\nHost: %s\r\n\r\n"

#define SERVER_PORT 3000

int main() {
  // Create a socket
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  // Set up the server address structure
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(SERVER_PORT);
  inet_pton(AF_INET, "127.0.0.1", &(server_addr.sin_addr));

  // Connect to the server
  if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) ==
      -1) {
    perror("Connection failed");
    close(sockfd);
    exit(EXIT_FAILURE);
  }

  printf("Connected to localhost:%d\n", SERVER_PORT);

  char request[1024];
  snprintf(request, sizeof(request), REQUEST_TEMPLATE, "3000");

  // Send the HTTP GET request
  if (send(sockfd, request, strlen(request), 0) == -1) {
    perror("Send failed");
    close(sockfd);
    exit(EXIT_FAILURE);
  }

  printf("GET request sent:\n%s\n", request);

  // Prepare the poll structure
  struct pollfd poll_fd;
  poll_fd.fd = sockfd;
  poll_fd.events = POLLIN; // Wait for data to read

  while (1) {
    // Use poll to wait for events
    int result = poll(&poll_fd, 1, -1); // Timeout of -1 means indefinite wait

    if (result == -1) {
      perror("Poll failed");
      break;
    }

    if (result > 0) {
      // Check if the socket has data to read
      if (poll_fd.revents & POLLIN) {
        puts("Polling 1");
        char buffer[1024];
        ssize_t bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);

        if (bytes_received > 0) {
          buffer[bytes_received] = '\0';
          printf("Received data: %s", buffer);
        } else if (bytes_received == 0) {
          // Connection closed by the server
          printf("Server closed the connection.\n");
          break;
        } else {
          perror("Receive error");
          break;
        }
      }
    }
  }

  // Close the socket
  close(sockfd);

  return 0;
}
