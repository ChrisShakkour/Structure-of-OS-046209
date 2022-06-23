#ifndef _ACCOUNT_H
#define _ACCOUNT_H
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

class Account {
public:
    int account_num;
    string password;
    int amount;
    unsigned number_of_readers = 0;
    // This mutex will be used for locking  writers, and enable multiple readers
    pthread_mutex_t mutex_account_read;
    // This mutex will be used for locking  writereadrs, and enable multiple readers
    pthread_mutex_t mutex_account_read_and_write;
    // This mutex will be used for locking  writers and readers
    pthread_mutex_t mutex_account_write;

    Account(int account_num, string new_password, int amount) : account_num(account_num), amount(amount)
    {
        password.assign(new_password);
        pthread_mutex_init(&mutex_account_read, NULL);
        pthread_mutex_init(&mutex_account_write, NULL);
        pthread_mutex_init(&mutex_account_read_and_write, NULL);
    }
    ~Account()
    {
        pthread_mutex_destroy(&mutex_account_read);
        pthread_mutex_destroy(&mutex_account_write);
        pthread_mutex_destroy(&mutex_account_read_and_write);
    }
    void new_account_reader ();
    void new_account_reader_with_sleep ();
    void reader_account_leave ();

    void new_account_reader_and_writer();
    void reader_and_writer_account_write();
    void reader_and_writer_account_leave();

    bool purchase(int amount_to_purchase,int atm_id);
    void inc_amount(int inc_amount, int atm_id);
    void inc_amount_no_sleep(int inc_amount, int atm_id);
    void check_amount(int atm_id);
    bool pass_amount(int amount_to_purchase,int atm_id);
    void print_status();
    void take_commision(int precent);

};

bool compare_acc_num(const Account& a, const Account& b);



#endif