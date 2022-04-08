#ifndef _COMMANDS_H
#define _COMMANDS_H

#define BUFFERSIZE 1024
#define MAX_LINE_SIZE 80
#define MAX_ARG 20 
#define STATUS_STP 0
#define STATUS_RUN 1
#define STATUS_FIN 2

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

using namespace std;
extern int fg_pid;

class job {
	public:
		int id;
		int pid;
		string job_name;
		int current_time;
		int creation_time;
		int status;

		job(int id_, int pid_, string job_name_, int status_, int current_time_) : id(id_), pid(pid_), job_name(job_name_), current_time(current_time_), status(status_)
		{
			time_t cr_time = time(NULL);
			creation_time = (int)cr_time;
		};

		int life_time() 
		{ 
			return (int)time(NULL) - creation_time + current_time; 
		}

};












/////////////


void add_to_jobs(int pid, int status, int time, string job_name, list<job>* jobs);
void history_control(char* cmd);
int BgCmd(char* lineSize, list<job>* jobs);
int ExeCmd(list<job>* jobs, char* lineSize, char* cmdString);
void ExeExternal(char* args[MAX_ARG], char* cmdString, list<job>* jobs);

#endif
