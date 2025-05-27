/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: afogonca <afogonca@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/27 09:34:48 by afogonca          #+#    #+#             */
/*   Updated: 2025/05/27 09:47:27 by afogonca         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef Client_HPP
# define Client_HPP

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdio>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>

class	Client {
	private:
		int					fd;
		struct sockaddr_in	addr;
		socklen_t			addrLen;
	public:
		Client(void);
		~Client(void);

		int	acceptConnection(int server_fd);
		int	receiveMessage(char	*buffer);
		void	closeFd(void);
		int		getFd(void);
};

#endif
