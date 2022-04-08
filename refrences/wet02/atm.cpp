#include "atm.h"
#define ATM_SLEEP 100000

extern ofstream output_log;

// Helper functions

// This function checks if a wrong password was inserted
void atm::wrong_password_check_and_print(int id, int account_password, int inserted_password)
{
	if (inserted_password != account_password) 
	{
		pthread_mutex_lock(mutex_log_print_ptr);
		output_log << "Error " << atm_num << ": Your transaction failed - password for account id " << id << " is incorrect" << endl;
		pthread_mutex_unlock(mutex_log_print_ptr);
	}
}

// This function delete an account
void atm::erase_account_by_id(int local_account_num)
{
	map<int, account>::iterator iter;
	iter = map_accounts_ptr->find(local_account_num);
	if (iter != map_accounts_ptr->end()) 
	{
		map_accounts_ptr->erase(iter);
	}
}

// This function returns an account pointer by checking if it exists
account* atm::find_account(int inserted_acc_num, int* exist_account)
{
	pthread_mutex_lock(mutex_global_accounts_ptr);
	account* account_inst;
	map<int, account>::iterator iter;
	iter = map_accounts_ptr->find(inserted_acc_num);
	if (iter != map_accounts_ptr->end())
	{
		iter->second.lock_for_readers();
		account_inst = &iter->second;
		*exist_account = 1;
		iter->second.unlock_for_readers();
	}
	pthread_mutex_unlock(mutex_global_accounts_ptr);
	return account_inst;
}

// This function prints error per operation
void atm::error_print(char letter, int inserted_acc_num, int target_account, int exist_account)
{
	pthread_mutex_lock(mutex_log_print_ptr);
	if (letter == 'O')
	{
		output_log << "Error " << atm_num << ": Your transaction failed - account with the same id exists" << endl;
	}
	else if  (letter == 'T')
	{
		if (exist_account)
		{
			output_log << "Error " << atm_num << ": Your transaction failed - account id " << target_account << " does not exist" << endl;
		}
		else
		{
			output_log << "Error " << atm_num << ": Your transaction failed - account id " << inserted_acc_num << " does not exist" << endl;
		}
	}
	else
	{
		output_log << "Error " << atm_num << ": Your transaction failed - account id " << inserted_acc_num << " does not exist" << endl;
	}
	pthread_mutex_unlock(mutex_log_print_ptr);
}


// This function opens a new account
void atm::O_function(int inserted_acc_num, int inserted_password, int inserted_balance) 
{
	sleep(1);
	pthread_mutex_lock(mutex_global_accounts_ptr);
	account account_inst = account(inserted_acc_num, inserted_password, inserted_balance);
	map_accounts_ptr->insert(pair<int, account>(inserted_acc_num, account_inst));
	pthread_mutex_unlock(mutex_global_accounts_ptr);
	
	pthread_mutex_lock(mutex_log_print_ptr);
	output_log << atm_num << ": New account id is " << inserted_acc_num << " with password " << inserted_password << " and initial balance " << inserted_balance << endl;
	pthread_mutex_unlock(mutex_log_print_ptr);
}

// This function deposits a given amount to a given account by ID
void atm::D_function(int inserted_password, int inserted_amount, account* account_inst) 
{
	account_inst->lock_for_writers();
	sleep(1);
	int local_account_num = account_inst->account_num;
	int local_account_password = account_inst->password;
	wrong_password_check_and_print(local_account_num, local_account_password, inserted_password);
	if (inserted_password == local_account_password) 
	{
		account_inst->balance += inserted_amount;
		int local_account_balance = account_inst->balance;
		pthread_mutex_lock(mutex_log_print_ptr);
		output_log << atm_num << ": Account " << local_account_num << " new balance is " << local_account_balance << " after " << inserted_amount << " $ was deposited" << endl;
		pthread_mutex_unlock(mutex_log_print_ptr);
	}
	account_inst->unlock_for_writers();
}

// This function withdraws a given amount from a given account by ID
void atm::W_function(int inserted_password, int inserted_amount, account* account_inst) 
{
	account_inst->lock_for_writers();
	sleep(1);
	int local_account_num = account_inst->account_num;
	int local_account_password = account_inst->password;
	int local_account_balance = account_inst->balance;

	wrong_password_check_and_print(local_account_num, local_account_password, inserted_password);
	if (inserted_password == local_account_password) 
	{
		if (local_account_balance < inserted_amount)
		{
			pthread_mutex_lock(mutex_log_print_ptr);
			output_log << "Error " << atm_num << ": Your transcation failed - account id " << local_account_num << " balance is lower than " << inserted_amount << endl;
			pthread_mutex_unlock(mutex_log_print_ptr);
		}
		else
		{
			account_inst->balance -= inserted_amount;
			local_account_balance = account_inst->balance;
			pthread_mutex_lock(mutex_log_print_ptr);
			output_log << atm_num << ": Account " << local_account_num << " new balance is " << local_account_balance << " after " << inserted_amount << " $ was withdrew" << endl;
			pthread_mutex_unlock(mutex_log_print_ptr);
		}
	}
	account_inst->unlock_for_writers();
}

// This function prints the balance of a given account by ID
void atm::B_function(int inserted_password, account* account_inst) 
{
	account_inst->lock_for_readers();
	sleep(1);
	int local_account_num = account_inst->account_num;
	int local_account_password = account_inst->password;
	int local_account_balance = account_inst->balance;
		
	wrong_password_check_and_print(local_account_num, local_account_password, inserted_password);
	if (inserted_password == local_account_password) {
		pthread_mutex_lock(mutex_log_print_ptr);
		output_log << atm_num << ": Account " << local_account_num << " balance is " << local_account_balance << endl;
		pthread_mutex_unlock(mutex_log_print_ptr);
	}
	account_inst->unlock_for_readers();

}

// This function removes account by ID
void atm::Q_function(int inserted_password, account* account_inst) 
{
	account_inst->lock_for_readers();
	sleep(1);
	int local_account_num = account_inst->account_num;
	int local_account_password = account_inst->password;
	
	wrong_password_check_and_print(local_account_num, local_account_password, inserted_password);
	account_inst->unlock_for_readers();

	if (inserted_password == local_account_password) 
	{
		pthread_mutex_lock(mutex_global_accounts_ptr);
		int local_account_balance = account_inst->balance;
		erase_account_by_id(local_account_num);
		pthread_mutex_lock(mutex_log_print_ptr);
		output_log << atm_num << ": Account " << local_account_num << " is now closed. Balance was " << local_account_balance << endl;
		pthread_mutex_unlock(mutex_log_print_ptr);
		pthread_mutex_unlock(mutex_global_accounts_ptr);
	}
}

// This function transfers a given amount from a given account to a given account
void atm::T_function(int inserted_src_password, int inserted_amount, account* account_src_inst, account* account_dst_inst)
{
	account_src_inst->lock_for_writers();
	account_dst_inst->lock_for_writers();	
	sleep(1);
	int local_account_src_num = account_src_inst->account_num;
	int local_account_src_password = account_src_inst->password;
	int local_account_src_balance = account_src_inst->balance;
	int local_account_dst_num = account_dst_inst->account_num;

	wrong_password_check_and_print(local_account_src_num, local_account_src_password, inserted_src_password);
	
	if (local_account_src_password == inserted_src_password) 
	{
		bool self_trans = false;
		if (local_account_dst_num == local_account_src_num)
		{
			self_trans = true;
			pthread_mutex_lock(mutex_log_print_ptr);
			output_log << atm_num << ": Transfer " << inserted_amount << " from account " << local_account_src_num
				<< " to account " << local_account_dst_num << " new account balance is " <<
				local_account_src_balance << " new target account balance is " << local_account_src_balance << endl;
			pthread_mutex_unlock(mutex_log_print_ptr);
		}
		
		if ((local_account_src_balance >= inserted_amount) && !self_trans)
		{
			account_src_inst->balance -= inserted_amount;
			account_dst_inst->balance += inserted_amount;

			local_account_src_balance = account_src_inst->balance;
			int local_account_dst_balance = account_dst_inst->balance;

			pthread_mutex_lock(mutex_log_print_ptr);
			output_log << atm_num << ": Transfer " << inserted_amount << " from account " << local_account_src_num
				<< " to account " << local_account_dst_num << " new account balance is " <<
				local_account_src_balance << " new target account balance is " << local_account_dst_balance << endl;
			pthread_mutex_unlock(mutex_log_print_ptr);
		}
		
		else if ((local_account_src_balance < inserted_amount) && !self_trans)
		{
			pthread_mutex_lock(mutex_log_print_ptr);
			output_log << "Error " << atm_num << ": Your transcation failed - account id " << local_account_src_num << " balance is lower than " << inserted_amount << endl;
			pthread_mutex_unlock(mutex_log_print_ptr);
		}
	}
	
	account_src_inst->unlock_for_writers();
	account_dst_inst->unlock_for_writers();
}

bool atm::init_atm_func(void* atm_inst)
{
	if (atm_inst == NULL)
	{
		return false;
	}
	return true;
}
	
// This function executes all atms operations
void atm::all_functions_caller()
{
	ifstream txt(atm_txt_file.c_str());
	char cmd;
	string parse_line;
	
	while (getline(txt, parse_line))
	{
		if (parse_line.empty())
			continue;
		stringstream line_stream(parse_line);
		line_stream >> cmd;
		int inserted_acc_num;
		line_stream >> inserted_acc_num;
		int inserted_password;
		line_stream >> inserted_password;
		int inserted_balance;
		int target_account;
		
		int exist_account = 0;
		account* account_inst;
		account_inst = find_account(inserted_acc_num, &exist_account);
		switch (cmd)
		{

			case 'O':
				line_stream >> inserted_balance;
				if (exist_account)
				{
					error_print(cmd, inserted_acc_num, 0, exist_account);
				}
				else
				{
					O_function(inserted_acc_num, inserted_password, inserted_balance);
				}
				break;

			case 'D':
				line_stream >> inserted_balance;
				if (exist_account)
				{
					D_function(inserted_password, inserted_balance, account_inst);
				}
				else
				{
					error_print(cmd, inserted_acc_num, 0, exist_account);
				}
				break;

			case 'W':
				line_stream >> inserted_balance;
				if (exist_account)
				{
					W_function(inserted_password, inserted_balance, account_inst);
				}
				else
				{
					error_print(cmd, inserted_acc_num, 0, exist_account);
				}
				break;

			case 'B':
				if (exist_account)
				{
					B_function(inserted_password, account_inst);
				}
				else
				{
					error_print(cmd, inserted_acc_num, 0, exist_account);
				}
				break;

			case 'Q':
				if (exist_account)
				{
					if (exist_account)
					{
						Q_function(inserted_password, account_inst);
					}
				}
				else
				{
					error_print(cmd, inserted_acc_num, 0, exist_account);
				}
				break;

			case 'T':
				line_stream >> target_account >> inserted_balance;
				int exist_account_target = 0;
				account* target_account_inst;
				target_account_inst = find_account(target_account, &exist_account_target);
				
				if (exist_account & exist_account_target)
				{
					T_function(inserted_password, inserted_balance, account_inst, target_account_inst);
				}
				else 
				{
					error_print(cmd, inserted_acc_num, target_account, exist_account);
				}
				break;
		}
		
		// In case of an illegal command
		if ((cmd != 'O') && (cmd != 'D') && (cmd != 'W') && (cmd != 'B') && (cmd != 'Q') && (cmd != 'T'))
		{
			pthread_mutex_lock(mutex_log_print_ptr);
			output_log << "Command is illegal" << endl;
			pthread_mutex_unlock(mutex_log_print_ptr);
		}
		// Send the atm to sleep after executing any command
		usleep(ATM_SLEEP);
	}

	txt.close();
	return;
}
