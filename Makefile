NAME=4l3a_server
SRC_DIR=src/
INC_DIR=inc/
OBJ_DIR=obj/

SRCS=4l3a_server.cpp


SRCS 	:=	$(addprefix $(SRC_DIR), $(SRCS))
OBJS	=	$(SRCS:$(SRC_DIR)%.c=$(OBJ_DIR)%.o)
INCS	=	-I$(INC_DIR)			\
CFLAGS  =	-Wall -Wextra -Werror -g

CC=c++
all: $(NAME)

$(OBJ_DIR)%.o	: $(SRC_DIR)%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

$(NAME)			: $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ -o $(NAME)

clean			:
	@echo "Cleaning object..."
	@rm -rf $(OBJ_DIR)

fclean			: clean
	@echo "Cleaning everything..."
	@rm -f $(NAME)

.PHONY	: all clean fclean re print_vars 
