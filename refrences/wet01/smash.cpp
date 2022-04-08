/*	smash.c
main file. This file contains the main function of smash
*******************************************************************/
#include <unistd.h> 
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <cstdio>
#include <iostream>
#include <queue>         
#include <list>
#include <fstream>
#include <dirent.h>
#include <fcntl.h>
#include <string>
#include <sys/syscall.h>
#include <sys/stat.h>

#include "commands.h"
#include "signals.h"
 
#define STATUS_STP 0
#define STATUS_RUN 1
#define STATUS_FIN 2
#define MAX_LINE_SIZE 80
#define MAXARGS 20

list<job> jobs;
char lineSize[MAX_LINE_SIZE];

struct sigaction ctrl_c_action;
struct sigaction ctrl_z_action;

///////////////////////////////////////////////

// Helper function - remove finished job
void remove_job_from_jobs(list<job>* jobs) {
	if (!(jobs->size())) {return;}
 	list<job>::iterator iter;
 	for (iter = jobs->begin(); iter != jobs->end(); ++iter)
 	{
 		if (waitpid(iter->pid, NULL, WNOHANG)!=0)
		{
			jobs->erase(iter);
			return;
		}
 		if (iter->status == STATUS_FIN)
 		{
 			jobs->erase(iter);
 			return;
 		}

 	}
 }

//**************************************************************************************
// function name: main
// Description: main function of smash. get command from user and calls command functions
//**************************************************************************************
int main(int argc, char* argv[])
{
	char cmdString[MAX_LINE_SIZE];

	//signal declaretions

	ctrl_c_action.sa_handler = &ctrl_c_func;
	ctrl_c_action.sa_flags = 0;

	ctrl_z_action.sa_handler = &ctrl_z_func;
	ctrl_z_action.sa_flags = 0;

	sigaction(SIGINT, &ctrl_c_action, NULL);
	sigaction(SIGTSTP, &ctrl_z_action, NULL);

	while (1)
	{
		printf("smash > ");
		fgets(lineSize, MAX_LINE_SIZE, stdin);
		strcpy(cmdString, lineSize);
		cmdString[strlen(lineSize) - 1] = '\0';

		// every iteration we remove finished job
		remove_job_from_jobs(&jobs);
		history_control(cmdString);

		// background command	
		if (!BgCmd(lineSize, &jobs)) continue;
		// built in commands
		ExeCmd(&jobs, lineSize, cmdString);
		/* initialize for next line read*/
		lineSize[0] = '\0';
		cmdString[0] = '\0';
	}
	jobs.clear();
	return 0;

}
