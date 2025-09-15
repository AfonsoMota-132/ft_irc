/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afogonca <afogonca@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 09:10:24 by afogonca          #+#    #+#             */
/*   Updated: 2025/09/12 09:52:32 by afogonca         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/Server.hpp"

// subs return for exceptions
Server::Server(int _port, std::string _password)
    : port(_port), password(_password) {
  serverFd = socket(AF_INET, SOCK_STREAM, 0);
  if (serverFd < 0) {
    port = -1;
    std::cerr << "Error: Failed to create socket" << std::endl;
    return;
  }

  // Allow reuse of address to avoid "Address already in use" error
  int opt = 1;
  if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    port = -1;
    std::cerr << "Error: Failed to set socket" << std::endl;
    close(serverFd);
    return;
  }
  fcntl(serverFd, F_SETFL, fcntl(serverFd, F_GETFL, 0) | O_NONBLOCK);
  serverAddr.sin_family = 0;
  serverAddr.sin_port = 0;
  serverAddr.sin_addr.s_addr = 0;

  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  serverAddr.sin_port = htons(port);
  if (bind(serverFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
    std::cerr << "Error: Failed to bind socket to address" << std::endl;
    close(serverFd);
    this->port = -1;
    return;
  }
  if (listen(serverFd, SOMAXCONN) < 0) {
    std::cerr << "Error: Failed to listen on socket" << std::endl;
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
      std::cerr << "Error: poll() failed" << std::endl;
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
  if (i > 0 && (i - 1) < Clients.size() && !reason.empty()) {
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
  closeClientFd(i, clientFd);
}

void Server::kickClient(size_t i, int clientFd, const std::string &reason) {
  std::string errorMsg = "ERROR :Closing Link: " + reason + "\r\n";
  send(clientFd, errorMsg.c_str(), errorMsg.size(), 0);
  closeClientFd(i, clientFd);
};

void Server::handleClientMsg(size_t &i, int &clientFd, int &bytes) {
  buffer[bytes] = '\0';
  std::string tmp = buffer;
  if (bytes >= 512) {
    std::string msg =
        ":ft_irc 903 " + Clients[i - 1].getNick() + " :Message too long";
    send(clientFd, msg.c_str(), msg.size(), 0);
  } else {
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

void Server::privMsg(const std::vector<std::string> &tokens, Client &client) {
  if (tokens.size() <= 2 || tokens[2].empty()) {
    sendNoParams(client, "PRIVMSG");
    return;
  } else if (userExists(tokens[1]) != -1) {
    int userI = userExists(tokens[1]);
    client.sendMessage(tokens[2], Clients[userI].getNick(),
                       Clients[userI].getFd());
    return;
  } else if (channelExists(tokens[1]) != -1) {
    int channelI = channelExists(tokens[1]);
    if (channelI == -1) {
      sendNoChannel(client, tokens[1]);
    } else {
      Channels[channelI].privMsg(client, tokens);
    }
  } else {
    std::string msg = ":ft_irc 401 " + client.getNick() + " " + tokens[1] +
                      " :No such nick/channel\r\n";
    send(client.getFd(), msg.c_str(), msg.size(), 0);
  }
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
    if (ft_strtoupper(tokens[0]) == "PRIVMSG") {
      privMsg(tokens, client);
    } else if (ft_strtoupper(tokens[0]) == "JOIN") {
      handleJoin(tokens, client);
    } else if (ft_strtoupper(tokens[0]) == "KICK") {
      handleKick(tokens, client);
    } else if (ft_strtoupper(tokens[0]) == "INVITE") {
      handleInvite(tokens, client);
    } else if (ft_strtoupper(tokens[0]) == "TOPIC") {
      handleTopic(tokens, client);
    } else if (ft_strtoupper(tokens[0]) == "MODE") {
      handleMode(tokens, client);
    } else if (ft_strtoupper(tokens[0]) == "QUIT") {
      handleQuit(tokens, client);
      disconnectClient(i, clientFd, "");
    }
  }
  // Handle authentication result
  if (authResult == 1) {
    // Authentication failed, disconnect client
    kickClient(i, clientFd, "Authentication failed");
  }
  // authResult == 0 means continue processing (success or waiting for more
  // auth steps)
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
  }
};
