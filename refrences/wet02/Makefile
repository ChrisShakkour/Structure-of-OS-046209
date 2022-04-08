# Makefile for the bank program
CC = g++
CFLAGS = -g -Wall
CCLINK = $(CC)
LIBS = -lpthread
OBJS = Bank.o bank_op.o atm.o #account.o 
RM = rm -f
# Creating the  executable
Bank: $(OBJS)
	$(CCLINK) -o Bank $(OBJS) $(LIBS)
# Creating the object files
Bank.o: Bank.cpp bank_op.h atm.h account.h
# account.o : account.h
atm.o : atm.cpp atm.h account.h
bank_op.o : bank_op.cpp bank_op.h account.h atm.h


# Cleaning old files before new make
clean:
	$(RM) $(TARGET) *.o *~ "#"* core.* Bank
