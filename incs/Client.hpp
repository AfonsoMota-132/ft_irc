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

class Client {
private:
  int fd;
  std::string nick;
  std::string user;
  struct sockaddr_in addr;
  socklen_t addrLen;
  bool capLs;
  bool auth;
  // bool pass;

public:
  Client(void);
  ~Client(void);

  int acceptConnection(int server_fd);
  int receiveMessage(char *buffer);
  void closeFd(void);
  int getFd(void);
  bool getCapLs(void);
  bool getAuth(void);
};

#endif
