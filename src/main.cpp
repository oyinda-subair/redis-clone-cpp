#include <cstring>
#include <iostream>
#include <mutex>
#include <string>
#include <sstream>
#include <thread>
#include <vector>
#include <unordered_map>

#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "string_helper.h"

std::unordered_map<std::string, std::string> store;
std::mutex store_mutex;

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

    // std::vector<std::string> tokens = token(input, ' ');
    auto tokens = split(input);

    if (tokens.empty())
    {
      std::string response = "-ERR empty command\r\n";
      send(client_socket, response.c_str(), response.size(), 0);
      continue;
    }

    std::string command = tokens[0];

    if (command == "PING")
    {
      if (tokens.size() > 1 && tokens[1] == "hello")
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
    else if (command == "SET")
    {
      if (tokens.size() != 3)
      {
        std::string response = error_response("SET");
        send(client_socket, response.c_str(), response.size(), 0);
        continue;
      }

      std::string key = tokens[1];
      std::string value = tokens[2];

      {
        std::lock_guard<std::mutex> lock(store_mutex);
        store[key] = value;
      }

      std::string response = "+OK\r\n";
      send(client_socket, response.c_str(), response.size(), 0);
    }

    else if (command == "GET")
    {
      if (tokens.size() != 2)
      {
        std::string response = error_response("GET");
        send(client_socket, response.c_str(), response.size(), 0);
        continue;
      }

      std::string key = tokens[1];

      std::lock_guard<std::mutex> lock(store_mutex);

      auto it = store.find(key);

      if (it == store.end())
      {
        std::string response = "$-1\r\n";
        send(client_socket, response.c_str(), response.size(), 0);
      }
      else
      {
        std::string value = it->second;

        std::string response =
            "$" + std::to_string(value.size()) + "\r\n" +
            value + "\r\n";

        send(client_socket, response.c_str(), response.size(), 0);
      }
    }

    else if (command == "EXISTS")
    {
      if (tokens.size() != 2)
      {
        std::string response = error_response("EXISTS");
        send(client_socket, response.c_str(), response.size(), 0);
        continue;
      }

      std::string key = tokens[1];

      std::lock_guard<std::mutex> lock(store_mutex);

      auto it = store.find(key);

      std::string response = ":" + std::to_string(it != store.end() ? 1 : 0) + "\r\n";
      send(client_socket, response.c_str(), response.size(), 0);
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
