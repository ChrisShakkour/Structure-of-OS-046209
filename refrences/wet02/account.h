#ifndef _ACCOUNT_H
#define _ACCOUNT_H

// Includes
#include <string>
#include <string.h>
#include <map>
#include <vector>         
#include <iostream>
#include <fstream>
#include <sstream>
#include <pthread.h>
#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <ctime>

// This will represent the structure and the action of a bank account
class account {

public:
	
	int account_num;
	int password;
	int balance;
	unsigned number_of_readers;
	// This mutex will be used for locking  writers, and enable multiple readers
	pthread_mutex_t mutex_account_read;
	// This mutex will be used for locking  writers and readers
	pthread_mutex_t mutex_account_write;
	
	account(int account_num_inst, int password_inst, int balance_inst) : account_num(account_num_inst), password(password_inst), balance(balance_inst) 
	{
		pthread_mutex_init(&mutex_account_read, NULL);
		pthread_mutex_init(&mutex_account_write, NULL);
		number_of_readers = 0;
	}
	
	~account() 
	{
		pthread_mutex_destroy(&mutex_account_read);
		pthread_mutex_destroy(&mutex_account_write);
	}

	void lock_for_writers() {pthread_mutex_lock(&mutex_account_write);}

	void unlock_for_writers() {pthread_mutex_unlock(&mutex_account_write);}
	
	void lock_for_readers() 
	{
		pthread_mutex_lock(&mutex_account_read);
		number_of_readers += 1;
		if (number_of_readers == 1) {pthread_mutex_lock(&mutex_account_write);}		
		
		pthread_mutex_unlock(&mutex_account_read);
	}
	
	void unlock_for_readers() 
	{
		pthread_mutex_lock(&mutex_account_read);
		number_of_readers -= 1;
		if (!number_of_readers) {pthread_mutex_unlock(&mutex_account_write);}
		pthread_mutex_unlock(&mutex_account_read);
	}

};

#endif