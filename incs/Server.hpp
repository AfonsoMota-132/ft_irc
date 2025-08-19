/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afogonca <afogonca@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 09:05:47 by afogonca          #+#    #+#             */
/*   Updated: 2025/07/09 09:32:52 by afogonca         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"
#include "Channel.hpp"
#include <utility>

class Channel;
class Server {
private:
  int serverFd;
  int port;
  const std::string password;
  struct sockaddr_in serverAddr;
  std::vector<pollfd> pollFds;
  std::vector<Client> Clients;
  std::vector<Channel> Channels;
  char buffer[1024];
  int pollCount;

public:
  Server(int _port, std::string _password);
  ~Server(void);
  
  void serverListen(void);
  int getPort(void) const;
  void closeClientFd(size_t &i, int &clientFd);
  void handleClientMsg(size_t &i, int &clientFd, int &bytes);
  void newClient(void);
  void parseMsg(const std::string &other, size_t &i, int &clientFd);
};

#endif
