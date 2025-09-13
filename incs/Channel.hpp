/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afogonca <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/14 09:02:39 by afogonca          #+#    #+#             */
/*   Updated: 2025/09/12 08:48:44 by afogonca         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#ifndef CLIENT_HPP
#include "Client.hpp"
#endif
#ifndef SERVER_HPP
#include "Server.hpp"
#endif

class Channel {
public:
  Channel(const std::string &_name, Client &client,
          const std::string &Password);
  ~Channel();

  void join(Client &user, const std::string &pass, bool sudo);
  void kick(Client &client, const std::vector<std::string> &tokens);
  void invite(Client &client, Client &invited);
  void privMsg(Client &client, const std::vector<std::string> &tokens);
  void handleTopic(Client &client, const std::vector<std::string> &tokens);
  void handleMode(Client &client, const std::vector<std::string> &tokens,
                  const std::vector<Client> &clients);
  void handleQuit(Client &client, const std::vector<std::string> &tokens);
  std::string getName(void) const;
  std::string getPassword(void) const;

private:
  bool isUserInChannel(const std::string &name);
  bool isUserSudo(const std::string &name);
  bool isUserInvited(const std::string &name);
  void sendJoinMessage(Client &client);
  void sendNotSudo(Client &client);
  void sendNotInChannel(Client &client);
  void sendCantJoin(Client &client, char mode, const std::string &code);
  void sendTopic(Client &client);
  void sendClientList(Client &client);
  void sendEndNameList(Client &client);

  void handleModeK(Client &client, const std::vector<std::string> &tokens,
                   bool add);
  void handleModeO(Client &client, const std::vector<std::string> &tokens,
                   bool add);
  void handleModeL(Client &client, const std::vector<std::string> &tokens,
                   bool add);
  std::string name;
  std::string topic;
  std::string password;
  std::vector<Client> Users;
  std::vector<Client> sudoUsers;
  std::vector<Client> Invites;
  bool inv;
  bool top;
  int lim;
};
#endif
