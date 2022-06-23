#include "atm.h"
#include "account.h"
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
#define COMMAND_PARTS 5

list<Atm> atms;
list<string> files;
list<Account> accounts;
unsigned number_of_list_readers = 0;

extern ofstream output_file;

list<Atm>::iterator it_atm;
list<Account>::iterator it_account;
list<Account>::iterator it_account_2;
list<string>::iterator it_files;

pthread_mutex_t mutex_list_accounts_reader;
pthread_mutex_t mutex_list_accounts_writer;

pthread_mutex_t mutex_erase_locker;

extern pthread_mutex_t mutex_output_writer;
using namespace std;


void new_list_reader() {
    pthread_mutex_lock(&mutex_list_accounts_reader);
    number_of_list_readers++;
    if (number_of_list_readers == 1){
        pthread_mutex_lock(&mutex_list_accounts_writer);
    }
    pthread_mutex_unlock(&mutex_list_accounts_reader);
}


void reader_list_leave() {
    pthread_mutex_lock(&mutex_list_accounts_reader);
    number_of_list_readers--;
    if (number_of_list_readers == 0) {
        pthread_mutex_unlock(&mutex_list_accounts_writer);
    }
    pthread_mutex_unlock(&mutex_list_accounts_reader);
}

bool compare_acc_num(const Account& a, const Account& b)
{
    return a.account_num < b.account_num;
}


int account_exist(int account) {
	//pthread_mutex_lock(&mutex_erase_locker);	
    list<Account>::iterator it;
    int index = 0;
    for (it = accounts.begin(); it != accounts.end(); ++it) {
        if (it->account_num == account) {
        	//pthread_mutex_unlock(&mutex_erase_locker);	
            return index;
        }
        index++;
    }
	//pthread_mutex_unlock(&mutex_erase_locker);	
    return -1;
}


bool correct_password(string password, int account_number) {
    list<Account>::iterator it;
    it = accounts.begin();
    advance(it, account_number);
    if (it->password.compare(password) == 0) {
            return true;
    }
    return false;
}


void*
atm_routine(void* atm_index_p) {

    int atm_index = (((Atm*)atm_index_p)->id);
    string line;
    it_atm = atms.begin();
    advance(it_atm, atm_index);
    it_files = files.begin();
    advance(it_files, atm_index);


    ifstream input_file(it_files->c_str());
    if (!input_file.is_open()) {
        perror("Bank error: illegal arguments");
        exit(1);
    }

    atm_index++;
    while (getline(input_file, line)){
        char* full_command[COMMAND_PARTS];
        char* cmd;
        char *ptr;
        const char* delimiters = " \t\n";
        int i = 0;
        char* writable = new char[line.size() + 1];
        std::copy(line.begin(), line.end(), writable);
        writable[line.size()] = '\0'; // don't forget the terminating 0
        cmd = strtok(writable, delimiters);
        if (cmd == NULL)
            pthread_exit(NULL);

        full_command[0] = cmd;
        for (i=1; i < COMMAND_PARTS; i++)
        {
            full_command[i] = strtok(NULL, delimiters);
        }
        int account_atoi = strtol(full_command[1], &ptr, 10);
        string password(full_command[2]);
        
        switch(*cmd) {
            case 'O': {
//            	pthread_mutex_lock(&mutex_list_accounts_reader);
            	pthread_mutex_lock(&mutex_list_accounts_writer);
//            	number_of_list_readers++;
                int account_number = account_exist(account_atoi); //TODO
                it_account = accounts.begin();
                advance(it_account, account_number);
                if (account_number >= 0) {
                    pthread_mutex_lock(&mutex_output_writer);
                    output_file << "Error " << atm_index << ": Your transaction failed – account with the same id exists" << endl;
                    pthread_mutex_unlock(&mutex_output_writer);
//                	pthread_mutex_unlock(&mutex_list_accounts_reader);
//                	number_of_list_readers--;
                    pthread_mutex_unlock(&mutex_list_accounts_writer);
                    sleep(ATM_OP_SLEEP);
                    break;
                }
                int amount = strtol(full_command[3], &ptr, 10);
                Account account(account_atoi, password, amount);
                accounts.push_back(account);
                accounts.sort(compare_acc_num);
                pthread_mutex_lock(&mutex_output_writer); // TODO
                output_file << atm_index << ": New account id is " << account_atoi << " with password " << password <<" and initial balance " << amount << endl;
                pthread_mutex_unlock(&mutex_output_writer);
//            	pthread_mutex_unlock(&mutex_list_accounts_reader);
//            	number_of_list_readers--;
                pthread_mutex_unlock(&mutex_list_accounts_writer);
                sleep(ATM_OP_SLEEP);
                break;
            }
            case 'D': {
                //new_list_reader();
            	pthread_mutex_lock(&mutex_list_accounts_writer);
                int account_number = account_exist(account_atoi);
                it_account = accounts.begin();
                advance(it_account, account_number);
                if (account_number == -1) {
                    pthread_mutex_lock(&mutex_output_writer);
                    output_file << "Error " << atm_index << ": Your transaction failed – account id " << account_atoi << " does not exist" << endl;
                    pthread_mutex_unlock(&mutex_output_writer);
//                    reader_list_leave();
                    pthread_mutex_unlock(&mutex_list_accounts_writer);
                    sleep(ATM_OP_SLEEP);
                    break;
                }
                bool correctness_password = correct_password(password, account_number);
                if (correctness_password == false) {
                    pthread_mutex_lock(&mutex_output_writer);
                    output_file << "Error " << atm_index << ": Your transaction failed – password for account id " << account_atoi << " is incorrect" << endl;
                    pthread_mutex_unlock(&mutex_output_writer);
//                    reader_list_leave();
                    pthread_mutex_unlock(&mutex_list_accounts_writer);
                    sleep(ATM_OP_SLEEP);
                    break;
                }
                int amount = atoi(full_command[3]);
                it_account->inc_amount(amount, atm_index);
//                reader_list_leave();
                pthread_mutex_unlock(&mutex_list_accounts_writer);
                sleep(ATM_OP_SLEEP);
                break;
            }
            case 'W': {
//                new_list_reader();
            	pthread_mutex_lock(&mutex_list_accounts_writer);
                int account_number = account_exist(account_atoi);
                it_account = accounts.begin();
                advance(it_account, account_number);
                if (account_number == -1) {
                    pthread_mutex_lock(&mutex_output_writer);
                    output_file << "Error " << atm_index << ": Your transaction failed – account id " << account_atoi << " does not exist" << endl;
                    pthread_mutex_unlock(&mutex_output_writer);
//                    reader_list_leave();
                    pthread_mutex_unlock(&mutex_list_accounts_writer);
                    sleep(ATM_OP_SLEEP);
                    break;
                }
                bool correctness_password = correct_password(password, account_number);
                if (correctness_password == false) {
                    pthread_mutex_lock(&mutex_output_writer);
                    output_file << "Error " << atm_index << ": Your transaction failed – password for account id " << account_atoi << " is incorrect" << endl;
                    pthread_mutex_unlock(&mutex_output_writer);
//                    reader_list_leave();
                    pthread_mutex_unlock(&mutex_list_accounts_writer);
                    sleep(ATM_OP_SLEEP);
                    break;
                }
                int amount = atoi(full_command[3]);
                it_account->purchase(amount, atm_index);
//                reader_list_leave();
                pthread_mutex_unlock(&mutex_list_accounts_writer);
                sleep(ATM_OP_SLEEP);
                break;
            }
            case 'B': {
//                new_list_reader();
                pthread_mutex_lock(&mutex_list_accounts_writer);
                int account_number = account_exist(account_atoi);
                it_account = accounts.begin();
                advance(it_account, account_number);
                if (account_number == -1) {
                    pthread_mutex_lock(&mutex_output_writer);
                    output_file << "Error " << atm_index << ": Your transaction failed – account id " << account_atoi << " does not exist" << endl;
                    pthread_mutex_unlock(&mutex_output_writer);
//                    reader_list_leave();
                    pthread_mutex_unlock(&mutex_list_accounts_writer);
                    sleep(ATM_OP_SLEEP);
                    break;
                }
                bool correctness_password = correct_password(password, account_number);
                if (correctness_password == false) {
                    pthread_mutex_lock(&mutex_output_writer);
                    output_file << "Error " << atm_index << ": Your transaction failed – password for account id " << account_atoi << " is incorrect" << endl;
                    pthread_mutex_unlock(&mutex_output_writer);
//                    reader_list_leave();
                    pthread_mutex_unlock(&mutex_list_accounts_writer);
                    sleep(ATM_OP_SLEEP);
                    break;
                }
                it_account->check_amount(atm_index);
//                reader_list_leave();
                pthread_mutex_unlock(&mutex_list_accounts_writer);
                sleep(ATM_OP_SLEEP);
                break;
            }
            case  'Q': {
                pthread_mutex_lock(&mutex_list_accounts_writer);
                int account_number = account_exist(account_atoi);
                it_account = accounts.begin();
                advance(it_account, account_number);
                if (account_number == -1) {
                    pthread_mutex_lock(&mutex_output_writer);
                    output_file << "Error " << atm_index << ": Your transaction failed – account id " << account_atoi << " does not exist" << endl;
                    pthread_mutex_unlock(&mutex_output_writer);
                    pthread_mutex_unlock(&mutex_list_accounts_writer);
                    sleep(ATM_OP_SLEEP);
                    break;
                }
                bool correctness_password = correct_password(password, account_number);
                if (correctness_password == false) {
                    pthread_mutex_lock(&mutex_output_writer);
                    output_file << "Error " << atm_index << ": Your transaction failed – password for account id " << account_atoi << " is incorrect" << endl;
                    pthread_mutex_unlock(&mutex_output_writer);
                    pthread_mutex_unlock(&mutex_list_accounts_writer);
                    sleep(ATM_OP_SLEEP);
                    break;
                }
//                pthread_mutex_lock(&mutex_list_accounts_reader);
//                if(number_of_list_readers != 0){
//                	pthread_mutex_lock(&mutex_list_accounts_reader);
//                }
            	//pthread_mutex_lock(&mutex_erase_locker);	
                accounts.erase(it_account);
                accounts.sort(compare_acc_num);
            	//pthread_mutex_unlock(&mutex_erase_locker);	

//                pthread_mutex_unlock(&mutex_list_accounts_reader);                
                pthread_mutex_unlock(&mutex_list_accounts_writer);
                sleep(ATM_OP_SLEEP);
                break;
            }
            case  'T': {
//                new_list_reader();
            	pthread_mutex_lock(&mutex_list_accounts_writer);
            	int account_number = account_exist(account_atoi);
                it_account = accounts.begin();
                advance(it_account, account_number);
                int account_2 = atoi(full_command[3]);
                int account_number_2 = account_exist(account_2);
                it_account_2 = accounts.begin();
                advance(it_account_2, account_number_2);
                if (account_number == -1) {
                    pthread_mutex_lock(&mutex_output_writer);
                    output_file << "Error " << atm_index << ": Your transaction failed – account id " << account_atoi << " does not exist" << endl;
                    pthread_mutex_unlock(&mutex_output_writer);
//                    reader_list_leave();
                    pthread_mutex_unlock(&mutex_list_accounts_writer);
                    sleep(ATM_OP_SLEEP);
                    break;
                }
                if (account_number_2 == -1) {
                    pthread_mutex_lock(&mutex_output_writer);
                    output_file << "Error " << atm_index << ": Your transaction failed – account id " << account_2 << " does not exist" << endl;
                    pthread_mutex_unlock(&mutex_output_writer);
//                    reader_list_leave();
                    pthread_mutex_unlock(&mutex_list_accounts_writer);
                    sleep(ATM_OP_SLEEP);
                    break;
                }
                bool correctness_password = correct_password(password, account_number);
                if (correctness_password == false) {
                    pthread_mutex_lock(&mutex_output_writer);
                    output_file << "Error " << atm_index << ": Your transaction failed – password for account id " << account_atoi << " is incorrect" << endl;
                    pthread_mutex_unlock(&mutex_output_writer);
//                    reader_list_leave();
                    pthread_mutex_unlock(&mutex_list_accounts_writer);
                    sleep(ATM_OP_SLEEP);
                    break;	
                }
                int amount = atoi(full_command[3]);
                it_account->pass_amount(amount, atm_index);
//                reader_list_leave();
                pthread_mutex_unlock(&mutex_list_accounts_writer);
                sleep(ATM_OP_SLEEP);
                break;
            }
        }
        delete[] writable;
        usleep(ATM_OP_RETIRE);
    }
    input_file.close();
    pthread_exit(NULL);
}







