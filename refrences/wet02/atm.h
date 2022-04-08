﻿#ifndef _ATM_H
#define _ATM_H

#include "account.h"

using namespace std;

// This will represent the structure and the action of an atm

class atm {

public:
	int atm_num;
	// This ADT enables to go over all accounts by using its ID's as the key
	map<int, account>* map_accounts_ptr;
	// This string will hold the content of the atm's actions inserted
	string atm_txt_file;
	// This mutex will be used for locking while the log file is being updated
	pthread_mutex_t* mutex_log_print_ptr;
	// This mutex will be used for locking while there is a chance in the map account
	pthread_mutex_t* mutex_global_accounts_ptr;

	atm(int atm_num_inst, map<int, account>* map_accounts_ptr_inst, string atm_txt_file_inst, \
			pthread_mutex_t* mutex_log_print_ptr_inst, pthread_mutex_t* mutex_global_accounts_ptr_inst) :
			atm_num(atm_num_inst), map_accounts_ptr(map_accounts_ptr_inst), atm_txt_file(atm_txt_file_inst)
	{
		mutex_log_print_ptr = mutex_log_print_ptr_inst;
		mutex_global_accounts_ptr = mutex_global_accounts_ptr_inst;
	}
	
	~atm() 
	{
		pthread_mutex_destroy(mutex_log_print_ptr);
		pthread_mutex_destroy(mutex_global_accounts_ptr);
	}
	
	// ATM methods
	void wrong_password_check_and_print(int id, int account_password, int inserted_password);
	void erase_account_by_id(int local_account_num);
	account* find_account(int inserted_acc_num, int* is_found);
	void error_print(char letter, int inserted_acc_num, int target_account, int is_found);
	bool init_atm_func(void* atm_inst);
	void all_functions_caller();
	void O_function(int inserted_acc_num, int inserted_password, int inserted_balance);
	void D_function(int inserted_password, int inserted_amount, account* account_ptr);
	void W_function(int inserted_password, int inserted_amount, account* account_ptr);
	void B_function(int inserted_password, account* account_ptr);
	void Q_function(int inserted_password, account* account_ptr);
	void T_function(int inserted_password_src, int inserted_amount, account* account_src_ptr, account* account_dest_ptr);
};

#endif