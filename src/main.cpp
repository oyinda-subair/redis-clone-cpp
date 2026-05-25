#include <cstring>
#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <vector>

#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "string_helper.h"

#define PORT 6379
#define BUFFER_SIZE 4096

void handle_client(int client_socket)
{
  char buffer[BUFFER_SIZE];

  // Send welcome message (optional)
  const char *welcome = "+OK\r\n";
  send(client_socket, welcome, strlen(welcome), 0);

  while (true)
  {
    memset(buffer, 0, BUFFER_SIZE);

    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);

    if (bytes_received <= 0)
    {
      std::cout << "Client disconnected\n";
      break;
    }

    std::string input(buffer);
    std::cout << "Received: " << input;

    // Remove trailing newline/carriage return
    input.erase(input.find_last_not_of("\r\n") + 1);

    std::vector<std::string> tokens = token(input, ' ');
    // if (tokens.size() >= 1)
    //{
    //  std::string response = "-ERR unknown command\r\n";
    //  send(client_socket, tokens[0].c_str(), tokens[0].size(), 0);

    // continue;
    //}

    std::cout << "Command 0 : " << tokens[0] << "\n";
    std::cout << "Command 1 : " << tokens[1] << "\n";

    if (tokens[0] == "PING")
    {
      if (tokens[1] == "hello")
      {
        /* code */
        std::string response = "$5\r\nhello\r\n";
        send(client_socket, response.c_str(), response.size(), 0);
      }
      else
      {
        std::string response = "+PONG\r\n";
        send(client_socket, response.c_str(), response.size(), 0);
      }
    }
    else
    {
      std::string response = "-ERR unknown command\r\n";
      send(client_socket, response.c_str(), response.size(), 0);
    }
  }

  close(client_socket);
}

int main()
{
  int server_fd;
  struct sockaddr_in address;
  int opt = 1;
  int addrlen = sizeof(address);

  // Create socket
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == 0)
  {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  // Allow reuse of address
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  address.sin_family = AF_INET;
  // Listen on all interfaces
  // Accept connections from any interface (like 0.0.0.0)
  address.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0
  address.sin_port = htons(PORT);

  // Bind
  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
  {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  // Listen
  if (listen(server_fd, 5) < 0)
  {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  std::cout << "Server listening on port " << PORT << std::endl;

  while (true)
  {
    int client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);

    if (client_socket < 0)
    {
      perror("accept");
      continue;
    }

    std::cout << "New client connected\n";

    std::thread t(handle_client, client_socket);
    t.detach(); // allow multiple clients
  }

  close(server_fd);
  return 0;
}
