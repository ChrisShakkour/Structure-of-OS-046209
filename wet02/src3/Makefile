# Makefile for the Bank program
CXX = g++
CXXFLAGS = -g -std=c++11 -Wall -Werror -pedantic -errors -DNDEBUG 
CCLINK = $(CXX)
LIBS = -pthread
OBJS = Bank.o atm.o account.o
RM = rm -f
# Creating the  executable
Bank: $(OBJS)
	$(CCLINK) -o Bank $(OBJS) $(LIBS)
	
Bank.o: Bank.cpp atm.h account.h
	$(CXX) $(CXXFLAGS) -c Bank.cpp
	
atm.o: atm.cpp account.h atm.h
	$(CXX) $(CXXFLAGS) -c atm.cpp
	
account.o: account.cpp atm.h account.h
	$(CXX) $(CXXFLAGS) -c account.cpp
	
# Cleaning old files before new make
clean:
	rm -f  Bank *.o *~ "#"* core.*

