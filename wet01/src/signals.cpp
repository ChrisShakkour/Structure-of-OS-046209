// signals.c
// contains signal handler funtions
// contains the function/s that set the signal handlers

/*******************************************/
/* Name: handler_cntlc
   Synopsis: handle the Control-C */
#include "signals.h"

extern std::list<job> jobs;

//**************************************************************************************
// function name: main
// Description: main function of smash. get command from user and calls command functions
//**************************************************************************************
void ctrlCFunction(int dummyVar){
	// remove finished jobs 
	// before doing ctrl-c routine
	remove_finished_jobs(&jobs);
	
}


//**************************************************************************************
// function name: main
// Description: main function of smash. get command from user and calls command functions
//**************************************************************************************
void ctrlZFunction(int dummyVar){
	// remove finished jobs 
	// before doing ctrl-c routine
	remove_finished_jobs(&jobs);
	
}
