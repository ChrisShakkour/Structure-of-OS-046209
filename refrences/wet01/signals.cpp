// signals.c
// contains signal handler funtions
// contains the function/s that set the signal handlers

/*******************************************/
/* Name: handler_cntlc
   Synopsis: handle the Control-C */

#include "signals.h"

#include "commands.h"

using namespace std;

#define PID_KILLED -1
#define STATUS_STP 0
#define STATUS_RUN 1
#define STATUS_FIN 2

///////////////////////////////////////////////

// Externals vars
extern bool fg_process;
extern int fg_pid;
extern list<job> jobs;
extern char lineSize[MAX_LINE_SIZE];

///////////////////////////////////////////////

// Helper function - checks if a process is running in fg
void fg_check(char* cmd, char* fisrt_arg)
{
	if (!strcmp(cmd, "fg")) {
		int id;
		list<job>::iterator iter;
		// get to the last job
		for (iter = jobs.begin(); iter != jobs.end(); ++iter) {}
		id = iter->id;
		// find stopped jobs
		for (iter = jobs.begin(); iter != jobs.end(); ++iter) {
			if (id == iter->id)
			{
				iter->status=STATUS_STP;
				return;
			}
		}
	}

	else
	{
		add_to_jobs(fg_pid, STATUS_STP, 0, fisrt_arg, &jobs);
		fg_process = false;
		fg_pid = PID_KILLED;
	}
}

///////////////////////////////////////////////

void ctrl_z_func(int not_in_use) 
{
	char* cmd;
	char* args[MAX_ARG];
	char delmtr[] = " \t\n";
	cmd = strtok(lineSize, delmtr);
	if (cmd == NULL)
	{
		return;
	}
	args[0] = cmd;
	int i;
	for (i = 1; i < MAX_ARG; i++)
	{
		args[i] = strtok(NULL, delmtr);
	}
	// sending signal to stop process
	if ((fg_pid != PID_KILLED) && (fg_process))
	{
		if (kill(fg_pid, SIGTSTP) != -1)
		{
			// call helper function
			fg_check(cmd, args[0]);
			cout << "\n";
			return;
		}
		else
		{
			perror("Process stop failed\n");
			return;
		}
	}
	else
	{
		cout << "\n";
		return;
	}
}

///////////////////////////////////////////////

void ctrl_c_func(int not_in_use) {
	int kill_succes = -1;
	if (fg_pid == PID_KILLED) 
	{
		return;
	}
	else
	{
		//sending signal to kill process
		if (fg_process) 
		{
			kill_succes = kill(fg_pid, SIGINT);
			if (kill_succes!=0) 
			{
				perror("Process kill failed\n");
			}
			else 
			{
				 fg_pid = PID_KILLED;
				 fg_process = false;
			}
		}
		cout << "\n";
		return;
	}
}

