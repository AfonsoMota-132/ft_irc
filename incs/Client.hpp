/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afogonca <afogonca@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/27 09:34:48 by afogonca          #+#    #+#             */
/*   Updated: 2025/05/27 09:47:27 by afogonca         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef Client_HPP
#define Client_HPP

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

const std::string ft_strtoupper(const std::string &other);

class Client {
private:
  int fd;
  std::string nick;
  std::string user;
  std::string buffer;
  struct sockaddr_in addr;
  socklen_t addrLen;
  bool capLs;
  bool pass;
  bool auth;

  bool authCapLs(std::vector<std::string> &tokens);
  bool authPass(std::vector<std::string> &tokens, const std::string &password);
  bool authNick(std::vector<std::string> &tokens,
                const std::vector<Client> &Clients);
  bool authUser(std::vector<std::string> &tokens);
  void sendWelcomeMessages();

public:
  Client(void);
  ~Client(void);

  int acceptConnection(int server_fd);
  void setBuffer(const std::string &msg);
  std::string getBuffer(void) const;
  void closeFd(void);
  int getFd(void) const;
  bool getCapLs(void) const;
  bool getAuth(void) const;
  const std::string getNick(void) const;
  const std::string getUser(void) const;
  void handleQuit(const std::vector<std::string> &tokens);
  int authenticate(std::vector<std::string> &tokens, const std::string &other,
                   const std::vector<Client> &Clients);
  void sendMessage(const std::string &msg, const std::string &user,
                   int fd) const;
};

#endif
