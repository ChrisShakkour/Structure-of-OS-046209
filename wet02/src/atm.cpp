#include "atm.h"


extern ofstream output_log;


/* a function that determines whether the password inserted by the user
 * is correct or not*/
void atm::wrong_password_check_and_print (int acc_id, int acc_password, int user_password)
{
	if (user_password != acc_password)
    {
	    pthread_mutex_lock(mutex_log_print_ptr);
	    output_log << "Error " << atm_num << ": Your transaction failed – password for account id " << acc_id << " is incorrect" << endl;
	    pthread_mutex_unlock(mutex_log_print_ptr);
    }
}

/* description */
void atm::erase_account_by_id(int local_account_num)
{
	
}

/* description */
account* atm::find_account(int inserted_acc_num, int* is_found)
{
	
}

/* description */
void atm::error_print(char letter, int inserted_acc_num, int target_account, int is_found)
{
	
}

/* description */
bool atm::init_atm_func(void* atm_inst)
{
	
}

/* description */
void atm::all_functions_caller()
{
	
}

/* description */
void atm::O_function(int inserted_acc_num, int inserted_password, int inserted_balance)
{
	
}

/* description */
void atm::D_function(int inserted_password, int inserted_amount, account* account_ptr)
{
	
}

/* description */
void atm::W_function(int inserted_password, int inserted_amount, account* account_ptr)
{
	
}

/* description */
void atm::B_function(int inserted_password, account* account_ptr)
{
	
}

/* description */
void atm::Q_function(int inserted_password, account* account_ptr)
{
	
}

/* description */
void atm::T_function(int inserted_password_src, int inserted_amount, account* account_src_ptr, account* account_dest_ptr)
{
	
}




















