#ifndef ACCOUNT_H
#define ACCOUNT_H

// Includes
//#include <string>
//#include <string.h>
//#include <map>
//#include <vector>         
//#include <iostream>
//#include <fstream>
//#include <sstream>
#include <pthread.h>
#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
//#include <ctime>


// bank account structure
// and actions class
class account 
	{
public:
	
	// account ID variables
	int account_num;
	int password;
	int balance;

	unsigned number_of_readers;
	
	// This mutex will be used for 
	// locking writers, and 
	// enabling multiple readers.
	pthread_mutex_t read_access_lock;
	
	// This mutex will be used for 
	// locking writers and readers
	pthread_mutex_t write_access_lock;
	
	// class constructor, inits mutex
	account(int account_num_inst, int password_inst, int balance_inst) : account_num(account_num_inst), password(password_inst), balance(balance_inst) 
	{
		pthread_mutex_init(&read_access_lock, NULL);
		pthread_mutex_init(&write_access_lock, NULL);
		number_of_readers = 0;
	}
	
	// class distructor, distroys mutex
	~account() 
	{
		pthread_mutex_destroy(&read_access_lock);
		pthread_mutex_destroy(&write_access_lock);
	}

	void lock_for_writers()   {pthread_mutex_lock(&write_access_lock);}
	void unlock_for_writers() {pthread_mutex_unlock(&write_access_lock);}
	
	
	void lock_for_readers() 
	{
		pthread_mutex_lock(&read_access_lock);
		number_of_readers += 1;
		if (number_of_readers == 1) {pthread_mutex_lock(&write_access_lock);}		
		pthread_mutex_unlock(&read_access_lock);
	}
	void unlock_for_readers() 
	{
		pthread_mutex_lock(&read_access_lock);
		number_of_readers -= 1;
		if (!number_of_readers) {pthread_mutex_unlock(&write_access_lock);}
		pthread_mutex_unlock(&read_access_lock);
	}
};

#endif