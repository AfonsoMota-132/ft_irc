/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afogonca <afogonca@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/14 09:58:17 by afogonca          #+#    #+#             */
/*   Updated: 2025/08/14 09:58:37 by afogonca         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/Channel.hpp"

Channel::Channel(const std::string &_name, Client &client,
                 const std::string &_password)
    : name(_name), password(_password), inv(true), top(false), lim(false) {
  sudoUsers.push_back(client);
  sendJoinMessage(client);
};

void Channel::sendJoinMessage(Client &client) {
  std::string msg = ":" + client.getNick() + "!" + client.getUser() +
                    "@host JOIN :#" + name + "\r\n";
  send(client.getFd(), msg.c_str(), msg.size(), 0);
  for (size_t i = 0; i < sudoUsers.size(); i++) {
    std::string _topic = sendTopic(sudoUsers[i]);
    std::string clientList = sendClientList(sudoUsers[i]);
    std::string clientEnd = ":ft_irc 366 " + sudoUsers[i].getNick() + " #" +
                            name + " :End of /NAMES list.\r\n";
    send(sudoUsers[i].getFd(), _topic.c_str(), _topic.size(), 0);
    send(sudoUsers[i].getFd(), clientList.c_str(), clientList.size(), 0);
    send(sudoUsers[i].getFd(), clientEnd.c_str(), clientEnd.size(), 0);
  }
  for (size_t i = 0; i < Users.size(); i++) {
    std::string _topic = sendTopic(Users[i]);
    std::string clientList = sendClientList(Users[i]);
    std::string clientEnd = ":ft_irc 366 " + Users[i].getNick() + " #" + name +
                            " :End of /NAMES list.\r\n";
    send(Users[i].getFd(), _topic.c_str(), _topic.size(), 0);
    send(Users[i].getFd(), clientList.c_str(), clientList.size(), 0);
    send(Users[i].getFd(), clientEnd.c_str(), clientEnd.size(), 0);
  }
}

std::string Channel::sendClientList(Client &client) {
  bool first = true;
  std::string clientList =
      ":ft_irc 353 " + client.getNick() + " = #" + name + " :";
  for (size_t i = 0; i < sudoUsers.size(); i++) {
    if (!first) {
      clientList += " ";
    }
    clientList += "@" + sudoUsers[i].getNick();
    first = false;
  }
  for (size_t i = 0; i < Users.size(); i++) {
    if (!first) {
      clientList += " ";
    }
    clientList += Users[i].getNick();
    first = false;
  }
  clientList += "\r\n";
  return (clientList);
}

std::string Channel::sendTopic(Client &client) {
  std::string _topic;
  if (topic.empty()) {
    _topic = ":ft_irc 331 " + client.getNick() + " #" + name +
             " :No topic is set\r\n";
  } else {
    _topic =
        ":ft_irc 332 " + client.getNick() + " #" + name + " :" + topic + "\r\n";
  }
  return _topic;
};

void Channel::join(Client &user, const std::string &pass, bool sudo) {
  if (!password.empty() && password != pass) {
    std::cout << "Wrong Password" << std::endl;
    std::string msg = ":ft_irc 475 " + user.getNick() + " #" + name +
                      " :Cannot join channel (+k)\r\n";
    send(user.getFd(), msg.c_str(), msg.size(), 0);
    return;
  }
  if (inv) {
    bool wasInvited = false;
    for (unsigned int i = 0; i < Invites.size(); i++) {
      std::cout << user.getNick() << "\t" << Invites[i].getNick() << std::endl;
      if (user.getNick() == Invites[i].getNick()) {
        wasInvited = true;
        break;
      }
    }
    if (!wasInvited) {
      std::string msg = ":ft_irc 473 " + user.getNick() + " #" + name +
                        " :Cannot join channel (+i)\r\n";
      send(user.getFd(), msg.c_str(), msg.size(), 0);
      return;
    }
  }
  if (!sudo) {
    Users.push_back(user);
  } else {
    sudoUsers.push_back(user);
  }
  sendJoinMessage(user);
}

void Channel::invite(Client &user, Client &invited) {
  bool isSudo = false;
  for (size_t i = 0; i < sudoUsers.size(); i++) {
    if (user.getNick() == sudoUsers[i].getNick()) {
      isSudo = true;
      break;
    }
  }
  if (!isSudo) {
    std::string msg = ":ft_irc 482 " + user.getNick() + " #" + name +
                      " :You're not channel operator\r\n";
    send(user.getFd(), msg.c_str(), msg.size(), 0);
  } else {
    for (size_t i = 0; i < sudoUsers.size(); i++) {
      if (invited.getNick() == sudoUsers[i].getNick()) {
        std::string msg = ":ft_irc 443 " + user.getNick() + " " +
                          invited.getNick() + " #" + name +
                          " :Is already on Channel\r\n";
        send(user.getFd(), msg.c_str(), msg.size(), 0);
        return;
      }
    }
    for (size_t i = 0; i < Users.size(); i++) {
      if (invited.getNick() == Users[i].getNick()) {
        std::string msg = ":ft_irc 443 " + user.getNick() + " " +
                          invited.getNick() + " #" + name +
                          " :Is already on Channel\r\n";
        send(user.getFd(), msg.c_str(), msg.size(), 0);
        return;
      }
    }
    Invites.push_back(invited);
  }
  (void)invited;
}

Channel::~Channel(void) {};

std::string Channel::getName(void) const { return (name); };
