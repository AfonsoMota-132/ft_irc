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

void Channel::sendNotInServer(Client &client) {
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

bool Channel::isUserInServer(const std::string &name) {
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
  if (!isUserInServer(client.getNick())) {
    sendNotInServer(client);
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
  if (!isUserInServer(client.getNick())) {

  } else if (top && !isUserSudo(client.getNick())) {
    sendNotSudo(client);
    return;
  } else {
  }
  (void)tokens;
}

void Channel::invite(Client &client, Client &invited) {
  if (!isUserInServer(client.getNick())) {
    sendNotInServer(client);
    return;
  } else if (!isUserSudo(client.getNick())) {
    sendNotSudo(client);
    return;
  } else {
    if (isUserInServer(invited.getNick())) {
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

std::string Channel::getName(void) const { return (name); };
