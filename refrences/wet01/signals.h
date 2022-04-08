#ifndef _SIGS_H
#define _SIGS_H
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <iostream>
#include <queue>         
#include <list>

// int not_in_use just for sending function to sa_handler
void ctrl_c_func(int not_in_use);
void ctrl_z_func(int not_in_use);

#endif



