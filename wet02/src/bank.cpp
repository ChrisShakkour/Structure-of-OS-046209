
#include "bank.h"


// number of atms
int num_of_atm;

// output file
ofstream output_log;

// a map of all acounts, shared with all atm's 
map<int, account> map_of_accounts;

// vector of available atm's
vector<atm> vector_of_atm;


int main(int argc, char* argv[])
{	
	// exit if no args provided
	if (argc < 2)
	{
			cout << "Bank error: illegal arguments" << endl;
			exit(1);
	}
	
	int num_of_atm_txt = argc - 1;
	//cout << "atms recieved " << num_of_atm_txt << endl;
	//int num_of_atm = atoi(argv[1]);
	//cout << "atms recieved " << num_of_atm << endl;
		
	
	// atm threads pointers.
	pthread_t* atm_thread_ptr = new pthread_t[num_of_atm];
	
	// thread for printing the status of all acounts.
	pthread_t status_print_thread;
	
	// bank commision charge thread.
	pthread_t commision_charge_thread;
	
	
	return 0;
}



void* bank_commission_routine(void* bank)
{
	
}


void* atm_routine(void* atm)
{
	
}


void* bank_status_routine(void* bank)
{
	
}


