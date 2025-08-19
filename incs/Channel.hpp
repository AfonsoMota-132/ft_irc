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

# ifndef CLIENT_HPP
#  include "Client.hpp"
# endif
# ifndef SERVER_HPP
#  include "Server.hpp"
# endif

class Channel {
public:
  Channel(const std::string &_name, const std::string &_topic);
  ~Channel();

  void	join(Client &user, bool sudo);
private:
  std::string name;
  std::string topic;
  std::vector<Client> Users;
  std::vector<Client> sudoUsers;
  bool inv;
  bool top;
  bool kick;
  bool lim;
};
#endif
