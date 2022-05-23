#ifndef BANK_H
#define BANK_H

// INCLUDES
#include <stdio.h>
#include <map>
#include <list>
#include <vector>         
#include <iostream>
#include <fstream>
#include <pthread.h>
#include "atm.h"
#include "account.h"


// NAMESPACE
using namespace std;


// DEFINES
#define PRINT_PERIOD 500000      // 0.5 second
#define COMMISION_PERIOD 3		 // 3 seconds


// FUNCTIONS
bool  is_file_exist(const char *fileName);
void* bank_commission_routine(void* main_bank);
void* atm_routine(void* atm_in);
void* bank_status_routine(void* bank);
int main(int argc, char* argv[]);


// CLASS
class bank {
public:
	
	// This ADT enables to go over all accounts by using its ID's as the key
	map<int, account>* map_accounts_ptr;
	// This ADT enables to go over all atms
	vector<atm>* vector_all_atm_ptr;
	int num_of_atm;
	int current_balance_bank;
	int is_atm_finished;
	int number_of_readers;
	
	pthread_mutex_t mutex_log_print;
	// This mutex will be used for locking while we print to the screen
	pthread_mutex_t mutex_screen_print;
	pthread_mutex_t mutex_global_accounts;
	pthread_mutex_t mutex_bank_read;
	pthread_mutex_t mutex_bank_write;


	bank(map<int, account>* map_accounts_ptr_inst, vector<atm>* vector_all_atm_ptr_inst, int num_of_atm_inst) : \
		map_accounts_ptr(map_accounts_ptr_inst), vector_all_atm_ptr(vector_all_atm_ptr_inst), 					\
		num_of_atm(num_of_atm_inst) 
		{
			current_balance_bank = 0;
			is_atm_finished = 0;
			number_of_readers = 0;
			pthread_mutex_init(&mutex_log_print, NULL);
			pthread_mutex_init(&mutex_screen_print, NULL);
			pthread_mutex_init(&mutex_global_accounts, NULL);
			pthread_mutex_init(&mutex_bank_read, NULL);
			pthread_mutex_init(&mutex_bank_write, NULL);
	};
	
	~bank() 
	{
		pthread_mutex_destroy(&mutex_log_print);
		pthread_mutex_destroy(&mutex_screen_print);
		pthread_mutex_destroy(&mutex_global_accounts);
		pthread_mutex_destroy(&mutex_bank_read);
		pthread_mutex_destroy(&mutex_bank_write);
	};
	
	void lock_for_writers() {pthread_mutex_lock(&mutex_bank_write);}	
	void unlock_for_writers() {pthread_mutex_unlock(&mutex_bank_write);}
	
	
	void lock_for_readers() 
	{
		pthread_mutex_lock(&mutex_bank_read);
		number_of_readers += 1;
		if (number_of_readers == 1) {pthread_mutex_lock(&mutex_bank_write);}
		pthread_mutex_unlock(&mutex_bank_read);
	}
	
	
	void unlock_for_readers() 
	{
		pthread_mutex_lock(&mutex_bank_read);
		number_of_readers -= 1;
		if (!number_of_readers) {pthread_mutex_unlock(&mutex_bank_write);}
		pthread_mutex_unlock(&mutex_bank_read);
	}

	bool bank_balance_print();
	bool commission();
	bool init_print_bank_func(void* main_bank);
};

#endif
