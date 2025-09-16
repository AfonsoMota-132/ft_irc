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
      sendModeChange("+k", tokens[3], client);
    } else {
      std::string msg = "ft_irc 461 " + client.getNick() + " MODE" +
                        " :Not enough parameters\r\n";
      send(client.getFd(), msg.c_str(), msg.size(), 0);
      return;
    };
  } else {
    password = "";
    sendModeChange("-k", "", client);
  }
};

void Channel::handleModeO(Client &client,
                          const std::vector<std::string> &tokens, bool add) {
  bool sendMessage = false;
  if (tokens.size() >= 4) {
    if (add) {
      if (!isUserSudo(tokens[3]) && isUserInChannel(tokens[3])) {
        for (size_t i = 0; i < Users.size(); i++) {
          if (ft_strtoupper(tokens[3]) == ft_strtoupper(Users[i].getNick())) {
            sudoUsers.push_back(Users[i]);
            Users.erase(Users.begin() + i);
            sendMessage = true;
            sendModeChange("+o", tokens[3], client);
            break;
          }
        }
      } else if (!isUserInChannel(tokens[3])) {
        std::string msg = ":ft_irc 441 " + client.getNick() + " " + tokens[3] +
                          " #" + name + " :They aren't on that channel";
        send(client.getFd(), msg.c_str(), msg.size(), 0);
        return;
      }
    } else {
      if (isUserSudo(tokens[3]) && isUserInChannel(tokens[3])) {
        for (size_t i = 0; i < sudoUsers.size(); i++) {
          if (ft_strtoupper(tokens[3]) ==
              ft_strtoupper(sudoUsers[i].getNick())) {
            Users.push_back(sudoUsers[i]);
            sudoUsers.erase(sudoUsers.begin() + i);
            sendMessage = true;
            sendModeChange("-o", tokens[3], client);
            break;
          }
        }
      } else if (!isUserInChannel(tokens[3])) {
        std::string msg = ":ft_irc 441 " + client.getNick() + " " + tokens[3] +
                          " #" + name + " :They aren't on that channel";
        send(client.getFd(), msg.c_str(), msg.size(), 0);
        return;
      }
    }
  } else {
    std::string msg = "ft_irc 461 " + client.getNick() + " MODE" +
                      " :Not enough parameters\r\n";
    send(client.getFd(), msg.c_str(), msg.size(), 0);
    return;
  }
  if (sendMessage) {
    for (size_t i = 0; i < sudoUsers.size(); i++) {
      sendClientList(sudoUsers[i]);
      sendEndNameList(sudoUsers[i]);
    }
    for (size_t i = 0; i < Users.size(); i++) {
      sendClientList(Users[i]);
      sendEndNameList(Users[i]);
    }
  }
};

void Channel::handleModeL(Client &client,
                          const std::vector<std::string> &tokens, bool add) {
  if (add) {
    if (tokens.size() >= 4) {
      if (tokens[3].empty() || tokens[3].size() > 10) {
        std::string msg = ":ft_irc 696 " + client.getNick() + " #" + name +
                          " l :Invalid limit parameters\r\n";
        send(client.getFd(), msg.c_str(), msg.size(), 0);
        return;
      }
      for (size_t i = 0; i < tokens[3].size(); i++) {
        if (!std::isdigit(static_cast<unsigned char>(tokens[3][i]))) {
          std::string msg = ":ft_irc 696 " + client.getNick() + " #" + name +
                            " l :Invalid limit parameters\r\n";
          send(client.getFd(), msg.c_str(), msg.size(), 0);
          return;
        }
      }
      std::stringstream ss(tokens[3]);
      long long limit;
      ss >> limit;
      if (!ss.eof() || ss.fail() || limit <= 0 || limit >= INT_MAX) {
        std::string msg = ":ft_irc 696 " + client.getNick() + " #" + name +
                          " l :Invalid limit parameters\r\n";
        send(client.getFd(), msg.c_str(), msg.size(), 0);
        return;
      } else {
        lim = static_cast<int>(limit);
        sendModeChange("+l", tokens[3], client);
      }
    } else {
      std::string msg = "ft_irc 461 " + client.getNick() + " MODE" +
                        " :Not enough parameters\r\n";
      send(client.getFd(), msg.c_str(), msg.size(), 0);
    }
  } else {
    lim = 0;
    sendModeChange("-l", "", client);
  }
};
