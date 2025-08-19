/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afogonca <afogonca@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 09:10:24 by afogonca          #+#    #+#             */
/*   Updated: 2025/07/09 09:32:41 by afogonca         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/Server.hpp"

// subs return for exceptions
Server::Server(int _port, std::string _password)
    : port(_port), password(_password) {
  serverFd = socket(AF_INET, SOCK_STREAM, 0);
  if (serverFd < 0) {
    perror("socket");
    return;
  }
  fcntl(serverFd, F_SETFL, fcntl(serverFd, F_GETFL, 0) | O_NONBLOCK);
  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  serverAddr.sin_port = htons(port);
  if (bind(serverFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
    perror("bind");
    close(serverFd);
    this->port = -1;
    return;
  }
  if (listen(serverFd, SOMAXCONN) < 0) {
    perror("listen");
    close(serverFd);
    return;
  }
  pollfd serverPollFd = {serverFd, POLLIN, 0};

  pollFds.push_back(serverPollFd);
};

Server::~Server(void) { close(serverFd); }

void Server::serverListen(void) {
  while (true) {
    pollCount = poll(pollFds.data(), pollFds.size(), -1);
    if (pollCount < 0) {
      perror("poll");
      break;
    }
    for (size_t i = 0; i < pollFds.size(); ++i) {
      if (pollFds[i].revents & POLLIN) {
        if (pollFds[i].fd == serverFd) {
          newClient();
        } else {
          int clientFd = pollFds[i].fd;
          int bytes = read(clientFd, buffer, sizeof(buffer) - 1);
          if (bytes <= 0) {
            closeClientFd(i, clientFd);
          } else {
            handleClientMsg(i, clientFd, bytes);
          }
        }
      }
    }
  }
};

int Server::getPort(void) const { return (this->port); };

void Server::closeClientFd(size_t &i, int &clientFd) {
  std::cerr << "Client disconnected: FD = " << clientFd << std::endl;
  close(clientFd);
  pollFds.erase(pollFds.begin() + i);
  Clients.erase(Clients.begin() + (i - 1));
};

void Server::handleClientMsg(size_t &i, int &clientFd, int &bytes) {
  buffer[bytes] = '\0';
  std::string tmp = buffer;
  if (bytes >= 512)
    send(clientFd, "Error: Message is too big!\n", 27, 0);
  else if (Clients[i - 1].getAuth() == true) {
    std::cout << "Message from FD " << clientFd << ": " << tmp;
    send(clientFd, buffer, bytes, 0);
  } else {
    while (tmp.find_first_of("\r\n") == std::string::npos) {
      int tmpBytes = read(clientFd, buffer, sizeof(buffer) - bytes - 1);
      if (tmpBytes != -1) {
        tmp += buffer;
        bytes += tmpBytes;
      }
      if (bytes >= 512)
        break;
    }
    if (bytes >= 512)
      send(clientFd, "Error: Message is too big!\n", 27, 0);
    else
	  parseMsg(tmp, i, clientFd);
      // std::cout << "Message from FD " << clientFd << ":\n" << tmp;
  }
};

void Server::parseMsg(const std::string &other, size_t &i, int &clientFd) {
  if (Clients[i - 1].getCapLs() == false) {
    std::string tmp(other);
    for (size_t i = 0; i < tmp.size(); i++) {
      tmp[i] = std::toupper(tmp[i]);
    }
    std::cout << tmp << std::endl;
  }
  // if (other == "QUIT :Leaving\r\n")
  // 	closeClientFd(i, clientFd);
  (void)other;
  (void)i;
  (void)clientFd;
};

void Server::newClient(void) {
  Client newClient;
  newClient.acceptConnection(serverFd);
  if (newClient.getFd() >= 0) {
    fcntl(newClient.getFd(), F_SETFL,
          fcntl(newClient.getFd(), F_GETFL, 0) | O_NONBLOCK);
    Clients.push_back(newClient);
    struct pollfd new_pollfd;
    new_pollfd.fd = newClient.getFd();
    new_pollfd.events = POLLIN;
    new_pollfd.revents = 0;
    pollFds.push_back(new_pollfd);
    std::cout << "New client connected: FD = " << newClient.getFd()
              << std::endl;
    // Channel newChannel("channel", "Topic we're discussing");
    // Channels.push_back(newChannel);
    //   std::string f0 = ":afogonca!user@host JOIN :#channel\r\n";
    //   std::string f1 =
    //       ":server 332 afogonca #channel :Topic we're discussing\r\n";
    //   std::string f2 = ":server 353 afogonca = #channel :@afogonca\r\n";
    //   std::string f3 = ":server 366 afogonca #channel :End of /NAMES
    //   list.\r\n";
    // send(newClient.getFd(), f0.c_str(), f0.size(), 0);
    // send(newClient.getFd(), f1.c_str(), f1.size(), 0);
    // send(newClient.getFd(), f2.c_str(), f2.size(), 0);
    // send(newClient.getFd(), f3.c_str(), f3.size(), 0);
  }
};
