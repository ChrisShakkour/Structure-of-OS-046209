# Makefile for the smash program

CC = g++
CFLAGS = -std=c++11 -Wall -Werror -pedantic-errors -DNDEBUG
CCLINK = $(CC)
OBJS = smash.o commands.o signals.o
RM = rm -f


# Creating the  executable
smash: $(OBJS)
	$(CCLINK) -o smash $(OBJS)


# Creating the object files
commands.o: commands.cpp commands.h
smash.o: signals.h smash.cpp commands.h	
signals.o: commands.h signals.cpp signals.h	


# Cleaning old files before new make
clean:
	$(RM) $(TARGET) *.o *~ "#"* core.* smash

