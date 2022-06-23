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
extern list<Account> accounts;
extern ofstream output_file;

extern list<Atm>::iterator it_atm;
extern list<Account>::iterator it_account;
extern list<Account>::iterator it_account_2;
extern list<string>::iterator it_files;

extern int bank_amount;

extern pthread_mutex_t mutex_output_writer;

void
Account::new_account_reader() {
        pthread_mutex_lock(&mutex_account_read);
        number_of_readers++;
        if (number_of_readers == 1) {
            pthread_mutex_lock(&mutex_account_write);
        }
        pthread_mutex_unlock(&mutex_account_read);
}


void
Account::reader_account_leave() {
    pthread_mutex_lock(&mutex_account_read);
    number_of_readers--;
    if (number_of_readers == 0) {
        pthread_mutex_unlock(&mutex_account_write);
    }
    pthread_mutex_unlock(&mutex_account_read);
}


bool
Account::purchase(int amount_to_purchase, int atm_id) {
    pthread_mutex_lock(&mutex_account_write);
    sleep(1);
    if (amount_to_purchase > amount) {
        pthread_mutex_lock(&mutex_output_writer);
        output_file << "Error "<< atm_id <<": Your transaction failed – account id "<<account_num<<" balance is lower than "<<amount_to_purchase<< endl;
        pthread_mutex_unlock(&mutex_output_writer);
        pthread_mutex_unlock(&mutex_account_write);
        return false;
    }
    else
    {
        pthread_mutex_lock(&mutex_output_writer);
        output_file << atm_id <<": Account "<<account_num<<" new balance is " << amount - amount_to_purchase<<" after "<< amount_to_purchase<<" $ was withdrew"<< endl;
        pthread_mutex_unlock(&mutex_output_writer);
        amount -= amount_to_purchase;
        pthread_mutex_unlock(&mutex_account_write);
        return true;
    }
}

void
Account::inc_amount(int inc_amount, int atm_id) {
    pthread_mutex_lock(&mutex_account_write);
    sleep(1);
    pthread_mutex_lock(&mutex_output_writer);
    output_file << atm_id <<": Account "<<account_num<<" new balance is " << amount + inc_amount << " after " << inc_amount <<" $ was deposited" <<endl;
    pthread_mutex_unlock(&mutex_output_writer);
    amount += inc_amount;
    pthread_mutex_unlock(&mutex_account_write);
}

void
Account::inc_amount_no_sleep(int inc_amount, int atm_id) {
    pthread_mutex_lock(&mutex_account_write);
    pthread_mutex_lock(&mutex_output_writer);
    output_file << atm_id <<": Account "<<account_num<<" new balance is " << amount + inc_amount << " after " << inc_amount <<" $ was deposited" <<endl;
    pthread_mutex_unlock(&mutex_output_writer);
    amount += inc_amount;
    pthread_mutex_unlock(&mutex_account_write);
}

void
Account::check_amount(int atm_id)
{
    new_account_reader();
    pthread_mutex_lock(&mutex_output_writer);
    output_file <<atm_id<<": Account "<<account_num<<" balance is "<<amount << endl;
    pthread_mutex_unlock(&mutex_output_writer);
    reader_account_leave();
}

bool
Account::pass_amount(int amount_to_purchase,int atm_id) {
    pthread_mutex_lock(&mutex_account_write);
    sleep(1);
    if (amount_to_purchase > amount) {
        pthread_mutex_lock(&mutex_output_writer);
        output_file << "Error "<< atm_id <<": Your transaction failed – account id "<<account_num<<" balance is lower than "<<amount_to_purchase<< endl;
        pthread_mutex_unlock(&mutex_output_writer);
        pthread_mutex_unlock(&mutex_account_write);
        return false;
    }
    else
    {
        output_file <<atm_id <<": Transfer " << amount_to_purchase <<" from account " << account_num<<" to account " <<it_account_2->account_num << " new account balance is " << amount <<" new target account balance is " << it_account_2->amount << endl;
        pthread_mutex_unlock(&mutex_output_writer);
        amount -= amount_to_purchase;
        pthread_mutex_unlock(&mutex_account_write);//
        it_account_2->inc_amount_no_sleep(amount_to_purchase, atm_id);
        return true;
    }
}

void
Account::print_status()
{
    new_account_reader();
    cout << "Account " << account_num << ": Balance - " << amount << " $ , Account Password - " << password << endl;
    reader_account_leave();
}
void
Account::take_commision(int precent)
{
    pthread_mutex_lock(&mutex_account_write);
    int amount_to_take = int((amount*precent)/100);
    bank_amount += amount_to_take;
    amount -= amount_to_take;
    pthread_mutex_lock(&mutex_output_writer);
    output_file<< "Bank: commissions of "<< precent <<" % were charged, the bank gained "<< amount_to_take<<" $ from account "<< account_num<< endl;
    pthread_mutex_unlock(&mutex_output_writer);
    pthread_mutex_unlock(&mutex_account_write);
}
