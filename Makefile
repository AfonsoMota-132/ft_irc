# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: afogonca <afogonca@student.42porto.com>    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/05/27 09:06:24 by afogonca          #+#    #+#              #
#    Updated: 2025/09/12 09:46:06 by afogonca         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = irc
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -g
RM = rm -f
VAL = valgrind  --leak-check=full --show-leak-kinds=all --track-origins=yes
SRCS = srcs/main.cpp				\
	   srcs/Client.cpp				\
	   srcs/Server.cpp				\
	   srcs/Channel.cpp				\
	   srcs/ServerHandle.cpp		\
	   srcs/ServerCheck.cpp			\
	   srcs/ChannelHelpers.cpp		\
	   srcs/ChannelHandleMode.cpp

OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

valgrind: all
	$(VAL) ./$(NAME) 6667 abc 

re: fclean all

.PHONY: all clean fclean re valgrind
