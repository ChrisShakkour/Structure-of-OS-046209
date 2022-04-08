#include "bank_op.h"

extern ofstream output_log;

// This function charges a random commission from all accounts
bool bank::commission()
{
	pthread_mutex_lock(&mutex_global_accounts);
	int total_commission = 0;
	map<int, account>::iterator iter;
	srand((unsigned) time(0));
	double random_num = (rand() % 2) + 2;
	map<int, account>::iterator begin_iter = map_accounts_ptr->begin();
	map<int, account>::iterator last_iter = map_accounts_ptr->end();

	for (iter = begin_iter; iter != last_iter; ++iter)
	{
		iter->second.lock_for_writers();
		int current_balance = iter->second.balance;
		int current_commission = (int)((random_num / 100) * current_balance);
		iter->second.balance = current_balance - current_commission;
		int current_account = iter->second.account_num;
		pthread_mutex_lock(&mutex_log_print);
		output_log << "Bank: commissions of " << random_num << " % were charged, the bank gained " << current_commission << " $ from account " << current_account << "\n";
		pthread_mutex_unlock(&mutex_log_print);
		total_commission += current_commission;
		iter->second.unlock_for_writers();
	}
	pthread_mutex_unlock(&mutex_global_accounts);
	current_balance_bank += total_commission;
	if (!is_atm_finished) {return false;}
	else {return true;}
}

// This function prints the bank's balance
bool bank::bank_balance_print()
{
	pthread_mutex_lock(&mutex_global_accounts);
	printf("\033[2J");
	printf("\033[1;1H");
	cout << "Current Bank Status" << endl;
	map<int, account>::iterator iter;
	map<int, account>::iterator iter_begin = map_accounts_ptr->begin();
	map<int, account>::iterator iter_last = map_accounts_ptr->end();
	for (iter = iter_begin; iter != iter_last; ++iter)
	{
		iter->second.lock_for_readers();
		int current_account_num = iter->second.account_num;
		int current_account_balance = iter->second.balance;
		int current_account_password = iter->second.password;
		iter->second.unlock_for_readers();
		cout << "Account " << current_account_num << ": Balance - " << current_account_balance << " $ , Account Password - " << current_account_password << endl;
	}
	cout << "." << endl;
	cout << "." << endl;
	cout << "The Bank has " << current_balance_bank << " $" << endl;
	pthread_mutex_unlock(&mutex_global_accounts);
	if (!is_atm_finished) {return false;}
	else {return true;}
}

bool bank::init_print_bank_func(void* main_bank)
{
	if (main_bank == NULL)
	{
		return false;
	}
	return true;
}
