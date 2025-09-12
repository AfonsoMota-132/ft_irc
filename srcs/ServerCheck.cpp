/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerCheck.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afogonca <afogonca@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/12 09:31:03 by afogonca          #+#    #+#             */
/*   Updated: 2025/09/12 09:42:21 by afogonca         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/Server.hpp"

int Server::channelExists(const std::string &channel) {
  if (channel.empty() || channel.at(0) != '#') {
    return -1;
  } else {
    for (size_t i = 0; i < Channels.size(); i++) {
      if (ft_strtoupper(channel.substr(1)) ==
          ft_strtoupper(Channels[i].getName())) {
        return i;
        break;
      }
    }
    return -1;
  }
}

int Server::userExists(const std::string &user) {
  for (size_t i = 0; i < Clients.size(); i++) {
    if (ft_strtoupper(user) == ft_strtoupper(Clients[i].getNick())) {
      return i;
    }
  }
  return -1;
}

void Server::sendNoChannel(Client &client, const std::string &name) {
  std::string msg = ":ft_irc 403 " + client.getNick() + " " + name +
                    " :No such channel\r\n";
  send(client.getFd(), msg.c_str(), msg.size(), 0);
}
