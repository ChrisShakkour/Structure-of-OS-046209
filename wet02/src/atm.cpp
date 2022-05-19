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

/* a function that deletes an account*/
void atm::erase_account_by_id(int local_account_num)
{
    map<int, account>::iterator i;
    i = map_accounts_ptr->find(local_account_num);
    if (i != map_accounts_ptr->end())
    {
        map_accounts_ptr->erase(i);        
    }
}

/* a function that returns a pointer to an account selected by the user */
account* atm::find_account(int inserted_acc_num, int* account_found)
{
	pthread_mutex_lock(mutex_global_accounts_ptr);
	account* account_wanted;
	map<int, account>::iterator i;
	i = map_accounts_ptr->find(inserted_acc_num);
	if (i != map_accounts_ptr->end())
    {
	    i->second.lock_for_readers();
	    account_wanted = &i->second;
	    *account_found = 1;
	    i->second.unlock_for_readers();
    }
	pthread_mutex_unlock(mutex_global_accounts_ptr);
    return account_wanted;
}

/* a function that prints errors according to the right operation */
void atm::error_print(char letter, int inserted_acc_num, int target_account, int is_found) {
    pthread_mutex_lock(mutex_log_print_ptr);
    if (letter == '0')
    {
        output_log << "Error " << atm_num << ": Your transaction failed – account with the same id exists" << endl;
    }
    else if (letter == 'T')
    {
        if(is_found)
        {
            output_log << "Error " << atm_num << ": Your transaction failed – account id " << target_account << " does not exist" << endl;
        }
        else
        {
            output_log << "Error " << atm_num << ": Your transaction failed – account id " << inserted_acc_num << " does not exist" << endl;
        }
    }
    else
    {
        output_log << "Error " << atm_num << ": Your transaction failed – account id " << inserted_acc_num << " does not exist" << endl;
    }
    pthread_mutex_unlock(mutex_log_print_ptr);
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




















