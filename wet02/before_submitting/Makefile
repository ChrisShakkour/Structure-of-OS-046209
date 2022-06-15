# Makefile for the bank program
CC = g++
CFLAGS = -g -Wall -std=c++11 -Werror -pedantic-errors -DNDEBUG
CCLINK = $(CC)
LIBS = -lpthread
OBJS = bank.o atm.o
RM = rm -f

# perform linkage, creates Bank executable
Bank: $(OBJS)
	$(CCLINK) -o Bank $(OBJS) $(LIBS)

# bank object file
bank.o: bank.cpp atm.h account.h

# atm object file
atm.o : atm.cpp atm.h account.h

# bank operations object file
# bank_op.o : bank_op.cpp bank_op.h account.h atm.h


# Cleaning old files before new make
clean:
	$(RM) $(TARGET) *.o *~ "#"* core.* Bank
