
#include "bank.h"

using namespace std;
#define PRINT_DELAY 500000
#define COMMISSION_DELAY 3

// GLOBAL VARIABLES
vector<string> files_vector;

// a map of all acounts, shared with all atm's 
map<int, account> map_of_accounts;

// vector of available atm's
vector<atm> vector_of_atm;

// number of atms
int num_of_atm;

// output file
ofstream output_log;

// a function who takes random commission from all the accounts
bool bank::commission()
{
    pthread_mutex_lock(&mutex_global_accounts);
    int tot_comiss = 0;
    map<int, account>::iterator i;
    srand((unsigned) time(0));
    double rand_num = (rand() % 2) + 2;
    map<int, account>::iterator begin_i = map_accounts_ptr->begin();
    map<int, account>::iterator last_i = map_accounts_ptr->end();

    for (i = begin_i; i != last_i; ++i)
    {
        i->second.lock_for_writers();
        int curr_balance = i->second.balance;
        int curr_comiss = (int)((rand_num / 100) * curr_balance);
        i->second.balance = curr_balance - curr_comiss;
        int curr_acc = i->second.account_num;
        pthread_mutex_lock(&mutex_log_print);
        output_log << "Bank: commissions of " << rand_num << " % were charged, the bank gained " << curr_comiss << " $ from account " << curr_acc << "\n";
        pthread_mutex_unlock(&mutex_log_print);
        tot_comiss += curr_comiss;
        i->second.unlock_for_writers();
    }
    pthread_mutex_unlock(&mutex_global_accounts);
    current_balance_bank += tot_comiss;
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
    map<int, account>::iterator i;
    map<int, account>::iterator i_begin = map_accounts_ptr->begin();
    map<int, account>::iterator i_last = map_accounts_ptr->end();
    for (i = i_begin; i != i_last; ++i)
    {
        i->second.lock_for_readers();
        int curr_acc_num = i->second.account_num;
        int curr_acc_balance = i->second.balance;
        int curr_acc_password = i->second.password;
        i->second.unlock_for_readers();
        cout << "Account " << curr_acc_num << ": Balance – " << curr_acc_balance << " $, Account Password – " << curr_acc_password << endl;
    }
    cout << "." << endl;
    cout << "." << endl;
    cout << "The Bank has " << current_balance_bank << " $" << endl;
    pthread_mutex_unlock(&mutex_global_accounts);
    if(!is_atm_finished) {return false;}
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

/* a function who is being called by the atm threads*/
void* atm_routine(void* atm_in)
{
    atm* atm_mask = (atm*)atm_in;
    if (atm_mask->init_atm_func(atm_in))
    {
        atm_mask->all_functions_caller();
        delete atm_mask;
        pthread_exit(NULL);
    }
    delete atm_mask;
	pthread_exit(NULL);
}

/*a function who is in charge on the commission thread*/
void* bank_commission_routine(void* main_bank)
{
    bool finished_bank;
    bank* bank_type = (bank*)main_bank;
    if(bank_type->init_print_bank_func(main_bank))
    {
        while (true)
        {
            finished_bank = bank_type->commission();
            sleep(COMMISSION_DELAY);
            if (finished_bank)
                break;
        }
        pthread_exit(NULL);
        delete(bank_type);
    }
    pthread_exit(NULL);
    delete(bank_type);
}

/* a function who is being called by the bank print thread*/
void* bank_status_routine(void* main_bank)
{
    bank* bank_type = (bank*)main_bank;
    bool finished_bank;
    if(bank_type->init_print_bank_func(main_bank))
    {
        while (true)
        {
            finished_bank = bank_type->bank_balance_print();
            usleep(PRINT_DELAY);
            if (finished_bank)
                break;
        }
        pthread_exit(NULL);
        delete(bank_type);
    }
	pthread_exit(NULL);
    delete(bank_type);
}

int main(int argc, char* argv[])
{	
	// exit if no args provided
	if (argc < 2)
	{
		cout << "Bank error: illegal arguments" << endl;
		exit(1);
	}
	
	num_of_atm = argc - 1;
    for (int i = 1; i <= num_of_atm; i++) {
        string file = argv[i];
        if(!is_file_exist(&file[0])) {
    		cout << "Bank error: illegal arguments" << endl;        	
        	exit(1);
        }
        files_vector.push_back(file);
    }
    
    bank* main_bank = new bank(&map_of_accounts, &vector_of_atm, num_of_atm);
    atm* temp_atm;
    output_log.open("log.txt", ios::out);   
    
	// atm threads pointers.
	pthread_t* atm_thread_ptr = new pthread_t[num_of_atm];
    for (int i = 0; i < num_of_atm; i++) {
        temp_atm = new atm(i+1, &map_of_accounts, files_vector[i], &main_bank->mutex_log_print, &main_bank->mutex_global_accounts);
        if (pthread_create(&atm_thread_ptr[i], NULL, atm_routine, (void *)temp_atm)) {
            perror("Error: thread fail");
        }
        vector_of_atm.push_back(*temp_atm);
    }
    
	// thread for printing the status of all acounts.
	pthread_t status_print_thread;
    if (pthread_create(&status_print_thread, NULL, bank_status_routine, (void*)main_bank))
        perror("Error: thread fail");

    
	// bank commision charge thread.
	pthread_t commision_charge_thread;
    if (pthread_create(&commision_charge_thread, NULL, bank_commission_routine, (void*)main_bank))
        perror("Error: thread fail");
    
    
    // wait for all atm threads to finish execution.
    for (int i=0; i< num_of_atm; i++) {
        if (pthread_join(atm_thread_ptr[i], NULL))
            perror("Error: thread join fail");
    }
    main_bank->is_atm_finished = 1;

    if (pthread_join(commision_charge_thread, NULL))
    	perror("Error: thread join fail");
    if (pthread_join(status_print_thread, NULL))
        perror("Error: thread join fail");
    
    // Clear memory
    vector_of_atm.clear();
    output_log.close();
    delete[] atm_thread_ptr;
    delete main_bank; 

    return 0;
}

bool is_file_exist(const char *fileName)
{
    std::ifstream infile(fileName);
    return infile.good();
}


