
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _linux

void setnonblocking_nix(int sockfd) {
  int flags = fcntl(sockfd, F_GETFL, 0);
  fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}
#else

#include <winsock2.h>

void setnonblocking_win(SOCKET sockfd) {
  u_long mode = 1; // 1 for non-blocking, 0 for blocking

  if (ioctlsocket(sockfd, FIONBIO, &mode) == SOCKET_ERROR) {
    // Handle error
    perror("ioctlsocket");
    // You might want to add additional error handling code here
  }
};

#endif

#define PORT 8080
#define BUFFER_SIZE 1024

void setnonblocking(int sockfd) {
#ifdef _linux
  return setnonblocking_nix(sockfd);
#else
  return setnonblocking_win(sockfd);
#endif
}

void handle_client(SOCKET client_socket) {
  FILE *html_file = fopen("index.html", "r");
  if (html_file == NULL) {
    fprintf(stderr, "Error opening HTML file.\n");
    exit(EXIT_FAILURE);
  }

  char response_buffer[BUFFER_SIZE];
  char line_buffer[BUFFER_SIZE];

  // Send HTTP header
  sprintf(response_buffer,
          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
  send(client_socket, response_buffer, strlen(response_buffer), 0);

  // Send the content of index.html
  while (fgets(line_buffer, BUFFER_SIZE, html_file) != NULL) {
    send(client_socket, line_buffer, strlen(line_buffer), 0);
  }

  fclose(html_file);
  closesocket(client_socket);
}

void handle_form_submission(char *query_string) {
  // Find the start of the input data in the query string
  char *input_start = strstr(query_string, "userInput=");
  if (input_start != NULL) {
    // Move the pointer to the actual input value
    input_start += strlen("userInput=");

    // Find the end of the input value
    char *input_end = strstr(input_start, "HTTP");
    if (input_end == NULL) {
      // If there's no "&" character, consider the input value until the end of
      // the string
      input_end = strchr(input_start, '\0');
    }

    // Allocate memory to store the input value
    char *user_input = malloc(input_end - input_start + 1);
    if (user_input == NULL) {
      fprintf(stderr, "Error allocating memory for user input.\n");
      exit(EXIT_FAILURE);
    }

    // Copy the input value to the allocated memory
    strncpy(user_input, input_start, input_end - input_start);
    user_input[input_end - input_start] = '\0'; // Null-terminate the string

    // Print the user input
    printf("Received input from the form: %s\n", user_input);

    // Free the allocated memory
    free(user_input);
  }
}

int main() {
  WSADATA wsaData;
  SOCKET server_socket, client_socket;
  struct sockaddr_in server_addr, client_addr;
  int addr_len = sizeof(client_addr);
  char buffer[BUFFER_SIZE];

  // Initialize Winsock
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    fprintf(stderr, "WSAStartup failed.\n");
    return 1;
  }

  // Create socket
  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket == INVALID_SOCKET) {
    fprintf(stderr, "Error creating socket.\n");
    WSACleanup();
    return 1;
  }

  // Set up server address struct
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  // Bind socket
  if (bind(server_socket, (struct sockaddr *)&server_addr,
           sizeof(server_addr)) == SOCKET_ERROR) {
    fprintf(stderr, "Error binding socket.\n");
    closesocket(server_socket);
    WSACleanup();
    return 1;
  }

  // Listen for incoming connections
  if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
    fprintf(stderr, "Error listening on socket.\n");
    closesocket(server_socket);
    WSACleanup();
    return 1;
  }

  printf("Server listening on port %d...\n", PORT);

  while (1) {
    // Accept a connection
    client_socket =
        accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
    if (client_socket == INVALID_SOCKET) {
      fprintf(stderr, "Error accepting connection.\n");
      closesocket(server_socket);
      WSACleanup();
      return 1;
    }

    time_t current_time = 0;
    time(&current_time);
    printf("%s Accepted connection from %s:%d\n", ctime(&current_time),
           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    // printf("Accepted connection from %s:%d\n",
    // inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    // Receive client request
    memset(buffer, 0, BUFFER_SIZE);
    recv(client_socket, buffer, BUFFER_SIZE, 0);

    // Handle the client request
    handle_client(client_socket);

    // Check if the request contains form data
    char *query_string = strstr(buffer, "GET /submit");
    if (query_string != NULL) {
      handle_form_submission(query_string);
    }
  }

  // Close the server socket
  closesocket(server_socket);
  WSACleanup();

  return 0;
}
