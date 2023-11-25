

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/event.h>
#include <time.h>
#include <unistd.h>

#define PORT 8080
#define MAX_EVENTS 10
#define INITIAL_TIMEOUT 1000  // Initial poll timeout in milliseconds
#define WINDOW_SIZE 60        // Time window in seconds

#ifdef DEBUG
#define BOLD_RED "\033[1;31m"
#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define RESET "\033[0m"

enum {
  INFO,
  WARN,
  ERROR,
} level = INFO;

#define debug(level, fmt, ...)                                              \
  do {                                                                      \
    switch (level) {                                                        \
      case INFO:                                                            \
        fprintf(stderr, "%s[INFO] %s:%d:%s(): " fmt, GREEN, __FILE__,       \
                __LINE__, __func__, ##__VA_ARGS__);                         \
        fprintf(stderr, "%s\n", RESET);                                     \
        break;                                                              \
      case ERROR:                                                           \
        fprintf(stderr, "%s[ERROR] %s:%d:%s(): \n" fmt, BOLD_RED, __FILE__, \
                __LINE__, __func__, ##__VA_ARGS__);                         \
        fprintf(stderr, "%s\n", RESET);                                     \
        exit(EXIT_FAILURE);                                                 \
        break;                                                              \
      case WARN:                                                            \
        fprintf(stderr, "%s[WARN] %s:%d:%s(): \n" fmt, YELLOW, __FILE__,    \
                __LINE__, __func__, ##__VA_ARGS__);                         \
        fprintf(stderr, "%s\n", RESET);                                     \
        break;                                                              \
    }                                                                       \
  } while (0)
#else
#define debug(...) (void)0
#define INFO(void) 0
#define WARN (void)0
#define ERROR (void)0
#endif

static inline void setnonblocking(int sockfd) {
  int flags = fcntl(sockfd, F_GETFL, 0);
  if (flags == -1) debug(WARN, "Error getting socket flags");
  if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1)
    debug(WARN, "Error setting socket to non-blocking");
}

int main(int argc, char **argv) {
  int serverSocket, clientSocket, kqueue_fd;
  struct sockaddr_in serverAddr, clientAddr;
  socklen_t addrSize = sizeof(struct sockaddr_in);
  struct kevent change, event;

  // Create socket
  if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    debug(ERROR, "Socket creation failed");
  debug(INFO, "Server socket created");
  // Prepare server address structure
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  serverAddr.sin_port = htons(PORT);
  if (serverAddr.sin_port == 0) debug(ERROR, "Port htons failed");
  //
  // Bind the socket
  if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) ==
      -1)
    debug(ERROR, "Socket binding failed");
  debug(INFO, "Server socket binding successful");

  // Set socket to non-blocking
  setnonblocking(serverSocket);

  // Listen for incoming connections
  if (listen(serverSocket, SOMAXCONN) == -1) debug(ERROR, "Listen failed");

  // Create kqueue instance
  kqueue_fd = kqueue();
  if (kqueue_fd == -1) debug(ERROR, "Kqueue creation failed");

  debug(INFO, "Kqueue created");
  // Register server socket for read events
  EV_SET(&change, serverSocket, EVFILT_READ, EV_ADD, 0, 0, NULL);

  if (kevent(kqueue_fd, &change, 1, NULL, 0, NULL) == -1)
    debug(ERROR, "Kevent registration failed for server socket");
  debug(INFO, "Kevent registered");

  /* printf("Server listening on port %d...\n", PORT); */
  debug(INFO, "Server listening on port %d...", PORT);

  int timeout = INITIAL_TIMEOUT;
  time_t start_time = time(NULL);
  int events_count = 0;

  while (1) {
    int num_events = kevent(kqueue_fd, NULL, 0, &event, 1, NULL);

    switch (num_events) {
      case 0: {
        time_t current_time = time(NULL);
        double elapsed_time = difftime(current_time, start_time);

        if (elapsed_time >= WINDOW_SIZE) {
          // Calculate events per second
          double events_per_second = events_count / elapsed_time;

          // Adjust timeout based on events per second
          timeout = events_per_second > 0 ? (int)(1000 / events_per_second)
                                          : INITIAL_TIMEOUT;
          // Reset counters
          events_count = 0;
          start_time = current_time;
          debug(INFO, "Timeout adjusted to %d", timeout);
        }
      } break;

      case -1:
        debug(ERROR, "Kqueue wait error");
        break;
      default: {
        if (event.ident == serverSocket) {
          while ((clientSocket =
                      accept(serverSocket, (struct sockaddr *)&clientAddr,
                             &addrSize)) != -1) {
            debug(INFO, "Connection accepted from %s:%d",
                  inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

            setnonblocking(clientSocket);

            // Register client socket for read events
            EV_SET(&change, clientSocket, EVFILT_READ, EV_ADD, 0, 0, NULL);
            if (kevent(kqueue_fd, &change, 1, NULL, 0, NULL) == -1) {
              debug(WARN, "Kevent registration failed for client socket");
              close(clientSocket);
              shutdown(clientSocket, SHUT_RDWR);
            }
          }

          if (clientSocket == -1 && errno != EAGAIN && errno != EWOULDBLOCK)
            debug(WARN, "Accept failed");

        } else {
          // Handle data from client
          char buffer[1024];
          ssize_t bytesRead;
          while ((bytesRead = read(event.ident, buffer, sizeof(buffer))) > 0) {
            debug(INFO, "Received data from client %zu: %.*s\n", event.ident,
                  (int)bytesRead, buffer);
            events_count++;
          }

          if (bytesRead == 0) {
            // Connection closed by the client
            debug(INFO, "Client %zu disconnected.\n", event.ident);

            // Unregister client socket from kqueue
            EV_SET(&change, event.ident, EVFILT_READ, EV_DELETE, 0, 0, NULL);

            if (kevent(kqueue_fd, &change, 1, NULL, 0, NULL) == -1) {
              debug(ERROR, "Kevent registration failed for server socket");
              close(event.ident);
            }
          } else if (bytesRead == -1 && errno != (EAGAIN | EWOULDBLOCK)) {
            debug(INFO, "Error reading data from client");

            // Unregister client socket from kqueue
            EV_SET(&change, event.ident, EVFILT_READ, EV_DELETE, 0, 0, NULL);
            kevent(kqueue_fd, &change, 1, NULL, 0, NULL);

            close(event.ident);
          }
        }

        // Reset timeout to initial value
        timeout = INITIAL_TIMEOUT;
        debug(INFO, "Timeout reset to %d", timeout);
      } break;
    }

#if 0
    if (num_events > 0) {
      if (event.ident == serverSocket) {
        while (
            (clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr,
                                   &addrSize)) != -1) {
          debug(INFO, "Connection accepted from %s:%d",
                inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

          setnonblocking(clientSocket);

          // Register client socket for read events
          EV_SET(&change, clientSocket, EVFILT_READ, EV_ADD, 0, 0, NULL);
          if (kevent(kqueue_fd, &change, 1, NULL, 0, NULL) == -1) {
            debug(WARN, "Kevent registration failed for client socket");
            close(clientSocket);
            shutdown(clientSocket, SHUT_RDWR);
          }
        }

        if (clientSocket == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
          debug(WARN, "Accept failed");
        }
      } else {
        // Handle data from client
        char buffer[1024];
        ssize_t bytesRead;
        while ((bytesRead = read(event.ident, buffer, sizeof(buffer))) > 0) {
          // Process the data received from the client
          printf("Received data from client %zu: %.*s\n", event.ident,
                 (int)bytesRead, buffer);
          events_count++;
        }

        if (bytesRead == 0) {
          // Connection closed by the client
          printf("Client %zu disconnected.\n", event.ident);

          // Unregister client socket from kqueue
          EV_SET(&change, event.ident, EVFILT_READ, EV_DELETE, 0, 0, NULL);
          kevent(kqueue_fd, &change, 1, NULL, 0, NULL);

          close(event.ident);
        } else if (bytesRead == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
          perror("Error reading data from client");

          // Unregister client socket from kqueue
          EV_SET(&change, event.ident, EVFILT_READ, EV_DELETE, 0, 0, NULL);
          kevent(kqueue_fd, &change, 1, NULL, 0, NULL);

          close(event.ident);
        }
      }

      // Reset timeout to initial value
      timeout = INITIAL_TIMEOUT;
      debug(INFO, "Timeout reset to %d", timeout);
    }

    else if (num_events == 0) {
      // No events, adjust the timeout dynamically based on the event
      // frequency
      time_t current_time = time(NULL);
      double elapsed_time = difftime(current_time, start_time);

      if (elapsed_time >= WINDOW_SIZE) {
        // Calculate events per second
        double events_per_second = events_count / elapsed_time;

        // Adjust timeout based on events per second
        timeout = events_per_second > 0 ? (int)(1000 / events_per_second)
                                        : INITIAL_TIMEOUT;
        debug(INFO, "Timeout adjusted to %d", timeout);
        // Reset counters
        events_count = 0;
        start_time = current_time;
      }
    } else {
      // Error handling
      debug(ERROR, "Kqueue wait error");
      break;
    }
#endif
  }
  // Close the server socket and kqueue instance (this part is unreachable
  // in this example)
  close(serverSocket);
  shutdown(serverSocket, SHUT_RDWR);
  close(kqueue_fd);
  shutdown(kqueue_fd, SHUT_RDWR);
  return 0;
}
