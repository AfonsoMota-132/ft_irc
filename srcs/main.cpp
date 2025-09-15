/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afogonca <afogonca@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/27 09:05:00 by afogonca          #+#    #+#             */
/*   Updated: 2025/09/15 11:28:30 by mloureir         ###   ########.pt       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/Client.hpp"
#include "../incs/Server.hpp"

Server *data;

const std::string ft_strtoupper(const std::string &other) {
  std::string result = other;

  for (size_t i = 0; i < result.size(); i++) {
    result[i] = static_cast<char>(std::toupper(result[i]));
  }
  return (result);
}

int	check_digs(std::string inp)
{
	if (inp.length() <= 0)
		return (1);
	for (unsigned long i = 0; i < inp.length(); i++)
	{
		if (std::isdigit(inp[i]) == 0)
			return (1);
	}
	return (0);
}

int	checkAv(int ac, char **av)
{
	if (ac != 3){
		std::cerr << "Error\nInvalid number of arguments: ./irc <port> <password>" << std::endl;
		return (1);
	}
	std::string av1 = av[1];
	if (check_digs(av1) == 1)
	{
		std::cerr << "Error\nInvalid port number" << std::endl;
		return (1);
	}
	return (0);
}

void signalHandler(int signum) {
  if (data) {
    delete data;
    data = NULL;
  }
  std::exit(signum);
}
// Implement size limit of 512 including \r\n, following
// the RFC 1459 protocol
int main(int ac, char **av) {
  if (checkAv(ac ,av) == 1) {
    return (1);
  }
  std::signal(SIGINT, &signalHandler);
  data = new Server(std::atoi(av[1]), av[2]);
  if (data->getPort() == -1) {
	  delete(data);
	  return (1);
  }
  std::cout << "Server listening on port " << data->getPort() << std::endl;
  data->serverListen();
  return 0;
}
