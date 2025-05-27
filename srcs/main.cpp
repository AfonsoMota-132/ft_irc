/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afogonca <afogonca@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/27 09:05:00 by afogonca          #+#    #+#             */
/*   Updated: 2025/05/27 10:48:06 by afogonca         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/Client.hpp"
#include <utility>

int main() {
  int server_fd;
  struct sockaddr_in server_addr;
  int port = 6667;
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    perror("socket");
    return 1;
  }
  fcntl(server_fd, F_SETFL, fcntl(server_fd, F_GETFL, 0) | O_NONBLOCK);
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    perror("bind");
    close(server_fd);
    return 1;
  }

  if (listen(server_fd, SOMAXCONN) < 0) {
    perror("listen");
    close(server_fd);
    return 1;
  }

  std::vector<pollfd> poll_fds;
  pollfd server_pollfd = {server_fd, POLLIN, 0};
  poll_fds.push_back(server_pollfd);

  std::vector<std::pair<int, Client>> Clients;

  std::cout << "Server listening on port " << port << std::endl;

  char buffer[1024];
  while (true) {
    int poll_count = poll(poll_fds.data(), poll_fds.size(), -1);
    if (poll_count < 0) {
      perror("poll");
      break;
    }
    for (size_t i = 0; i < poll_fds.size(); ++i) {
      if (poll_fds[i].revents & POLLIN) {
        if (poll_fds[i].fd == server_fd) {
          Client newClient;
          newClient.acceptConnection(server_fd);
          if (newClient.getFd() >= 0) {
            fcntl(newClient.getFd(), F_SETFL,
                  fcntl(newClient.getFd(), F_GETFL, 0) | O_NONBLOCK);
            Clients.push_back(std::make_pair(newClient.getFd(), newClient));
            struct pollfd new_pollfd;
            new_pollfd.fd = newClient.getFd();
            new_pollfd.events = POLLIN;
            new_pollfd.revents = 0;
            poll_fds.push_back(new_pollfd);
            std::cout << "New client connected: FD = " << newClient.getFd()
                      << std::endl;
          }
        } else {
          int client_fd = poll_fds[i].fd;
          int bytes = read(client_fd, buffer, sizeof(buffer) - 1);
          if (bytes <= 0) {
            std::cout << "Client disconnected: FD = " << client_fd << std::endl;
            close(client_fd);
            poll_fds.erase(poll_fds.begin() + i);
            --i;
          } else {
            buffer[bytes] = '\0';
            std::cout << "Message from FD " << client_fd << ": " << buffer;
            send(client_fd, buffer, bytes, 0);
          }
        }
      }
    }
  }

  close(server_fd);
  return 0;
}
