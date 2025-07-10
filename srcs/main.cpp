/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afogonca <afogonca@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/27 09:05:00 by afogonca          #+#    #+#             */
/*   Updated: 2025/07/09 09:29:13 by afogonca         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/Client.hpp"
#include "../incs/Server.hpp"

Server *data;

void signalHandler(int signum) {
  if (signum == SIGPIPE || signum == SIGSEGV)
    return;
  if (data) {
    delete data;
    data = NULL;
  }
  _exit(signum);
}
// Implement size limit of 512 including \r\n, following
// the RFC 1459 protocol
int main(void) {

  signal(SIGINT, &signalHandler);
  data = new Server(6667);
  std::cout << "Server listening on port " << data->getPort() << std::endl;
  data->serverListen();

  return 0;
}
