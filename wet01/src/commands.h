#ifndef _COMMANDS_H
#define _COMMANDS_H

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

#include <dirent.h>
#include <fcntl.h>
#include <string>
#include <sys/syscall.h>
#include <sys/stat.h>

#include <fstream>
#include <algorithm>


#define CMD_SUCCESS 0
#define CMD_ERROR 1

#define BUFFERSIZE 1024
#define MAX_LINE_SIZE 80
#define MAX_ARG 20

#define MAX_JOB_COUNT 100


typedef enum {BACKGROUND, STOPPED} jobStatus;


class job { public:
	int id;
	char command[MAX_LINE_SIZE];	
	int pid;
	jobStatus status;
	int current_time;
	int creation_time;
	
	job(int id_,
		char* command_,
		int pid_,
		jobStatus status_,
		int current_time_) {
		
		id = id_;
		strcpy(command,command_);
		pid = pid_;
		status = status_;
		current_time = current_time_;
		
		time_t cr_time = time(NULL);
		creation_time = (int)cr_time;
	};

	int life_time(){
		return (int)time(NULL) - creation_time + current_time; 
	}
};




// functions
// returns 1 on success, 
// return 1 when max job count is reached (100)
bool add_job_to_jobs_list(std::list<job>* jobsList, int pID, jobStatus status, char* cmdString , int startTime);

int ExeComp(char* lineSize);
int BgCmd(char* lineSize, std::list<job>* jobs);
int ExeCmd(std::list<job>* jobs, char* lineSize, char* cmdString);
void ExeExternal(char *args[MAX_ARG], char* cmdString, std::list<job>* jobList);

#endif

