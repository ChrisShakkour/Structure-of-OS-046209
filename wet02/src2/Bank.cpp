#include "account.h"
#include "atm.h"
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <fstream>
#include <iterator>
#include <string>
#include <algorithm>
#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <sstream>
#include <pthread.h>
#include <cstdlib>
#include <ctime>
#include <iterator>
#include <list>
using namespace std;

extern list<Atm> atms;
extern list<Account> accounts;
extern list<string> files;

extern unsigned number_of_list_readers;

bool finished_atms = false;

extern pthread_mutex_t mutex_list_accounts_reader;
extern pthread_mutex_t mutex_list_accounts_writer;
extern pthread_mutex_t mutex_erase_locker;


pthread_mutex_t mutex_output_writer;

ofstream output_file;

int bank_amount;


void*
print_routine(void* main)
{
    while (finished_atms == false)
    {
        printf("\033[2J");
        printf("\033[1;1H");
        printf("Current Bank Status\n");
//        new_list_reader();
    	pthread_mutex_lock(&mutex_list_accounts_writer);        
        list<Account>::iterator it;
        for (it = accounts.begin(); it != accounts.end(); ++it) {
            it->print_status();
        }
//        reader_list_leave();
    	pthread_mutex_unlock(&mutex_list_accounts_writer);
        cout << "." << endl;
        cout << "." << endl;
        cout << "The Bank has " << bank_amount << " $" << endl;
        usleep(500000);
    }
    pthread_exit(NULL);
}

// This function is being called by the bank commission thread
void*
commission_routine(void* main)
{
    bank_amount = 0;
    int precent = 0;
    while (finished_atms == false)
    {
        precent = (rand() % 5) + 1;
//        new_list_reader();
    	pthread_mutex_lock(&mutex_list_accounts_writer);        
        list<Account>::iterator it;
        for (it = accounts.begin(); it != accounts.end(); ++it) {
            it->take_commision(precent);
        }
    	pthread_mutex_unlock(&mutex_list_accounts_writer);
//        reader_list_leave();
        sleep(3);
    }
    pthread_exit(NULL);
}

int main(int argc, char* argv[])
{
    pthread_mutex_init(&mutex_output_writer, NULL);
    pthread_mutex_init(&mutex_list_accounts_reader, NULL);
    pthread_mutex_init(&mutex_list_accounts_writer, NULL);
    pthread_mutex_init(&mutex_erase_locker, NULL);    
    output_file.open("log.txt");

    // Checks the correction of inserted arguments
    if (argc <= 1)
    {
        printf("Bank error: illegal arguments\n");
        exit(1);
    }

    int num_of_atm = argc - 1;
    for (int i = 0; i < num_of_atm; i++) {
        string file(argv[i + 1]);
        files.push_back(file);
    }
    pthread_t* list_of_threads = new pthread_t[num_of_atm];
    Atm* atm = new Atm[num_of_atm];
    for (int j = 0; j < num_of_atm; j++) {
        atm[j].id = j;
        if (pthread_create(&list_of_threads[j], NULL, atm_routine, (void*)&atm[j])) {
            perror("Error: thread fail");
            return 0;
        }
    }

    pthread_t print_status;
    if (pthread_create(&print_status, NULL, print_routine, NULL))
    {
        perror("Error: thread fail");
        return 0;

    }

    pthread_t commision;
    if (pthread_create(&commision, NULL, commission_routine, NULL))
    {
        perror("Error: thread fail");
        return 0;
    }
    for (int i = 0; i < num_of_atm; ++i) {
        if (pthread_join(list_of_threads[i], NULL))
        {
            perror("Error: thread join fail");
            return 0;
        }
    }
    finished_atms = true;
    if (pthread_join(commision, NULL))
    {
        perror("Error: thread join fail");
        return 0;
    }
    if (pthread_join(print_status, NULL))
    {
        perror("Error: thread join fail");
        return 0;
    }
    output_file.close();
    pthread_mutex_destroy(&mutex_list_accounts_reader);
    pthread_mutex_destroy(&mutex_list_accounts_writer);
    pthread_mutex_destroy(&mutex_output_writer);
    delete [] list_of_threads;
    delete [] atm;
    return 0;
}

