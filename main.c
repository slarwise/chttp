#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

void fatal(char *msg) {
  perror(msg);
  exit(1);
}

int main(int argc, char **argv) {
  int body_only = 0;
  for (int i = 1; i < argc; i++) {
    char *arg = argv[i];
    if (strcmp(arg, "-b") == 0 || strcmp(arg, "--body-only") == 0) {
      body_only = 1;
    }
    if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
      printf("Usage: %s [-b | --body-only]\n", argv[0]);
      printf("    Make a get request to jsonplaceholder.typicode.com/todos/1\n");
      printf("\n");
      printf("    -b, --body-only\n");
      printf("        Do not print the status and headers, only print the response body\n");
      return 0;
    }
  }
  int client = socket(PF_INET, SOCK_STREAM, 0);
  if (client == -1) {
    fatal("Could not create socket");
  }
  struct hostent *server;
  server = gethostbyname("jsonplaceholder.typicode.com");
  if (server == NULL) fatal("Could not get host");
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(80);
  memcpy(&server_addr.sin_addr, server->h_addr, server->h_length);
  if (connect(client, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) fatal("Could not connect to server");
  char message[] = "GET /todos/1 HTTP/1.1\r\nHost: jsonplaceholder.typicode.com\r\nConnection: close\r\n\r\n";
  int total = strlen(message);
  int sent = 0;
  int bytes;
  while (sent < total) {
    bytes = send(client, message+sent, total-sent, 0);
    if (bytes < 0) fatal("Could not send message");
    sent += bytes;
  }
  char response[4096];
  memset(response, 0, sizeof(response));
  int received = 0;
  total = sizeof(response) - 1; // Gotta keep the last null
  while (received < total) {
    bytes = recv(client, response+received, total-received, 0);
    if (bytes < 0) fatal("Could not read from socket");
    if (bytes == 0) break;
    received += bytes;
  }
  char *line = strtok(response, "\n");
  // Status
  if (!body_only) fprintf(stderr, "%s\n", line);
  line = strtok(NULL, "\n");
  // Headers
  while (strcmp(line, "\r") != 0) {
    if (!body_only) fprintf(stderr, "%s\n", line);
    line = strtok(NULL, "\n");
  }
  line = strtok(NULL, "\n");
  // Body
  while (line != NULL) {
    fprintf(stdout, "%s\n", line);
    line = strtok(NULL, "\n");
  }
  close(client);
}
