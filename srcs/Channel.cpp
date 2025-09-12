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

void Channel::join(Client &client, const std::string &pass, bool sudo) {
  if (!password.empty() && password != pass) {
    std::cout << "Wrong Password" << std::endl;
    sendCantJoin(client, 'k', "475");
    return;
  }
  if (inv) {
    if (!isUserInvited(client.getNick())) {
      sendCantJoin(client, 'i', "473");
      return;
    }
  }
  if (!sudo) {
    Users.push_back(client);
  } else {
    sudoUsers.push_back(client);
  }
  sendJoinMessage(client);
}

void Channel::kick(Client &client, const std::vector<std::string> &tokens) {
  if (!isUserInChannel(client.getNick())) {
    sendNotInChannel(client);
    return;
  } else if (!isUserSudo(client.getNick())) {
    sendNotSudo(client);
    return;
  } else {
    int userExists = -1;
    bool kickSudo = false;
    for (size_t i = 0; i < sudoUsers.size(); i++) {
      if (ft_strtoupper(tokens[2]) == ft_strtoupper(sudoUsers[i].getNick())) {
        kickSudo = true;
        userExists = i;
        break;
      }
    }
    if (userExists == -1) {
      for (size_t i = 0; i < Users.size(); i++) {
        if (ft_strtoupper(tokens[2]) == ft_strtoupper(Users[i].getNick())) {
          userExists = i;
          break;
        }
      }
    }
    if (userExists == -1) {
      std::string msg = ":ft_irc 441 " + client.getNick() + " " + tokens[2] +
                        " #" + name + " :They aren't on that channel\r\n";
      send(client.getFd(), msg.c_str(), msg.size(), 0);
      return;
    } else {
      std::string msg = ":" + client.getNick() + "!" + client.getUser() +
                        "@localhost KICK #" + name + " " + tokens[2];
      if (tokens.size() >= 4) {
        msg += " :" + tokens[3] + "\r\n";
      } else {
        msg += "\r\n";
      }
      for (size_t i = 0; i < sudoUsers.size(); i++) {
        send(sudoUsers[i].getFd(), msg.c_str(), msg.size(), 0);
      }
      for (size_t i = 0; i < Users.size(); i++) {
        send(Users[i].getFd(), msg.c_str(), msg.size(), 0);
      }
      if (kickSudo) {
        sudoUsers.erase(sudoUsers.begin() + userExists);
      } else {
        Users.erase(Users.begin() + userExists);
      }
    }
  }
}

void Channel::handleTopic(Client &client,
                          const std::vector<std::string> &tokens) {
  if (!isUserInChannel(client.getNick())) {
    sendNotInChannel(client);
  } else if (tokens.size() == 2) {
    sendTopic(client);
  } else {
    if (top && !isUserSudo(client.getNick())) {
      sendNotSudo(client);
      return;
    } else {
      topic = tokens[2];
    }
  }
}

void Channel::invite(Client &client, Client &invited) {
  if (!isUserInChannel(client.getNick())) {
    sendNotInChannel(client);
    return;
  } else if (!isUserSudo(client.getNick())) {
    sendNotSudo(client);
    return;
  } else {
    if (isUserInChannel(invited.getNick())) {
      std::string msg = ":ft_irc 443 " + client.getNick() + " " +
                        invited.getNick() + " #" + name +
                        " :Is already on Channel\r\n";
      send(client.getFd(), msg.c_str(), msg.size(), 0);
      return;
    } else {
      Invites.push_back(invited);
    }
  }
}

Channel::~Channel(void) {};
