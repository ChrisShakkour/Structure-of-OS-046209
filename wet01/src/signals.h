#ifndef _SIGS_H
#define _SIGS_H

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "jobs.h"


void ctrlCFunction(int);
void ctrlZFunction(int);

#endif

