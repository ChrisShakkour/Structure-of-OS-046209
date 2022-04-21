# Makefile for the smash program

CC = g++
CFLAGS = -std=c++11 -Wall -Werror -pedantic-errors -DNDEBUG
CCLINK = $(CC)
OBJS = smash.o commands.o signals.o jobs.o
RM = rm -f


# Creating the  executable
smash: $(OBJS)
	$(CCLINK) -o smash $(OBJS)


# Creating the object files
commands.o: commands.cpp commands.h signals.h jobs.h
smash.o: signals.h smash.cpp commands.h	jobs.h
signals.o: commands.h signals.cpp signals.h jobs.h
jobs.o: jobs.cpp jobs.h


# Cleaning old files before new make
clean:
	$(RM) $(TARGET) *.o *~ "#"* core.* smash
