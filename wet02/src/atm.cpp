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

/* initiating the atm function */
bool atm::init_atm_func(void* atm_inst)
{
    if (atm_inst == NULL)
    {
        return false;
    }
    return true;
}

/* a function that in charge of all the atms operations */
void atm::all_functions_caller()
{
    ifstream txt(atm_txt_file.c_str());
	char cmd_line;
	string txt_line;

	while(getline(txt, txt_line))
    {
	    if(txt_line.empty())
            continue;
	    stringstream line_stream(txt_line);
	    line_stream >> cmd_line;
	    int inserted_acc_num;
	    line_stream >> inserted_acc_num;
	    int inserted_password;
	    line_stream >> inserted_password;
	    int inserted_balance;
	    int target_account;

	    int exist_account = 0;
	    account* account_inst;
	    account_inst = find_account(inserted_acc_num, &exist_account);
	    switch(cmd_line)
        {
	        
            case 'O':
                line_stream >> inserted_balance;
                if(exist_account)
                {
                    error_print(cmd_line, inserted_acc_num, 0, exist_account);
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
                    error_print(cmd_line, inserted_acc_num, 0, exist_account);
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
                    error_print(cmd_line, inserted_acc_num, 0, exist_account);
                }
                break;

            case 'B':
                if (exist_account)
                {
                    B_function(inserted_password, account_inst);
                }
                else
                {
                    error_print(cmd_line, inserted_acc_num, 0, exist_account);
                }
                break;

            case 'Q':
                if (exist_account)
                {
                    if(exist_account)
                    {
                        Q_function(inserted_password, account_inst);
                    }
                }
                else
                {
                    error_print(cmd_line, inserted_acc_num, 0, exist_account);
                }
                break;

            case 'T':
                line_stream >> target_account >> inserted_balance;
                int exist_acc_target = 0;
                account* target_acc_inst;
                target_acc_inst = find_account(target_account, &exist_acc_target);

                if(exist_account & exist_acc_target)
                {
                    T_function(inserted_password, inserted_balance, account_inst, target_acc_inst);
                }
                else
                {
                    error_print(cmd_line, inserted_acc_num, target_account, exist_account);
                }
                break;
        }

        // if there is an illegal command
        if ((cmd_line != 'O') && (cmd_line != 'D') && (cmd_line != 'W') && (cmd_line != 'B') && (cmd_line != 'Q') && (cmd_line != 'T'))
        {
            pthread_mutex_lock(mutex_log_print_ptr);
            output_log << "Command is illegal" << endl;
            pthread_mutex_unlock(mutex_log_print_ptr);
        }
        // the atm will go to sleep after any command
        usleep(ATM_SLEEP);
    }
	
	txt.close();
    return;
}

/* a function that opens a new account */
void atm::O_function(int inserted_acc_num, int inserted_password, int inserted_balance)
{
    sleep(1);
    pthread_mutex_lock(mutex_global_accounts_ptr);
    account account_inst = account(inserted_acc_num, inserted_password, inserted_balance);
    map_accounts_ptr->insert(pair<int, account>(inserted_acc_num, account_inst));
    pthread_mutex_unlock(mutex_global_accounts_ptr);

    pthread_mutex_lock(mutex_log_print_ptr);
    output_log << atm_num << ": New account id is " << inserted_password << " with password " << inserted_password << " and initial balance " << inserted_balance << endl;
    pthread_mutex_unlock(mutex_log_print_ptr);
}

/* a function that deposits a certain amount to a certain account by id */
void atm::D_function(int inserted_password, int inserted_amount, account* account_ptr)
{
	account_ptr->lock_for_writers();
	sleep(1);
	int local_acc_num = account_ptr->account_num;
	int local_acc_password = account_ptr->password;
	wrong_password_check_and_print(local_acc_num, local_acc_password, inserted_password);
	if (inserted_password == local_acc_password)
    {
	    account_ptr->balance += inserted_amount;
	    int local_acc_balance = account_ptr->balance;
	    pthread_mutex_lock(mutex_log_print_ptr);
	    output_log << atm_num << ": Account" << local_acc_num << " new balance is " << local_acc_balance << " after " << inserted_amount << " $ was deposited" << endl;
	    pthread_mutex_unlock(mutex_log_print_ptr);
    }
	account_ptr->unlock_for_writers();
}

/* a function that withdraws a certain amount from a certain account*/
void atm::W_function(int inserted_password, int inserted_amount, account* account_ptr)
{
	account_ptr->lock_for_writers();
	sleep(1);
	int local_acc_num = account_ptr->account_num;
	int local_acc_password = account_ptr->password;
	int local_acc_balance = account_ptr->balance;

    wrong_password_check_and_print(local_acc_num, local_acc_password, inserted_password);
    if (inserted_password == local_acc_password)
    {
        if (local_acc_balance < inserted_amount)
        {
            pthread_mutex_lock(mutex_log_print_ptr);
            output_log << "Error " << atm_num << ": Your transaction failed – account id " << local_acc_num << " balance is lower than " << inserted_amount << endl;
            pthread_mutex_unlock(mutex_log_print_ptr);
        }
        else
        {
            account_ptr->balance -= inserted_amount;
            local_acc_balance = account_ptr->balance;
            pthread_mutex_lock(mutex_log_print_ptr);
            output_log << atm_num << ": Account " << local_acc_num << " new balance is " << local_acc_balance << " after " << inserted_amount << " $ was withdrew" << endl;
            pthread_mutex_unlock(mutex_log_print_ptr);
        }
    }
    account_ptr->unlock_for_writers();
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




















