#include "atm.h"


extern ofstream output_log;


/*

bool exist_account(int account_id){
	for (list<Account>::iterator it=bank.accounts_list.begin(); it != bank.accounts_list.end(); it++) {
		if((*it).account_id == account_id){
	return true;
}
}
return false;
}
*/


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
	return;
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



/* a function that opens a new account */
void atm::O_function(int inserted_acc_num, int inserted_password, int inserted_balance)
{
	sleep(1);

	// #########################
    // locking global map
    // if map is empty create account
	// else iterate over to check if exists
	// unlock map mutex
	pthread_mutex_lock(mutex_global_accounts_ptr);
	if(map_accounts_ptr->find(inserted_acc_num) != map_accounts_ptr->end()){
	    pthread_mutex_unlock(mutex_global_accounts_ptr);
		pthread_mutex_lock(mutex_log_print_ptr);
		output_log << atm_num << ": your transaction failed - account with the same id exists" << endl;
		usleep(100);
		pthread_mutex_unlock(mutex_log_print_ptr);
	}
	else {
		account account_inst = account(inserted_acc_num, inserted_password, inserted_balance);
    	map_accounts_ptr->insert(pair<int, account>(inserted_acc_num, account_inst));
    	pthread_mutex_unlock(mutex_global_accounts_ptr);
    	pthread_mutex_lock(mutex_log_print_ptr);
    	output_log << atm_num << ": New account id is " << inserted_acc_num << " with password " << inserted_password << " and initial balance " << inserted_balance << endl;
		usleep(100);
    	pthread_mutex_unlock(mutex_log_print_ptr);    			
    }
//    pthread_mutex_unlock(mutex_global_accounts_ptr);
    return;
}




/* a function that deposits a certain amount to a certain account by id */
void atm::D_function(int inserted_acc_num, int inserted_password, int inserted_amount)
{
	sleep(1);
	int local_acc_num;
	int local_acc_password;
	int local_acc_balance;
	
	// #########################
	pthread_mutex_lock(mutex_global_accounts_ptr);
	if(map_accounts_ptr->find(inserted_acc_num) != map_accounts_ptr->end()) {	
		map<int, account>::iterator It;
	    for (It=map_accounts_ptr->begin(); It!=map_accounts_ptr->end(); It++)
	    {
	    	if(It->second.account_num == inserted_acc_num){
	    		pthread_mutex_unlock(mutex_global_accounts_ptr);
	    		
	    		It->second.lock_for_writers();
	    		local_acc_num = It->second.account_num;
	    		local_acc_password = It->second.password;
				local_acc_balance = It->second.balance;
	    		It->second.unlock_for_writers();
	    			    		
	    		//wrong_password_check_and_print(local_acc_num, local_acc_password, inserted_password);
	    		if(inserted_password == local_acc_password)
	    		{
		    		It->second.lock_for_readers();	    		
		    		It->second.balance += inserted_amount;
					It->second.unlock_for_readers();
		    							
					pthread_mutex_lock(mutex_log_print_ptr);
					output_log << atm_num << ": Account " << local_acc_num << " new balance is " << (local_acc_balance+inserted_amount) << " after " << inserted_amount << " $ was deposited" << endl;
					usleep(100);
					pthread_mutex_unlock(mutex_log_print_ptr);
				}
	    		else {
	    		    pthread_mutex_lock(mutex_log_print_ptr);
	    		    output_log << "Error " << atm_num << ": Your transaction failed – password for account id " << local_acc_num << " is incorrect" << endl;
					usleep(100);
	    		    pthread_mutex_unlock(mutex_log_print_ptr);
	    		}
	    		return;
	    	}	        
	    }
	}	
    else {
    	pthread_mutex_unlock(mutex_global_accounts_ptr);
    	pthread_mutex_lock(mutex_log_print_ptr);
        output_log << "Error " << atm_num << ": Your transaction failed – account id " << inserted_acc_num << " does not exist" << endl;
	    usleep(100);
        pthread_mutex_unlock(mutex_log_print_ptr);
    }
    
	pthread_mutex_unlock(mutex_global_accounts_ptr);
	return;
}




/* a function that withdraws a certain amount from a certain account*/
void atm::W_function(int inserted_acc_num, int inserted_password, int inserted_amount)
{
	sleep(1);
	int local_acc_num;
	int local_acc_password;
	int local_acc_balance;
	
	// #########################
	pthread_mutex_lock(mutex_global_accounts_ptr);
	if(map_accounts_ptr->find(inserted_acc_num) != map_accounts_ptr->end()) {	
		map<int, account>::iterator It;
	    for (It=map_accounts_ptr->begin(); It!=map_accounts_ptr->end(); It++)
	    {
	    	if(It->second.account_num == inserted_acc_num){
	    		pthread_mutex_unlock(mutex_global_accounts_ptr);
	    		It->second.lock_for_writers();
	    		local_acc_num = It->second.account_num;
	    		local_acc_password = It->second.password;
				local_acc_balance = It->second.balance;
	    		It->second.unlock_for_writers();

//				wrong_password_check_and_print(local_acc_num, local_acc_password, inserted_password);
	    		if(inserted_password == local_acc_password)
	    		{
	    			if (local_acc_balance < inserted_amount)
		            {
		                pthread_mutex_lock(mutex_log_print_ptr);
		                output_log << "Error " << atm_num << ": Your transaction failed – account id " << local_acc_num << " balance is lower than " << inserted_amount << endl;
						usleep(100);
		                pthread_mutex_unlock(mutex_log_print_ptr);
		                return;
		            }
		            else
		            {
						It->second.lock_for_readers();	    		
						It->second.balance -= inserted_amount;			            
						It->second.unlock_for_readers();

						pthread_mutex_lock(mutex_log_print_ptr);
						output_log << atm_num << ": Account " << local_acc_num << " new balance is " << (local_acc_balance-inserted_amount) << " after " << inserted_amount << " $ was withdrew" << endl;
						usleep(100);
						pthread_mutex_unlock(mutex_log_print_ptr);
						return;
		            }
		        }  		
	    		else {
	    		    pthread_mutex_lock(mutex_log_print_ptr);
	    		    output_log << "Error " << atm_num << ": Your transaction failed – password for account id " << local_acc_num << " is incorrect" << endl;
					usleep(100);
	    		    pthread_mutex_unlock(mutex_log_print_ptr);
	    		    return;
	    		}	    		
	    	}	        
	    }
	}	
    else {
    	pthread_mutex_unlock(mutex_global_accounts_ptr);
    	pthread_mutex_lock(mutex_log_print_ptr);
        output_log << "Error " << atm_num << ": Your transaction failed – account id " << inserted_acc_num << " does not exist" << endl;
		usleep(100);
	    pthread_mutex_unlock(mutex_log_print_ptr);
	    return;
    }
    
	pthread_mutex_unlock(mutex_global_accounts_ptr);
	return;

}



/* a function who prints the balance of a certain account by id */
void atm::B_function(int inserted_acc_num, int inserted_password)
{	
	sleep(1);
	int local_acc_num;
	int local_acc_password;
	int local_acc_balance;
	
	// #########################
	pthread_mutex_lock(mutex_global_accounts_ptr);
	if(map_accounts_ptr->find(inserted_acc_num) != map_accounts_ptr->end()) {	
		map<int, account>::iterator It;
	    for (It=map_accounts_ptr->begin(); It!=map_accounts_ptr->end(); It++)
	    {
	    	if(It->second.account_num == inserted_acc_num){
	    		pthread_mutex_unlock(mutex_global_accounts_ptr);
	    		
	    		It->second.lock_for_writers();
	    		local_acc_num = It->second.account_num;
	    		local_acc_password = It->second.password;
				local_acc_balance = It->second.balance;
	    		It->second.unlock_for_writers();
	    			    		
	    		//wrong_password_check_and_print(local_acc_num, local_acc_password, inserted_password);
	    		if(inserted_password == local_acc_password)
	    		{
		    		pthread_mutex_lock(mutex_log_print_ptr);
		            output_log << atm_num << ": Account " << local_acc_num << " balance is " << local_acc_balance << endl;
					usleep(100);
					pthread_mutex_unlock(mutex_log_print_ptr);
					return;
	    		}  		
	    		else {
	    		    pthread_mutex_lock(mutex_log_print_ptr);
	    		    output_log << "Error " << atm_num << ": Your transaction failed – password for account id " << local_acc_num << " is incorrect" << endl;
					usleep(100);
	    		    pthread_mutex_unlock(mutex_log_print_ptr);
	    		    return;
	    		}	    		
	    	}	        
	    }
	}	
    else {
    	pthread_mutex_unlock(mutex_global_accounts_ptr);
    	pthread_mutex_lock(mutex_log_print_ptr);
        output_log << "Error " << atm_num << ": Your transaction failed – account id " << inserted_acc_num << " does not exist" << endl;
		usleep(100);
	    pthread_mutex_unlock(mutex_log_print_ptr);
	    return;
    }
    
	pthread_mutex_unlock(mutex_global_accounts_ptr);
	return;
}


/* a function that removes an account by his id */
void atm::Q_function(int inserted_acc_num, int inserted_password)
{
	sleep(1);
	int local_acc_num;
	int local_acc_password;
	int local_acc_balance;
	
	// #########################
	pthread_mutex_lock(mutex_global_accounts_ptr);
	if(map_accounts_ptr->find(inserted_acc_num) != map_accounts_ptr->end()) {	
		map<int, account>::iterator It;
	    for (It=map_accounts_ptr->begin(); It!=map_accounts_ptr->end(); It++)
	    {
	    	if(It->second.account_num == inserted_acc_num){	            
	    		It->second.lock_for_writers();
	    		local_acc_num = It->second.account_num;
	    		local_acc_password = It->second.password;
				local_acc_balance = It->second.balance;
	    		It->second.unlock_for_writers();

	    		//wrong_password_check_and_print(local_acc_num, local_acc_password, inserted_password);
	    		if(inserted_password == local_acc_password)
	    		{
		    		//delete It->second;
		    		map_accounts_ptr->erase(It);
		    		pthread_mutex_unlock(mutex_global_accounts_ptr);
		    		pthread_mutex_lock(mutex_log_print_ptr);
		            output_log << atm_num << ": Account " << local_acc_num << " is now closed. Balance was " << local_acc_balance << endl;
					usleep(100);
					pthread_mutex_unlock(mutex_log_print_ptr);
					return;	
	    		}  		
	    		else {
	    		    pthread_mutex_lock(mutex_log_print_ptr);
	    		    output_log << "Error " << atm_num << ": Your transaction failed – password for account id " << local_acc_num << " is incorrect" << endl;
					usleep(100);
	    		    pthread_mutex_unlock(mutex_log_print_ptr);
	    		    return;
	    		}	    		
	    	}	        
	    }
	}	
    else {
    	pthread_mutex_unlock(mutex_global_accounts_ptr);
    	pthread_mutex_lock(mutex_log_print_ptr);
        output_log << "Error " << atm_num << ": Your transaction failed – account id " << inserted_acc_num << " does not exist" << endl;
		usleep(100);
	    pthread_mutex_unlock(mutex_log_print_ptr);
	    return;
    }
    
	pthread_mutex_unlock(mutex_global_accounts_ptr);
	return;
}


/* a function that transfers a certain amount from a certain account to another*/
void atm::T_function(int inserted_acc_num, int inserted_password, int inserted_amount, int target_acc_num)
{
	sleep(1);
	
	account* src_acc;
	
	int local_acc_num;
	int local_acc_password;
	int local_acc_balance;
	int target_acc_balance;
	
	// #########################
	pthread_mutex_lock(mutex_global_accounts_ptr);
	if(map_accounts_ptr->find(inserted_acc_num) != map_accounts_ptr->end())
	{	
		map<int, account>::iterator It;
	    for (It=map_accounts_ptr->begin(); It!=map_accounts_ptr->end(); It++)
	    {
	    	if(It->second.account_num == inserted_acc_num){
	    		src_acc = &(It->second);
	    		//pthread_mutex_unlock(atm::mutex_global_accounts_ptr);	    		
	    		It->second.lock_for_writers();
	    		local_acc_num = It->second.account_num;
	    		local_acc_password = It->second.password;
				local_acc_balance = It->second.balance;
	    		It->second.unlock_for_writers();    		
	    		//wrong_password_check_and_print(local_acc_num, local_acc_password, inserted_password);
	    		if(inserted_password == local_acc_password)
	    		{
//	    			pthread_mutex_lock(atm::mutex_global_accounts_ptr);
	    			if(map_accounts_ptr->find(target_acc_num) != map_accounts_ptr->end())
	    			{
	    			    for (It=map_accounts_ptr->begin(); It!=map_accounts_ptr->end(); It++)
	    			    {
	    			    	if(It->second.account_num == target_acc_num){
	    			    		It->second.lock_for_writers();
	    						target_acc_balance = It->second.balance;
	    			    		It->second.unlock_for_writers();    		
	    			            if (local_acc_balance >= inserted_amount){

	    			            	src_acc->lock_for_readers();
	    			                src_acc->balance -= inserted_amount;
	    			                src_acc->unlock_for_readers();
	    			                
	    			            	It->second.unlock_for_readers();	    			                
	    			                It->second.balance += inserted_amount;
	    			                It->second.unlock_for_readers();
	    			                
	    			                pthread_mutex_lock(mutex_log_print_ptr);
	    			                output_log << atm_num << ": Transfer " << inserted_amount << " from account " << local_acc_num << " to account " << target_acc_num << " new account balance is " << (local_acc_balance-inserted_amount) << " new target balance is " << (target_acc_balance+inserted_amount) << endl;
	    							usleep(100);
	    			                pthread_mutex_unlock(mutex_log_print_ptr);
	    			                return;
	    			            }
	    			            else {
//		    			    		pthread_mutex_unlock(mutex_global_accounts_ptr);
	    			                pthread_mutex_lock(mutex_log_print_ptr);
	    			                output_log << "Error " << atm_num << ": Your transaction failed – account id " << local_acc_num << " balance is lower than " << inserted_amount << endl;
	    							usleep(100);
	    			                pthread_mutex_unlock(mutex_log_print_ptr);
		    			    		pthread_mutex_unlock(mutex_global_accounts_ptr);
	    			                return;
	    			            }
	    			        }
	    			    }
	    			}
					else {
						pthread_mutex_lock(mutex_log_print_ptr);
						output_log << "Error " << atm_num << ": Your transaction failed – account id " << target_acc_num << " does not exist" << endl;
						usleep(100);
						pthread_mutex_unlock(mutex_log_print_ptr);
						return;
					}	
	    		}
	    		else {	
	    		    pthread_mutex_lock(mutex_log_print_ptr);
	    		    output_log << "Error " << atm_num << ": Your transaction failed – password for account id " << local_acc_num << " is incorrect" << endl;
					usleep(100);
	    		    pthread_mutex_unlock(mutex_log_print_ptr);
	    		    return;
	    		}	    		
	    	}	        
	    }
	}	
    else {
    	pthread_mutex_unlock(mutex_global_accounts_ptr);
    	pthread_mutex_lock(mutex_log_print_ptr);
        output_log << "Error " << atm_num << ": Your transaction failed – account id " << inserted_acc_num << " does not exist" << endl;
		usleep(100);
	    pthread_mutex_unlock(mutex_log_print_ptr);
	    return;
    }
    
	pthread_mutex_unlock(mutex_global_accounts_ptr);
	return;
}



/* a function that in charge of all the atms operations */
void atm::all_functions_caller()
{
    ifstream txt(atm_txt_file.c_str());
	char cmd_line;
	string txt_line;
	int inserted_acc_num;
	int inserted_password;
    int inserted_balance;
    int target_account;
    int exist_account = 0;
    
	
	while(getline(txt, txt_line))
    {
	    if(txt_line.empty())
            continue;
	    stringstream line_stream(txt_line);
	    line_stream >> cmd_line;
	    line_stream >> inserted_acc_num;
	    line_stream >> inserted_password;
	    switch(cmd_line)
        {
            case 'O':
                line_stream >> inserted_balance;
            	O_function(inserted_acc_num, inserted_password, inserted_balance);
                break;
            case 'D':
                line_stream >> inserted_balance;
                D_function(inserted_acc_num, inserted_password, inserted_balance);
                break;
            case 'W':
                line_stream >> inserted_balance;
                W_function(inserted_acc_num, inserted_password, inserted_balance);
                break;
            case 'B':
                B_function(inserted_acc_num, inserted_password);
                break;
            case 'Q':
            	Q_function(inserted_acc_num, inserted_password);
            	break;

            case 'T':
            	line_stream >> target_account >> inserted_balance;
                T_function(inserted_acc_num, inserted_password, inserted_balance, target_account);
                break;
        }

        // if there is an illegal command
        if ((cmd_line != 'O') && (cmd_line != 'D') && (cmd_line != 'W') && (cmd_line != 'B') && (cmd_line != 'Q') && (cmd_line != 'T'))
        {
            pthread_mutex_lock(mutex_log_print_ptr);
            output_log << "Command is illegal" << endl;
			usleep(100);
            pthread_mutex_unlock(mutex_log_print_ptr);
        }
        // the atm will go to sleep after any command
        usleep(ATM_SLEEP);
    }
	
	//txt.close();
    return;
}




















