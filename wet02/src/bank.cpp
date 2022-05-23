
#include "bank.h"

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

void* bank_status_routine(void* bank)
{
	// TODO: remove the code below until ##
	cout << "print routine called" << endl;
	sleep(2);
	cout << "print routine done" << endl;
	// ##
	
	
	pthread_exit(NULL);
}


bool is_file_exist(const char *fileName)
{
    std::ifstream infile(fileName);
    return infile.good();
}


