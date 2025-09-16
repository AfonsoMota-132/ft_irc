/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerHandle.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afogonca <afogonca@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/11 10:09:07 by afogonca          #+#    #+#             */
/*   Updated: 2025/09/12 09:58:16 by afogonca         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/Server.hpp"

void Server::handleInvite(const std::vector<std::string> &tokens,
                          Client &client) {

  if (tokens.size() < 3) {
    sendNoParams(client, "INVITE");
    return;
  } else {
    int channelI = channelExists(tokens[2]);
    int userI = userExists(tokens[1]);
    if (userI == -1) {
      std::string msg = ":ft_irc 401 " + client.getNick() + " " + tokens[1] +
                        " :No such nick/channel\r\n";
      send(client.getFd(), msg.c_str(), msg.size(), 0);
      return;
    } else if (channelI == -1) {
      sendNoChannel(client, tokens[2]);
      return;
    } else {
      Channels[channelI].invite(client, Clients[userI]);
    }
  }
}

void Server::handleKick(const std::vector<std::string> &tokens,
                        Client &client) {
  if (tokens.size() < 3) {
    sendNoParams(client, "KICK");
    return;
  } else {
    int channelI = channelExists(tokens[1]);
    if (channelI == -1) {
      sendNoChannel(client, tokens[1]);
      return;
    } else {
      Channels[channelI].kick(client, tokens);
    }
  }
}

void Server::handleTopic(const std::vector<std::string> &tokens,
                         Client &client) {
  if (tokens.size() < 2) {
    sendNoParams(client, "TOPIC");
    return;
  } else {
    int serverI = channelExists(tokens[1]);
    if (serverI == -1) {
      sendNoChannel(client, tokens[1]);
      return;
    } else {
      Channels[serverI].handleTopic(client, tokens);
    }
  }
}

void Server::handleJoin(const std::vector<std::string> &tokens,
                        Client &client) {
  bool channelExists = false;
  if (tokens.size() <= 1) {
    sendNoParams(client, "JOIN");
    return;
  } else if (tokens[1].empty() || tokens[1].at(0) != '#') {
    sendNoChannel(client, tokens[1]);
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
    Channels.push_back(newChannel);
  }
  (void)client;
}

void Server::handleMode(const std::vector<std::string> &tokens,
                        Client &client) {
  if (tokens.size() < 2) {
    sendNoParams(client, "MODE");
    return;
  } else {
    int channelI = channelExists(tokens[1]);
    if (channelI == -1) {
      sendNoChannel(client, tokens[1]);
      return;
    } else {
      if (tokens.size() >= 3 && tokens[2].size() > 2) {
        std::string msg = ":ft_irc 904 " + client.getNick() +
                          " MODE :Too many mode changes in one call!\r\n";
        send(client.getFd(), msg.c_str(), msg.size(), 0);
      } else if (tokens.size() >= 3 && tokens[2].size() < 2) {
        std::string msg = ":ft_irc 905 " + client.getNick() +
                          " MODE :Mode Changes need to be <sign><mode>!\r\n";
        send(client.getFd(), msg.c_str(), msg.size(), 0);
      } else {
        Channels[channelI].handleMode(client, tokens);
      }
    }
  }
}

void Server::handleQuit(const std::vector<std::string> &tokens,
                        Client &client) {
  for (size_t i = 0; i < Channels.size(); i++) {
    Channels[i].handleQuit(client, tokens);
  }
  for (size_t i = 0; i < Channels.size(); i++) {
    if (Channels[i].ClientCount() == 0) {
      Channels.erase(Channels.begin() + i);
      i = 0;
    }
  }
}

void Server::handlePart(const std::vector<std::string> &tokens,
                        Client &client) {
  if (tokens.size() >= 2) {
    int i = channelExists(tokens[1]);
    if (i != -1) {
      std::string msg = ":" + client.getNick() + "!" + client.getUser() +
                            "@localhost" + " PART #" + Channels[i].getName();
      if (tokens.size() >= 3 && tokens[2].empty()) {
        msg += " :" + tokens[2];
      } else {
        msg += " : Bye Bye";
      }
      msg += "\r\n";
	  send(client.getFd(), msg.c_str(), msg.size(), 0);
      std::vector<std::string> tmp(tokens);
      tmp.erase(tmp.begin() + 1);
      Channels[i].handleQuit(client, tmp);
      if (Channels[i].ClientCount() == 0) {
        Channels.erase(Channels.begin() + i);
      }
    } else {
      sendNoChannel(client, tokens[1]);
    }
  } else {
    sendNoParams(client, "PART");
  }
}
