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

const std::string	ft_strtoupper(const std::string &other)
{
	std::string	result = other;

	for (size_t i = 0; i < result.size(); i++) {
		result[i] = static_cast<char>(std::toupper(result[i]));
	}
	std::cout << result << std::endl;
	return (result);
}

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
int main(int ac, char **av) {
  if (ac != 3) {
    std::cout << "Error\nInvalid Number of Arguments!" << std::endl;
    return (1);
  }
  // signal(SIGINT, &signalHandler);
  data = new Server(std::atoi(av[1]), av[2]);
  if (data->getPort() == -1) {
	return (1);
  }
  std::cout << "Server listening on port " << data->getPort() << std::endl;
  data->serverListen();
  (void)av;
  return 0;
}
