/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afogonca <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/14 09:02:39 by afogonca          #+#    #+#             */
/*   Updated: 2025/08/14 10:01:37 by afogonca         ###   ########.fr       */
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
  void invite(Client &user, Client &invited);
  void handleTopic(Client &client, const std::vector<std::string> &tokens);
  std::string getName(void) const;
  std::string getPassword(void) const;

private:
  bool isUserInServer(const std::string &name);
  bool isUserSudo(const std::string &name);
  bool isUserInvited(const std::string &name);
  void sendJoinMessage(Client &client);
  std::string sendTopic(Client &client);
  std::string sendClientList(Client &client);
  std::string name;
  std::string topic;
  std::string password;
  std::vector<Client> Users;
  std::vector<Client> sudoUsers;
  std::vector<Client> Invites;
  bool inv;
  bool top;
  bool lim;
};
#endif
