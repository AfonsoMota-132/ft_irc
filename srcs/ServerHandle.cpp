/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerHandle.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afogonca <afogonca@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/11 10:09:07 by afogonca          #+#    #+#             */
/*   Updated: 2025/09/11 10:16:36 by afogonca         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/Server.hpp"

void Server::handleInvite(const std::vector<std::string> &tokens,
                          Client &client) {

  if (tokens.size() < 3) {
    std::string msg = ":ft_irc 461 " + client.getNick() +
                      " INVITE :Not enough parameters\r\n";
    send(client.getFd(), msg.c_str(), msg.size(), 0);
  } else {
    int channelExists = -1;
    int userExists = -1;

    if (tokens[2].empty() || tokens[2].at(0) != '#') {
      std::string msg = ":ft_irc 403 " + client.getNick() + " " + tokens[2] +
                        " :No such channel\r\n";
      send(client.getFd(), msg.c_str(), msg.size(), 0);
      return;
    }
    for (size_t i = 0; i < Clients.size(); i++) {
      if (ft_strtoupper(tokens[1]) == ft_strtoupper(Clients[i].getNick())) {
        userExists = i;
        break;
      }
    }
    for (size_t i = 0; i < Channels.size(); i++) {
      if (ft_strtoupper(tokens[2].substr(1)) ==
          ft_strtoupper(Channels[i].getName())) {
        channelExists = i;
        break;
      }
    }
    if (userExists == -1) {
      std::string msg = ":ft_irc 401 " + client.getNick() + " " + tokens[1] +
                        " :No such nick/channel\r\n";
      send(client.getFd(), msg.c_str(), msg.size(), 0);
      return;
    } else if (channelExists == -1) {
      std::string msg = ":ft_irc 403 " + client.getNick() + " " + tokens[2] +
                        " :No such channel\r\n";
      send(client.getFd(), msg.c_str(), msg.size(), 0);
      return;
    } else {
      Channels[channelExists].invite(client, Clients[userExists]);
    }
  }
}

void Server::handleKick(const std::vector<std::string> &tokens,
                        Client &client) {
  if (tokens.size() < 3) {
    std::string msg =
        ":ft_irc 461 " + client.getNick() + " KICK :Not enough parameters\r\n";
    send(client.getFd(), msg.c_str(), msg.size(), 0);
    return;
  } else if (tokens[1].empty() || tokens[1].at(0) != '#') {
    std::string msg = ":ft_irc 403 " + client.getNick() + " " + tokens[1] +
                      " :No such channel\r\n";
    send(client.getFd(), msg.c_str(), msg.size(), 0);
    return;
  } else {
    int channelExists = -1;
    for (size_t i = 0; i < Channels.size(); i++) {
      if (ft_strtoupper(tokens[1].substr(1)) ==
          ft_strtoupper(Channels[i].getName())) {
        channelExists = i;
        break;
      }
    }
    if (channelExists == -1) {
      std::string msg = ":ft_irc 403 " + client.getNick() + " " + tokens[1] +
                        " :No such channel\r\n";
      send(client.getFd(), msg.c_str(), msg.size(), 0);
      return;
    } else {
      Channels[channelExists].kick(client, tokens);
    }
  }
}

void Server::handleTopic(const std::vector<std::string> &tokens,
                         Client &client) {
  if (tokens.size() < 2) {
    std::string msg =
        ":ft_irc 461 " + client.getNick() + " JOIN :Not enough parameters\r\n";
    send(client.getFd(), msg.c_str(), msg.size(), 0);
    return;
  } else {
    int serverExists = -1;
    if (tokens[1].at(0) != '#') {
      std::string msg = ":ft_irc 403 " + client.getNick() + " " + tokens[1] +
                        " :No such channel\r\n";
      send(client.getFd(), msg.c_str(), msg.size(), 0);
      return;
    }
    for (size_t i = 0; i < Channels.size(); i++) {
      if (ft_strtoupper(tokens[1].substr(1)) == Channels[i].getName()) {
        serverExists = i;
      }
    }
    if (serverExists == -1) {
      std::string msg = ":ft_irc 403 " + client.getNick() + " " + tokens[1] +
                        " :No such channel\r\n";
      send(client.getFd(), msg.c_str(), msg.size(), 0);
      return;
    } else {
	  Channels[serverExists].handleTopic(client, tokens);
    }
  }
}

void Server::handleJoin(const std::vector<std::string> &tokens,
                        Client &client) {
  bool channelExists = false;
  if (tokens.size() <= 1) {
    std::string msg =
        ":ft_irc 461 " + client.getNick() + " JOIN :Not enough parameters\r\n";
    send(client.getFd(), msg.c_str(), msg.size(), 0);
    return;
  } else if (tokens[1].empty() || tokens[1].at(0) != '#') {
    std::string msg = ":ft_irc 403 " + client.getNick() + " " + tokens[1] +
                      " :No such channel\r\n";
    send(client.getFd(), msg.c_str(), msg.size(), 0);
    return;
  }
  for (size_t i = 0; i < Channels.size(); i++) {
    if (ft_strtoupper(Channels[i].getName()) ==
        ft_strtoupper(tokens[1].substr(1))) {
      if (tokens.size() >= 3) {
        Channels[i].join(client, tokens[2], 0);
      } else {
        Channels[i].join(client, "", 0);
      }
      channelExists = true;
    }
  }
  if (!channelExists) {
    std::string password = "";
    if (tokens.size() >= 3)
      password = tokens[2];
    Channel newChannel(tokens[1].substr(1), client, password);
    std::cout << "Creating new channel!" << std::endl;
    Channels.push_back(newChannel);
  }
  (void)client;
}
