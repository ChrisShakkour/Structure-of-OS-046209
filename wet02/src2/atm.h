#ifndef _ATM_H
#define _ATM_H
using namespace std;

#define ATM_OP_SLEEP 0.5// 1sec
#define ATM_OP_RETIRE 100000//100us

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