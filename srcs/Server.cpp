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

  // Allow reuse of address to avoid "Address already in use" error
  int opt = 1;
  if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    perror("setsockopt");
    close(serverFd);
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
    pollCount = poll(&pollFds[0], pollFds.size(), -1);
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
          int bytes = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
          if (bytes <= 0) {
            if (bytes == 0) {
              disconnectClient(i, clientFd, "Client disconnected");
            } else {
              disconnectClient(i, clientFd, "Connection error");
            }
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
  if (i < pollFds.size())
    pollFds.erase(pollFds.begin() + i);
  if (i - 1 < Clients.size())
    Clients.erase(Clients.begin() + (i - 1));
};

void Server::disconnectClient(size_t i, int clientFd,
                              const std::string &reason) {
  if (i > 0 && (i - 1) < Clients.size()) {
    Client &client = Clients[i - 1];

    // Send QUIT message if client is authenticated
    if (client.getAuth()) {
      std::string quitMsg = ":" + client.getNick() + "!" + client.getUser() +
                            "@host QUIT :" + reason + "\r\n";
      send(clientFd, quitMsg.c_str(), quitMsg.size(), 0);
    } else {
      // Send ERROR message for unauthenticated clients
      std::string errorMsg = "ERROR :Closing Link: " + reason + "\r\n";
      send(clientFd, errorMsg.c_str(), errorMsg.size(), 0);
    }
  }

  // Small delay to ensure message is sent
  usleep(100000); // 100ms
  closeClientFd(i, clientFd);
}

void Server::kickClient(size_t i, int clientFd, const std::string &reason) {
  // Send ERROR message for server-initiated disconnections
  std::string errorMsg = "ERROR :Closing Link: " + reason + "\r\n";
  send(clientFd, errorMsg.c_str(), errorMsg.size(), 0);

  // Small delay to ensure message is sent
  usleep(100000); // 100ms
  closeClientFd(i, clientFd);
};

void Server::handleClientMsg(size_t &i, int &clientFd, int &bytes) {
  buffer[bytes] = '\0';
  std::string tmp = buffer;
  if (bytes >= 512)
    send(clientFd, "Error: Message is too big!\n", 27, 0);
  // else if (Clients[i - 1].getAuth() == true) {
  //   std::cout << "Message from FD " << clientFd << ": " << tmp;
  //   send(clientFd, buffer, bytes, 0);
  else {
    while (tmp.find_first_of("\r\n") == std::string::npos) {
      int tmpBytes = recv(clientFd, buffer, sizeof(buffer) - bytes - 1, 0);
      if (tmpBytes > 0) {
        buffer[tmpBytes] = '\0';
        tmp += buffer;
        bytes += tmpBytes;
      }
      if (bytes >= 512)
        break;
    }
    if (bytes >= 512)
      send(clientFd, "Error: Message is too big!\n", 27, 0);
    else {
      size_t start = 0;
      size_t end = tmp.find("\r\n");

      while (end != std::string::npos) {
        std::string message = tmp.substr(start, end - start);
        if (!message.empty()) {
          parseMsg(message, i, clientFd);
        }
        start = end + 2;
        end = tmp.find("\r\n", start);
      }
    }
  }
};

void Server::privMsg(const std::vector<std::string> &tokens, int clientFd,
                     Client &client) {
  bool foundClient = false;
  if (tokens.size() == 2 || tokens[2].empty()) {
    std::string errorMsg =
        ":irc 461 " + client.getNick() + " PRIVMSG :Not enough parameters\r\n";
    send(clientFd, errorMsg.c_str(), errorMsg.size(), 0);
  } else {
    for (size_t i = 0; i < Clients.size(); i++) {
      if (ft_strtoupper(Clients[i].getNick()) == ft_strtoupper(tokens[1])) {
        client.sendMessage(tokens[2], Clients[i].getNick(), Clients[i].getFd());
        foundClient = true;
        break;
      }
    }
    if (!foundClient) {
      std::string errorMsg = ":irc 401 " + client.getNick() + " " + tokens[1] +
                             " :No such nick/channel\r\n";
      send(clientFd, errorMsg.c_str(), errorMsg.size(), 0);
    }
  }
}

void Server::handleChannels(const std::vector<std::string> &tokens,
                            Client &client) {
  bool channelExists = false;
  for (size_t i = 0; i < Channels.size(); i++) {
    if (ft_strtoupper(Channels[i].getName()) == ft_strtoupper(tokens[1])) {
      if (tokens.size() >= 3) {
        Channels[i].join(client, tokens[2], 0);
      } else {
        Channels[i].join(client, "", 0);
      }
	  std::cout << "wtf\n" << std::endl;
      channelExists = true;
    }
  }
  if (!channelExists) {
    std::string password = "";
    if (tokens.size() >= 3)
      password = tokens[2];
    Channel newChannel(tokens[1], client, password);
    std::cout << "Creating new channel!" << std::endl;
	Channels.push_back(newChannel);
  }
  (void)client;
}

void Server::parseMsg(const std::string &other, size_t i, int clientFd) {
  std::vector<std::string> tokens;
  std::string current = "";
  bool inTrailing = false;

  for (size_t j = 0; j < other.length(); ++j) {
    char c = other[j];
    if (!inTrailing && c == ':') {
      if (!current.empty()) {
        tokens.push_back(current);
        current = "";
      }
      inTrailing = true;
      continue;
    } else if (!inTrailing && c == ' ') {
      if (!current.empty()) {
        tokens.push_back(current);
        current = "";
      }
      while (j + 1 < other.length() && other[j + 1] == ' ') {
        ++j;
      }
    } else if (c == '\r' || c == '\n') {
      continue;
    } else {
      current += c;
    }
  }
  if (!current.empty()) {
    tokens.push_back(current);
  }
  for (size_t k = 0; k < tokens.size(); ++k) {
    std::cout << "Token[" << k << "]: " << tokens[k] << std::endl;
  }
  Client &client = Clients[i - 1];
  int authResult = 0;
  if (!client.getAuth()) {
    authResult = client.authenticate(tokens, password, Clients);
  } else {
    if (tokens.size() >= 2 && ft_strtoupper(tokens[0]) == "PRIVMSG") {
      privMsg(tokens, clientFd, client);
    } else if (tokens.size() >= 2 && ft_strtoupper(tokens[0]) == "JOIN") {
      handleChannels(tokens, client);
    }
  }
  // Handle authentication result
  if (authResult == 1) {
    // Authentication failed, disconnect client
    kickClient(i, clientFd, "Authentication failed");
  }
  // authResult == 0 means continue processing (success or waiting for more auth
  // steps)

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
