#ifndef BANK_H
#define BANK_H

// INCLUDES
#include <stdio.h>
#include <map>
#include <vector>         
#include <iostream>
#include <fstream>
#include "atm.h"
#include "account.h"


// NAMESPACE
using namespace std;


// DEFINES
#define PRINT_PERIOD 500000      // 0.5 second
#define COMMISION_PERIOD 3000000 // 3 seconds


// FUNCTIONS
void* bank_commission_routine(void* bank);
void* atm_routine(void* atm);
void* bank_status_routine(void* bank);
int main(int argc, char* argv[]);


#endif
