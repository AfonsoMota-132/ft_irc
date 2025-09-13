/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afogonca <afogonca@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/14 09:58:17 by afogonca          #+#    #+#             */
/*   Updated: 2025/09/13 12:22:20 by afogonca         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/Channel.hpp"

Channel::Channel(const std::string &_name, Client &client,
                 const std::string &_password)
    : name(_name), password(_password), inv(false), top(false), lim(-1) {
  sudoUsers.push_back(client);
  sendJoinMessage(client);
};

void Channel::privMsg(Client &client, const std::vector<std::string> &tokens) {
  if (!isUserInChannel(client.getNick())) {
    sendNotInChannel(client);
  } else {
    std::string msg = ":" + client.getNick() + "!" + client.getUser() +
                      "@localhost PRIVMSG #" + name + " :" + tokens[2] + "\r\n";
    for (size_t i = 0; i < sudoUsers.size(); i++) {
      send(sudoUsers[i].getFd(), msg.c_str(), msg.size(), 0);
    }
    for (size_t i = 0; i < Users.size(); i++) {
      send(Users[i].getFd(), msg.c_str(), msg.size(), 0);
    }
  }
};

void Channel::join(Client &client, const std::string &pass, bool sudo) {
  if (!password.empty() && password != pass) {
    std::cout << "Wrong Password" << std::endl;
    sendCantJoin(client, 'k', "475");
    return;
  }
  if (inv != -1 && sudoUsers.size() + Users.size() > lim) {
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

void Channel::handleMode(Client &client,
                         const std::vector<std::string> &tokens) {
  if (!isUserInChannel(client.getNick())) {
    sendNotInChannel(client);
    return;
  } else if (!isUserSudo(client.getNick())) {
    sendNotSudo(client);
    return;
  } else {
    if (tokens[1].at(0) != '+' && tokens[1].at(0) != '-') {
      std::string msg = ":ft_irc 472 " + client.getNick() + " " +
                        tokens[1].at(0) + " :is unknown mode char to me\r\n";
      send(client.getFd(), msg.c_str(), msg.size(), 0);
      return;
    } else if (tokens[1].at(1) != 'i' && tokens[1].at(1) != 't' &&
               tokens[1].at(1) != 'k' && tokens[1].at(1) != 'o' &&
               tokens[1].at(1) != 'l') {
      std::string msg = ":ft_irc 472 " + client.getNick() + " " +
                        tokens[1].at(1) + " :is unknown mode char to me\r\n";
      send(client.getFd(), msg.c_str(), msg.size(), 0);
      return;
    } else {
      bool add = (tokens[1].at(0) == '+');
      if (tokens[1].at(1) == 'i') {
        inv = add;
      } else if (tokens[1].at(1) == 't') {
        top = add;
      } else if (tokens[1].at(1) == 'k') {
        handleModeK(client, tokens, add);
      } else if (tokens[1].at(1) == 'o') {
        handleModeO(client, tokens, add);
      } else if (tokens[1].at(1) == 'l') {
        handleModeL(client, tokens, add);
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
      for (size_t i = 0; i < sudoUsers.size(); i++) {
        sendTopic(sudoUsers[i]);
      }
      for (size_t i = 0; i < Users.size(); i++) {
        sendTopic(Users[i]);
      }
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

void Channel::handleQuit(Client &client,
                         const std::vector<std::string> &tokens) {
  if (isUserInChannel(client.getNick())) {
    std::string msg =
        ":" + client.getNick() + "!" + client.getUser() + "@localhost QUIT :";
    if (tokens.size() >= 2) {
      msg += tokens[1] + "\r\n";
    } else {
      msg += "Bye Bye!\r\n";
    }
    int i = isUserSudo(client.getNick());
    if (i == -1) {
      i = isUserInChannel(client.getNick());
      Users.erase(Users.begin() + i);
    } else {
      sudoUsers.erase(sudoUsers.begin() + i);
    }
    for (size_t i = 0; i < sudoUsers.size(); i++) {
    }
  }
  int i = isUserInvited(client.getNick());
  if (i != -1) {
    Invites.erase(Invites.begin() + i);
  }
}

Channel::~Channel(void) {};
