/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ChannelHelpers.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afogonca <afogonca@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/12 08:41:34 by afogonca          #+#    #+#             */
/*   Updated: 2025/09/12 08:42:19 by afogonca         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/Channel.hpp"

void Channel::sendCantJoin(Client &client, char mode, const std::string &code) {
  std::cout << "Wrong Password" << std::endl;
  std::string msg = ":ft_irc " + code + " " + client.getNick() + " #" + name +
                    " :Cannot join channel (+" + mode + ")\r\n";
  send(client.getFd(), msg.c_str(), msg.size(), 0);
}

void Channel::sendNotSudo(Client &client) {
  std::string msg = ":ft_irc 482 " + client.getNick() + " #" + name +
                    " :You're not channel operator\r\n";
  send(client.getFd(), msg.c_str(), msg.size(), 0);
}

void Channel::sendNotInChannel(Client &client) {
  std::string msg = ":ft_irc 442 " + client.getNick() + " #" + name +
                    " :You're not on that channel\r\n";
  send(client.getFd(), msg.c_str(), msg.size(), 0);
  return;
}

bool Channel::isUserSudo(const std::string &name) {
  for (size_t i = 0; i < sudoUsers.size(); i++) {
    if (ft_strtoupper(name) == ft_strtoupper(sudoUsers[i].getNick())) {
      return true;
    }
  }
  return false;
}

bool Channel::isUserInChannel(const std::string &name) {
  for (size_t i = 0; i < Users.size(); i++) {
    if (ft_strtoupper(name) == ft_strtoupper(Users[i].getNick())) {
      return true;
    }
  }
  return isUserSudo(name);
}

bool Channel::isUserInvited(const std::string &name) {
  for (size_t i = 0; i < Invites.size(); i++) {
    if (ft_strtoupper(name) == ft_strtoupper(Invites[i].getNick())) {
      return true;
    }
  }
  return isUserSudo(name);
}

void Channel::sendJoinMessage(Client &client) {
  std::string msg = ":" + client.getNick() + "!" + client.getUser() +
                    "@host JOIN :#" + name + "\r\n";
  send(client.getFd(), msg.c_str(), msg.size(), 0);
  for (size_t i = 0; i < sudoUsers.size(); i++) {
    sendTopic(sudoUsers[i]);
    sendClientList(sudoUsers[i]);
    sendEndNameList(sudoUsers[i]);
  }
  for (size_t i = 0; i < Users.size(); i++) {
    sendTopic(Users[i]);
    sendClientList(Users[i]);
    sendEndNameList(Users[i]);
  }
}

void Channel::sendEndNameList(Client &client) {
  std::string msg = ":ft_irc 366 " + client.getNick() + " #" + name +
                    " :End of /NAMES list.\r\n";
  send(client.getFd(), msg.c_str(), msg.size(), 0);
}

void Channel::sendClientList(Client &client) {
  bool first = true;
  std::string msg = ":ft_irc 353 " + client.getNick() + " = #" + name + " :";
  for (size_t i = 0; i < sudoUsers.size(); i++) {
    if (!first) {
      msg += " ";
    }
    msg += "@" + sudoUsers[i].getNick();
    first = false;
  }
  for (size_t i = 0; i < Users.size(); i++) {
    if (!first) {
      msg += " ";
    }
    msg += Users[i].getNick();
    first = false;
  }
  msg += "\r\n";
  send(client.getFd(), msg.c_str(), msg.size(), 0);
}

void Channel::sendTopic(Client &client) {
  std::string msg;
  if (topic.empty()) {
    msg = ":ft_irc 331 " + client.getNick() + " #" + name +
          " :No topic is set\r\n";
  } else {
    msg =
        ":ft_irc 332 " + client.getNick() + " #" + name + " :" + topic + "\r\n";
  }
  send(client.getFd(), msg.c_str(), msg.size(), 0);
};

std::string Channel::getName(void) const { return (name); };
