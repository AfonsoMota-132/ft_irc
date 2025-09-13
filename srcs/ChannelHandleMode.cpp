/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ChannelHandleMode.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afogonca <afogonca@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 11:55:32 by afogonca          #+#    #+#             */
/*   Updated: 2025/09/13 12:32:05 by afogonca         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/Channel.hpp"

void Channel::handleModeK(Client &client,
                          const std::vector<std::string> &tokens, bool add) {
  if (add) {
    if (tokens.size() >= 3) {
      password = tokens[3];
    } else {
      std::string msg = "ft_irc 461 " + client.getNick() + " MODE" +
                        " :Not enough parameters\r\n";
      send(client.getFd(), msg.c_str(), msg.size(), 0);
    };
  } else {
    password = "";
  }
};

void Channel::handleModeO(Client &client,
                          const std::vector<std::string> &tokens, bool add) {
  if (tokens.size() >= 3) {
    if (add) {
      if (!isUserSudo(tokens[2]) && isUserInChannel(tokens[2])) {
        sudoUsers.push_back(client);
      } else if (isUserInChannel(tokens[2])) {
        sendNotInChannel(client);
      }
    } else {
      if (isUserSudo(tokens[2]) && isUserInChannel(tokens[2])) {
        for (size_t i = 0; i < sudoUsers.size(); i++) {
          if (ft_strtoupper(tokens[2]) ==
              ft_strtoupper(sudoUsers[i].getNick())) {
            sudoUsers.erase(sudoUsers.begin() + i);
            return;
          }
        }
      } else if (isUserInChannel(tokens[2])) {
        sendNotInChannel(client);
      }
    }
  } else {
    std::string msg = "ft_irc 461 " + client.getNick() + " MODE" +
                      " :Not enough parameters\r\n";
    send(client.getFd(), msg.c_str(), msg.size(), 0);
  }
};

void Channel::handleModeL(Client &client,
                          const std::vector<std::string> &tokens, bool add) {

};
