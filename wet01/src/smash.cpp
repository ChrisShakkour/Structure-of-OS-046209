/*	smash.c
main file. This file contains the main function of smash
*******************************************************************/
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "commands.h"
#include "signals.h"

#include <list>

#define MAX_LINE_SIZE 80
#define MAXARGS 20

char* L_Fg_Cmd;
char lineSize[MAX_LINE_SIZE];

std::list<job> jobs; //This represents the list of jobs. Please change to a preferred type (e.g array of char*)
struct sigaction ctrlC, ctrlZ;

//**************************************************************************************
// function name: main
// Description: main function of smash. get command from user and calls command functions
//**************************************************************************************
int main(int argc, char *argv[])
{
    char cmdString[MAX_LINE_SIZE]; 	   

	
	//signal declaretions
	//NOTE: the signal handlers and the function/s that sets the handler should be found in siganls.c
	 /* add your code here */
	ctrlC.sa_handler = &ctrlCFunction;
	ctrlC.sa_flags = 0;
	
    ctrlZ.sa_handler = &ctrlZFunction;
	ctrlZ.sa_flags = 0;

	/************************************/
	//NOTE: the signal handlers and the function/s that sets the handler should be found in siganls.c
	//set your signal handlers here
	/* add your code here */    
    sigaction(SIGINT, &ctrlC, NULL);
    sigaction(SIGTSTP, &ctrlZ, NULL);

	/************************************/

	/************************************/
	// Init globals
    
	
	L_Fg_Cmd =(char*)malloc(sizeof(char)*(MAX_LINE_SIZE+1));
	if (L_Fg_Cmd == NULL) 
			exit (-1); 
	L_Fg_Cmd[0] = '\0';
	
    	while (1)
    	{
	 	printf("smash > ");
		fgets(lineSize, MAX_LINE_SIZE, stdin);
		strcpy(cmdString, lineSize);    	
		cmdString[strlen(lineSize)-1]='\0';
				
		// remove finished jobs every new command
		remove_finished_jobs(&jobs);
		
		// background command	
	 	//if(!BgCmd(lineSize, &jobs)) continue; 
		
		if(!BgCmd(lineSize, &jobs)); //continue; 		
		else // built in commands
			ExeCmd(&jobs, lineSize, cmdString);
		
		/* initialize for next line read*/
		lineSize[0]='\0';
		cmdString[0]='\0';
	}
    return 0;
}

