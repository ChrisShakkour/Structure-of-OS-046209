#include "bank_op.h"

using namespace std;
#define PRINT_ROUTINE_DELAY 500000
#define COMMISSION_ROUTINE_DELAY 3

ofstream output_log;
int num_of_atm;
map<int, account> map_of_accounts;
vector<atm> vector_of_atm;

// This function is being called by the bank print thread
void* print_routine(void* main_bank)
{
	bank* cast_bank = (bank*)main_bank;
	bool bank_is_done;
	if(cast_bank->init_print_bank_func(main_bank))
	{
		while (true)
		{
			bank_is_done = cast_bank->bank_balance_print();
			usleep(PRINT_ROUTINE_DELAY);
			if (bank_is_done)
				break;
		}
		pthread_exit(NULL);
		delete(cast_bank);
	}
	pthread_exit(NULL);
	delete(cast_bank);
}

// This function is being called by the bank commission thread
void* commission_routine(void* main_bank)
{
	bool bank_is_done;
	bank* cast_bank = (bank*)main_bank;
	if(cast_bank->init_print_bank_func(main_bank))
	{
		while (true)
		{
			bank_is_done = cast_bank->commission();
			sleep(COMMISSION_ROUTINE_DELAY);
			if (bank_is_done)
				break;
		}
		pthread_exit(NULL);
		delete(cast_bank);
	}
	pthread_exit(NULL);
	delete(cast_bank);
}

// This function is being called by the atm threads
void* atm_routine(void* atm_inst)
{
	atm* cast_atm = (atm*)atm_inst;
	if (cast_atm->init_atm_func(atm_inst))
	{
		cast_atm->all_functions_caller();
		delete cast_atm;
		pthread_exit(NULL);
	}
	delete cast_atm;
	pthread_exit(NULL);
}


int main(int argc, char* argv[])
{
	// Checks the correction of inserted arguments
	if (argc <= 2)
	{
		cout << "illegal arguments" << endl;
		exit(1);
	}
	int num_of_atm_txt = argc - 2;
	int num_of_atm = atoi(argv[1]);
	if ((num_of_atm < 1) || (num_of_atm != num_of_atm_txt))
	{
		cout << "illegal arguments" << endl;
		exit(1);
	}

	// Creates pointers for the needed threads
	pthread_t* atm_thread_ptr = new pthread_t[num_of_atm];
	pthread_t status_print_thread;
	pthread_t commision_charge_thread;

	bank* main_bank = new bank(&map_of_accounts, &vector_of_atm, num_of_atm);
	atm* create_atm;
	output_log.open("log.txt", ios::out);

	// Create thread for each atm
	int i = 0;
	while (i < num_of_atm)
	{
		int account_num = i;
		account_num++;

		int index_atm_txt = i;
		index_atm_txt += 2;
		
		string atm_txt_file_name;
		atm_txt_file_name = argv[index_atm_txt];
		create_atm = new atm(account_num, &map_of_accounts, atm_txt_file_name, &main_bank->mutex_log_print, &main_bank->mutex_global_accounts);
		vector_of_atm.push_back(*create_atm);
		if (pthread_create(&atm_thread_ptr[i], NULL, atm_routine, (void*)create_atm))
		{
			perror("Error: thread fail");
		}
		i++;
	}

	// Create thread for bank commission
	if (pthread_create(&commision_charge_thread, NULL, commission_routine, (void*)main_bank))
	{
		perror("Error: thread fail");
	}
	
	// Create thread for bank print
	if (pthread_create(&status_print_thread, NULL, print_routine, (void*)main_bank))
	{
		perror("Error: thread fail");
	}
	
	// Wait till all threads are done
	i = 0;
	while (i < num_of_atm)
	{
		if (pthread_join(atm_thread_ptr[i], NULL))
		{
			perror("Error: thread join fail");
		}
		i++;
	}

	main_bank->is_atm_finished = 1;

	if (pthread_join(commision_charge_thread, NULL))
	{
		perror("Error: thread join fail");;
	}
	if (pthread_join(status_print_thread, NULL))
	{
		perror("Error: thread join fail");
	}
	
	// Clear memory
	vector_of_atm.clear();
	output_log.close();
	delete[] atm_thread_ptr;
	delete main_bank; 

	return 0;
}