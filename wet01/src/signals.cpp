// signals.c
// contains signal handler funtions
// contains the function/s that set the signal handlers

/*******************************************/
/* Name: handler_cntlc
   Synopsis: handle the Control-C */
#include "signals.h"

extern std::list<job> jobs;
extern bool waite_for_fg;
extern int fg_pid;
extern char fg_cmdString[MAX_LINE_SIZE];


//**************************************************************************************
// function name: ctrlCFunction
// Description: kills a running job in the fg.
//**************************************************************************************
void ctrlCFunction(int dummyVar){
	// remove finished jobs 
	// before doing ctrl-c routine
	remove_finished_jobs(&jobs);
	std::cout << "smash: caught ctrl-c" << std::endl;
	if(waite_for_fg){
		if(kill(fg_pid, SIGKILL)==-1) // send SIGKILL signal for stopped jobs
			perror("kill failed");
		else std::cout << "smash: proccess " << fg_pid << " was killed" << std::endl;		
	}
}


//**************************************************************************************
// function name: ctrlZFunction
// Description: stops a running job in the fg.
//**************************************************************************************
void ctrlZFunction(int dummyVar){
	// remove finished jobs 
	// before doing ctrl-c routine
	remove_finished_jobs(&jobs);
	std::cout << "smash: caught ctrl-z" << std::endl;
	if(waite_for_fg){
		if(add_job_to_jobs_list(&jobs, fg_pid, STOPPED, fg_cmdString, 0))
			std::cout << "smash error: reached job list limit (100 jobs)" << std::endl;
		else {
			if(kill(fg_pid, SIGSTOP)==-1) // send SIGSTOP signal for stopped jobs
				perror("kill failed");
			else std::cout << "smash: proccess " << fg_pid << " was stopped" << std::endl;
		}
	}
	return;
}
