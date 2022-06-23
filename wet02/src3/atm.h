#ifndef _ATM_H
#define _ATM_H
using namespace std;

class Atm {
public:
    int id;
    Atm(int id) : id(id) {

    }
    Atm(){

    }
};

void* atm_routine(void* atm_index_p);
void reader_list_leave();
void new_list_reader();


#endif