#OBJS specifies which files to compile as part of the project
OBJS = pong.c 

#CC specifies which compiler we're using
CC = gcc

#COMPILER_FLAGS specifies the additional compilation options we're using
# -w suppresses all warnings
COMPILER_FLAGS = -Wall -W -g -pedantic 

#LINKER_FLAGS specifies the libraries we're linking against
LINKER_FLAGS = -lncurses

#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = pong 

#This is the target that compiles our executable
pong: $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)