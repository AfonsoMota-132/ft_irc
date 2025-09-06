/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afogonca <afogonca@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/27 09:45:22 by afogonca          #+#    #+#             */
/*   Updated: 2025/05/27 10:00:17 by afogonca         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/Client.hpp"

Client::Client(void)
    : fd(-1), nick(""), user(""), addrLen(sizeof(addr)), capLs(false),
      pass(false), auth(false) {
  std::cout << "Client's Default Constructor called" << std::endl;
};
Client::~Client(void) {
  std::cout << "Client's Default destructor called" << std::endl;
};

int Client::acceptConnection(int server_fd) {
  fd = accept(server_fd, (struct sockaddr *)&addr, &addrLen);
  if (fd < 0) {
    perror("accept");
    fd = -1;
    close(server_fd);
    return (1);
  }
  return (0);
}

int Client::receiveMessage(char *buffer) {
  ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
  if (bytes_read < 0) {
    perror("read");
    close(fd);
    close(fd);
    return (1);
  }
  if (bytes_read == 0)
    return (2);

  buffer[bytes_read] = '\0';
  printf("%s\n", buffer);
  return (0);
}

void Client::closeFd(void) { close(fd); }

int Client::getFd(void) const { return (this->fd); }
bool Client::getCapLs(void) const { return (this->capLs); }
bool Client::getAuth(void) const { return (this->auth); }
const std::string Client::getNick(void) const { return (this->nick); }
const std::string Client::getUser(void) const { return (this->user); }

bool Client::authCapLs(std::vector<std::string> &tokens) {
  if (tokens.size() == 3 && ft_strtoupper(tokens[0]) == "CAP" &&
      ft_strtoupper(tokens[1]) == "LS" && tokens[2] == "302") {
    capLs = true;
    std::string msg = ":server CAP * LS :\r\n";
    send(fd, msg.c_str(), msg.size(), 0);
    return true;
  } else {
    std::cout << "Expected CAP LS command first" << std::endl;
    std::string msg = "ERROR :CAP LS required\r\n";
    send(fd, msg.c_str(), msg.size(), 0);
    return false;
  }
}

bool Client::authPass(std::vector<std::string> &tokens,
                      const std::string &password) {
  if (tokens.size() != 2 || ft_strtoupper(tokens[0]) != "PASS") {
    std::string msg = "ERROR :Invalid PASS command format\r\n";
    send(fd, msg.c_str(), msg.size(), 0);
    return false;
  }

  if (tokens[1] == password) {
    pass = true;
    std::cout << "Password accepted for FD " << fd << std::endl;
    return true;
  } else {
    std::string msg = "ERROR :Bad password\r\n";
    send(fd, msg.c_str(), msg.size(), 0);
    return false;
  }
}

bool Client::authNick(std::vector<std::string> &tokens,
                      const std::vector<Client> &Clients) {
  if (tokens.size() < 2 || ft_strtoupper(tokens[0]) != "NICK") {
    return true; // Not a NICK command, continue processing
  }
  if (!pass) {
    std::string msg = ":server 464 * :Password required\r\n";
    send(fd, msg.c_str(), msg.size(), 0);
    return false; // FATAL: Password required
  }
  if (tokens[1].empty()) {
    std::string msg = ":server 431 * :No nickname given\r\n";
    send(fd, msg.c_str(), msg.size(), 0);
    return false; // FATAL: Invalid nickname
  }
  // Check for invalid characters in nickname
  const std::string &nickname = tokens[1];
  if (nickname.length() > 9) { // IRC nickname limit
    std::string msg = ":server 432 * " + nickname + " :Erroneous nickname\r\n";
    send(fd, msg.c_str(), msg.size(), 0);
    return false; // FATAL: Nickname too long
  }
  for (size_t i = 0; i < Clients.size(); i++) {
    if (ft_strtoupper(Clients[i].nick) == ft_strtoupper(tokens[1])) {
      std::string msg =
          ":server 432 * " + nickname + " :Nickname is already in use\r\n";
      send(fd, msg.c_str(), msg.size(), 0);
      return true;
    }
  }
  nick = tokens[1];
  std::cout << "NICK set to: " << nick << " for FD " << fd << std::endl;
  // Check if we can complete authentication
  if (!nick.empty() && !user.empty()) {
    auth = true;
    sendWelcomeMessages();
  }
  return true;
}

bool Client::authUser(std::vector<std::string> &tokens) {
  std::cout << tokens.size() << "\t" << fd << std::endl;
  if (tokens.size() < 5 || ft_strtoupper(tokens[0]) != "USER") {
    return true; // Not a USER command, continue processing
  }

  if (!pass) {
    std::string msg = ":server 464 * :Password required\r\n";
    send(fd, msg.c_str(), msg.size(), 0);
    return false; // FATAL: Password required
  }

  if (tokens[1].empty()) {
    std::string msg =
        "ERROR :Invalid USER command - username cannot be empty\r\n";
    send(fd, msg.c_str(), msg.size(), 0);
    return false; // FATAL: Empty username
  }

  user = tokens[1];
  std::cout << "USER set to: " << user << " for FD " << fd << std::endl;
  // Check if we can complete authentication
  if (!nick.empty() && !user.empty()) {
    auth = true;
    sendWelcomeMessages();
  }

  return true;
}

void Client::sendWelcomeMessages() {
  std::cout << "Client authenticated successfully! Nick: " << nick
            << ", User: " << user << ", FD: " << fd << std::endl;

  // RPL_WELCOME (001)
  std::string welcome = ":server 001 " + nick + " :Welcome to the IRC Server " +
                        nick + "!" + user + "@host\r\n";
  send(fd, welcome.c_str(), welcome.size(), 0);

  // RPL_YOURHOST (002)
  std::string yourhost =
      ":server 002 " + nick + " :Your host is server, running version 1.0\r\n";
  send(fd, yourhost.c_str(), yourhost.size(), 0);

  // RPL_CREATED (003)
  std::string created =
      ":server 003 " + nick + " :This server was created today\r\n";
  send(fd, created.c_str(), created.size(), 0);

  // RPL_MYINFO (004)
  std::string myinfo = ":server 004 " + nick + " server 1.0 o o\r\n";
  send(fd, myinfo.c_str(), myinfo.size(), 0);
}

int Client::authenticate(std::vector<std::string> &tokens,
                         const std::string &password,
                         const std::vector<Client> &Clients) {
  // Handle empty command as fatal error
  if (tokens.empty()) {
    std::string msg = "ERROR :Empty command received\r\n";
    send(fd, msg.c_str(), msg.size(), 0);
    return 1; // FATAL: Empty command
  }

  // If already authenticated, ignore authentication commands
  if (auth) {
    if (ft_strtoupper(tokens[0]) == "CAP" ||
        ft_strtoupper(tokens[0]) == "PASS" ||
        ft_strtoupper(tokens[0]) == "NICK" ||
        ft_strtoupper(tokens[0]) == "USER") {
      return 0; // Silently ignore
    }
    return 0; // Let other commands be processed normally
  }

  // Step 1: CAP LS (optional but expected by modern clients)
  if (!capLs) {
    if (ft_strtoupper(tokens[0]) == "CAP") {
      return authCapLs(tokens) ? 0 : 1; // FATAL on any CAP error
    } else {
      std::string msg = "ERROR :You must send CAP LS first\r\n";
      send(fd, msg.c_str(), msg.size(), 0);
      return 1; // FATAL: No CAP LS
    }
  }
  // Step 2: PASS command
  if (!pass) {
    if (ft_strtoupper(tokens[0]) == "PASS") {
      return authPass(tokens, password) ? 0 : 1; // FATAL on wrong password
    } else {
      std::string msg = "ERROR :Password required before " + tokens[0] + "\r\n";
      send(fd, msg.c_str(), msg.size(), 0);
      return 1; // FATAL: No password provided
    }
  }
  // Step 3: NICK and USER commands (can be in any order)
  if (ft_strtoupper(tokens[0]) == "NICK") {
    return authNick(tokens, Clients) ? 0 : 1; // FATAL on any NICK error
  } else if (ft_strtoupper(tokens[0]) == "USER") {
    return authUser(tokens) ? 0 : 1; // FATAL on any USER error
  } else if (ft_strtoupper(tokens[0]) == "PASS") {
    // Password already provided - this is a fatal error
    std::string msg = "ERROR :Password already provided\r\n";
    send(fd, msg.c_str(), msg.size(), 0);
    return 0; // FATAL: Duplicate PASS command
  } else if (ft_strtoupper(tokens[0]) == "CAP") {
    // CAP command after authentication started - fatal error
    std::string msg =
        "ERROR :CAP commands not allowed after authentication\r\n";
    send(fd, msg.c_str(), msg.size(), 0);
    return 0; // FATAL: Late CAP command
  } else {
    // Unknown command during authentication - fatal error
    std::string msg =
        "ERROR :Unknown command during authentication: " + tokens[0] + "\r\n";
    send(fd, msg.c_str(), msg.size(), 0);
    return 1; // FATAL: Unknown command during auth
  }
}

void Client::sendMessage(const std::string &msg, const std::string &user,
                         int fd) const {
  std::string message = ":" + nick + "!" + user + "@localhost PRIVMSG " + user +
                        " :" + msg + "\r\n";
  std::cout << "sent : \"" << message << "\" to fd: " << fd << std::endl;
  send(fd, message.c_str(), message.size(), 0);
}
