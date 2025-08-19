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
    : fd(-1), addrLen(sizeof(addr)), capLs(false), auth(false) {
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

int Client::getFd(void) { return (this->fd); }
bool Client::getCapLs(void) { return (this->capLs); }
bool Client::getAuth(void) { return (this->auth); }
