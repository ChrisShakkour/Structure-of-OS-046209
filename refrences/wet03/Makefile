# Makefile for the program
CC = gcc
CFLAGS = -g -Wall -Werror -pedantic-errors -DNDEBUG 
CCLINK = $(CC)
OBJS = ttftps.o
RM = rm -f
# Creating the  executable
ttftps: $(OBJS)
	$(CCLINK) -o ttftps $(OBJS)
# Creating the object files

ttftps.o: ttftps.c 

# Cleaning old files before new make
clean:
	$(RM) $(TARGET) *.o *~ "#"* core.* ttftps

