//		commands.c
//********************************************
#include "commands.h"

#define PID_KILLED -1
#define STATUS_STP 0
#define STATUS_RUN 1
#define STATUS_FIN 2
#define HISTORY_MAX 50
#define SIGNUM_MAX 500
#define JOB_NUM_MAX 500
#define MAX_BUFFER_SIZE 1024

#define SIGNUM_RUN 18
#define SIGNUM_STP_1 6
#define SIGNUM_STP_2 19
#define SIGNUM_STP_3 20
#define SIGNUM_FIN_1 9
#define SIGNUM_FIN_2 15
#define MAX_ARGS 20

///////////////////////////////////////////////

// hist_q is a queue of jobs names
queue<string> hist_q;

int fg_pid;
bool fg_process = false;
int job_id = 0;
char prev_dir[MAX_LINE_SIZE];
bool cd_orig = true;


///////////////////////////////////////////////
// Helper functions

// History - add new job name to history queue
void history_control(char* cmd){
	char* new_cmd = cmd;
	if (HISTORY_MAX == hist_q.size())
		hist_q.pop();
	hist_q.push(new_cmd);
}

///////////////////////////////////////////////

// Add new job to the jobs list
void add_to_jobs(int pid, int status, int time, string job_name, list<job>* jobs)
{
	if (status == STATUS_FIN)
	{
		return;
	}
	job_id++;
	job new_job(job_id, pid, job_name, status, time);
	jobs->push_back(new_job);
}

///////////////////////////////////////////////

// cd - without "-"
int reg_cd_cmd(char* dest_path, char* current_path){
	if (!chdir(dest_path))
	{
		cd_orig = false;
		strcpy(prev_dir, current_path);
		return 0;
	}
	else
	{
		cout << "smash error: > " << dest_path << " - No such file or directory" << endl;
		return 1;
	}
}

///////////////////////////////////////////////

// cd - with "-"
int last_cd_cmd(char* dest_path,char* current_path) {
	if (cd_orig == true) 
	{
		cd_orig = false;
		perror("smash > no directory change\n");
		return 1;
	}
	else if (chdir(prev_dir))
	{
		perror("smash > cd cmd failed\n");
		return 1;
	}
	else
	{
		cout << prev_dir << "\n";
		strcpy(prev_dir, current_path);
		return 0;
	}
	
}


///////////////////////////////////////////////

// kill helper function
int exec_kill(const char* job_to_kill, int signum_as_int, list<job>* jobs) 
{
	list<job>::iterator iter;
	string string_err = "smash error: > kill ";
	iter = jobs->begin();
	while(iter != jobs->end()) 
	{
		if (atoi(job_to_kill) == iter->id)
		{
			int curr_pid = iter->pid;
			if (kill(curr_pid, signum_as_int) == -1) {
				// create error msg
				string error_msg = " - cannot send signal";
				string_err += job_to_kill;
				string_err += error_msg;
				string string_err_str = string_err.c_str();
				perror(string_err_str.c_str());
				return 1;
			}
			if (signum_as_int == SIGNUM_RUN) 
			{
				iter->status=STATUS_RUN;
			}
			if (signum_as_int == SIGNUM_STP_1 || signum_as_int == SIGNUM_STP_2 || signum_as_int == SIGNUM_STP_3) 
			{
				iter->status=STATUS_STP;
			}
			if (signum_as_int == SIGNUM_FIN_1 || signum_as_int == SIGNUM_FIN_2)
			{
				iter->status=STATUS_FIN;
			}
			return 0;
		}
		iter++;
	}
	// create error msg
	string_err += job_to_kill;
	string error_msg = " - job does not exist";
	string_err += error_msg.c_str();
	perror(string_err.c_str());
	return 1;
}

///////////////////////////////////////////////

// fg with arg helper function
int fg_with_arg(list<job>* jobs, char* arg_1, char* arg_2) 
{
	int process_done = 0;
	bool success = false;
	if ((arg_1 != NULL) && (arg_2 == NULL)) 
	{
		list<job>::iterator iter;
		char cmd_array[JOB_NUM_MAX];
		int job_to_kill_num = 1;
		strcpy(cmd_array, arg_1);
		job_to_kill_num = atoi(cmd_array);
		
		for (iter = jobs->begin(); iter != jobs->end(); ++iter) 
		{
			if (job_to_kill_num == iter->id)
			{
				cout << iter->job_name << endl;
				int kill_check = kill(iter->pid, SIGCONT);
				if (kill_check == -1)
				{
					perror("error - fg failed");
					return 1;
				}
				success = true;
				fg_process = 1;
				fg_pid = iter->pid;
				process_done = waitpid(iter->pid, NULL, WUNTRACED);
				if (process_done > 0) 
				{
					jobs->erase(iter);
					break;
				}
				fg_process = 0;
				fg_pid = -1;
			}
		}
		if (!success) 
		{
			perror("error - fg failed");
			return 1;
		}
	}
	else
	{
		perror("error - fg failed");
		return 1;
	}
	return 0;
}

///////////////////////////////////////////////

// fg without arg helper function
int fg_without_arg(list<job>* jobs, char* arg_1) 
{
	int process_done = 0;
	if (arg_1 == NULL)
	{
		bool kill_success = false;
		list<job>::iterator iter = jobs->begin();
		int upcoming_pid = iter->pid;
		int upcoming_id = iter->id;
		string upcoming_name = iter->job_name;
		
		for (iter = jobs->begin(); iter != jobs->end(); ++iter) 
		{
			if (iter->id > upcoming_id)
			{
				upcoming_id = iter->id;
				upcoming_pid = iter->pid;
				upcoming_name = iter->job_name;
			}
		}	

		
		if (upcoming_pid == 0) 
		{
			perror("error - fg failed, no jobs");
			return 1;
		}
		
		int kill_check = kill(upcoming_pid, SIGCONT);
		if (kill_check == -1)
		{
			perror("error - fg failed");
			return 1;
		}
		
		else
		{
			fg_process = true;
			kill_success = true;
			fg_pid = upcoming_pid;
			cout << upcoming_name.c_str() << endl;
			process_done = waitpid(upcoming_pid, NULL, WUNTRACED);
			if (process_done > 0)
			{
				for (iter = jobs->begin(); iter != jobs->end(); ++iter) 
				{
					if (upcoming_pid == iter->pid)
					{
						jobs->erase(iter);
						break;
					}
				}
			}
		}
		
		fg_pid = PID_KILLED;
		fg_process = false;
		if (!kill_success)
		{
			perror("error - fg failed");
			return 1;
		}
		return 0;
	}
	perror("error - fg failed");
	return 1;
}

///////////////////////////////////////////////

// bg with arg helper function
int bg_with_arg(list<job>* jobs, char* arg_1, char* arg_2) 
{
	if ((arg_1 != NULL) && (arg_2 == NULL)) 
	{
		bool success = false;
		char cmd_array[JOB_NUM_MAX];
		strcpy(cmd_array, arg_1);
		int current_job_number = 1;	
		current_job_number = atoi(cmd_array);
		list<job>::iterator iter;
		for (iter = jobs->begin(); iter != jobs->end(); ++iter) 
		{
			int iter_id = iter->id;
			if (current_job_number == iter_id)
			{
				if (iter->status != STATUS_STP)
				{
					perror("error - bg failed, proccess isn't stopped");
					return 1;
				}
				cout << iter->job_name << endl;
				int kill_check = kill(iter->pid, SIGCONT);
				if (kill_check == -1)
				{
					perror("error - bg failed");
					return 1;
				}
				success = true;
				iter->status=STATUS_RUN;
				return 0;
			}
		}
		if (success == 0)
		{
			perror("error - bg failed");
			return 1;
		}
	}
	perror("error - bg failed");
	return 1;
}

///////////////////////////////////////////////

// bg without arg helper function
int bg_without_arg(list<job>* jobs, char* arg_1) 
{
	if (arg_1 == NULL) 
	{
		int last_stopped_id = 0;
		bool success = false;
		list<job>::iterator iter;
		for (iter = jobs->begin(); iter != jobs->end(); ++iter) 
		{
			if (iter->status == STATUS_STP )
			{
				last_stopped_id = iter->id;
			}	
		}
		
		if (!last_stopped_id)
		{
			perror("error - bg failed, no jobs"); 
			return 1; 
		}

		list<job>::iterator iter_2;
		for (iter_2 = jobs->begin(); iter_2 != jobs->end(); ++iter_2)
		{
			if (iter_2->id == last_stopped_id)
			{
				int kill_check = kill(iter_2->pid, SIGCONT);
				if (kill_check == -1)
				{
					perror("error - bg failed"); 
					return 1;
				}
				success = true;
				iter_2->status=STATUS_RUN;
				cout << iter_2->job_name << endl;
				return 0;
			}
		}
		if (success == 0)
		{
			perror("error - bg failed, no jobs"); 
			return 1;
		}
	}
	perror("error - bg failed");
	return 1;
}

///////////////////////////////////////////////

// ExeCmd function - all built in functions
int ExeCmd(list<job>* jobs, char* lineSize, char* cmdString)
{
	bool illegal_cmd = false;
	char* cmd;
	char* args[MAX_ARG];
	char delmtr[] = " \t\n";
	int i = 0, num_arg = 0;
	cmd = strtok(lineSize, delmtr);
	if (cmd == NULL)
		return 0;
	args[0] = cmd;
	for (i = 1; i < MAX_ARG; i++)
	{
		args[i] = strtok(NULL, delmtr);
		if (args[i] != NULL)
			num_arg++;

	}
	
	/*************************************************/
	if (!strcmp(cmd, "cd"))
	{
		if (num_arg == 1) 
		{
			char current_path[MAX_LINE_SIZE];
			char dest_path[MAX_LINE_SIZE];
			int buffer_size = sizeof(current_path);
			strcpy(dest_path, args[1]);
			getcwd(current_path, buffer_size);
			// in case of cd -
			if (strcmp("-", dest_path)) 
			{
				// call helper function
				return reg_cd_cmd(dest_path, current_path);
			}
			// in case of cd
			else
			{
				// call helper function
				return last_cd_cmd(dest_path, current_path);
			}
		}
		illegal_cmd = true;
	}
	
	/*************************************************/
	else if (!strcmp(cmd, "pwd"))
	{
		if (!num_arg) 
		{
			char current_path[MAX_LINE_SIZE];
			int buffer_size = sizeof(current_path);
			getcwd(current_path, buffer_size);
			cout << current_path << endl;
			return 0;
		}
		illegal_cmd = true;
	}
	
	/*************************************************/
	else if (!strcmp(cmd, "history"))
	{
		if (!num_arg) {
			for (unsigned int i = 0; i < hist_q.size(); i++)
			{
				string line = hist_q.front();
				cout << line << '\n';
				hist_q.pop();
				hist_q.push(line);
			}
			return 0;
		}
		illegal_cmd = true;
	}
	
	/*************************************************/
	else if (!strcmp(cmd, "jobs"))
	{
		if (!num_arg) 
		{
			list<job>::iterator iter;
			iter = jobs->begin();
			while (iter != jobs->end()) 
			{
				cout << "[" << iter->id << "] " << iter->job_name << " : " << iter->pid << " " << iter->life_time() << " secs";
				if (iter->status != STATUS_STP) 
				{
					cout << endl;
				}
				else 
				{
					cout << " (Stopped)" << endl;
				}
				iter++;
			}
			return 0;
		}
		illegal_cmd = true;
	}
	
	/*************************************************/
	else if (!strcmp(cmd, "kill"))
	{
		if (num_arg == 2) { 
			char signum_number[SIGNUM_MAX]; 
			strcpy(signum_number, args[1]);	
			char* signum_number_shifted = signum_number + 1;
			int signum_as_int = atoi(signum_number_shifted);
			char job_to_kill[JOB_NUM_MAX]; 
			strcpy(job_to_kill, args[2]);
			//int job_to_kill_num = atoi(job_to_kill);
			// call helper function
			return exec_kill(job_to_kill, signum_as_int, jobs); 
		}
		illegal_cmd = true;
	}
	
	/*************************************************/
	else if (!strcmp(cmd, "showpid"))
	{
		if (!num_arg) 
		{
			unsigned int curr_pid = getpid();
			cout << "smash pid is " << curr_pid << endl;
			return 0;
		}
		illegal_cmd = true;
	}
	
	/*************************************************/
	else if (!strcmp(cmd, "fg"))
	{		
		switch(num_arg) 
		{
			case 0: //num_arg = 0
				return fg_without_arg(jobs, args[1]);
			case 1: //num_arg = 1
				return fg_with_arg(jobs, args[1], args[2]);
			default: //num_arg > 1
				illegal_cmd = true;
				perror("error - fg failed");
				return 1;
		}
	}
	
	/*************************************************/
	else if (!strcmp(cmd, "bg"))
	{		
		switch(num_arg) 
		{
			case 0: //num_arg = 0
				return bg_without_arg(jobs, args[1]);
			case 1: //num_arg = 1
				return bg_with_arg(jobs, args[1], args[2]);
			default: //num_arg > 1
				illegal_cmd = true;
				perror("error - fg failed");
				return 1;
		}
	}
	
	/*************************************************/
	else if (!strcmp(cmd, "quit"))
	{
		// reuglar quit
		if (args[1] == NULL) 
		{
			int curr_pid = getpid();
			if (kill(curr_pid, SIGNUM_FIN_1) == -1) 
			{
				perror("regular quit fail");
				return 1;
			}
		}
		
		// quit kill
		const char* str_kill = "kill";
		if (!strcmp(str_kill, args[1])) 
		{
			list<job>::iterator iter;
			for (iter = jobs->begin(); iter != jobs->end(); ++iter)
			{
				int curr_id = iter->id;
				int curr_pid = iter->pid;
				string curr_name = iter->job_name;
				const char* final_send_1 = " - Sending SIGTERM... ";

				cout << "[" << curr_id << "] " << curr_name << final_send_1 << endl;
				int kill_check = kill(curr_pid, SIGNUM_FIN_2);
				if (kill_check == -1)
				{
					perror("error - quit kill fail");
					return 1;
				}
				int start_time = (int)time(NULL);
				while (1) {
					int passed_time = (int)time(NULL) - start_time;
					int process_dead = waitpid(curr_pid, NULL, WNOHANG);
					if (process_dead && (passed_time<5))
					{
						cout << "Done." << endl;
						break;
					}
					else if (passed_time > 5)
					{
						cout << "(5 sec passed) Sending SIGKILL... Done." << endl;
						if (kill(curr_pid, SIGNUM_FIN_1) == -1)
							{
								perror("error - quit kill fail");
								return 1;
							}
						break;
					}
				}
			}
			if (kill(getpid(), SIGNUM_FIN_1) == -1)
			{
				perror("quit kill fail");
				return 1;
			}
			return 0;
		
		illegal_cmd = true;
		perror("quit kill fail");
		return 1;
		}
	}

	
/*************************************************/
	else if (!strcmp(cmd, "diff"))
	{
		if (num_arg == 2) 
		{
			  unsigned char file_1_buffer[MAX_BUFFER_SIZE];
			  unsigned char file_2_buffer[MAX_BUFFER_SIZE];

			  ifstream file_1(args[1], ios::in | ios::binary);
			  if(!file_1) 
			  {
			    perror("error - can't open file 1");
			    return 1;
			  }
			  
			  ifstream file_2(args[2], ios::in | ios::binary);
			  if(!file_2) 
			  {
				perror("error - can't open file 2");
			    return 1;
			   }

			  do 
			  {
				  file_1.read((char *) file_1_buffer, sizeof file_1_buffer);
				  file_2.read((char *) file_2_buffer, sizeof file_2_buffer);
				  if (file_1.gcount() != file_2.gcount()) 
				  {
					  cout << "1" << endl;
					  file_1.close();
					  file_2.close();
					  return 0;
				  }

				  // compare contents of buffers
				  for (int i=0; i<file_1.gcount(); i++) 
				  {
					  if (file_1_buffer[i] != file_2_buffer[i]) 
					  {
						  cout << "1" << endl;
						  file_1.close();
						  file_2.close();
						  return 0;
					  }
				  }
			  } while (!file_1.eof() && !file_2.eof());

			  cout << "0" << endl;;
			  file_1.close();
			  file_2.close();
			  return 0;
		}
		illegal_cmd = true;
	}
	
/*************************************************/
	// external command
	else
	{
		ExeExternal(args, cmdString, jobs);
		return 0;
	}
	if (illegal_cmd == true)
	{
	 	printf("smash error: > \"%s\"\n", cmdString);
		return 1;
	}
	return 0;
}

//**************************************************************************************
// function name: ExeExternal
// Description: executes external command
// Parameters: external command arguments, external command string
// Returns: void
//**************************************************************************************
void ExeExternal(char* args[MAX_ARG], char* cmdString, list<job>* jobs)
{
	int pID;
	switch (pID = fork())
	{

	case -1:
		perror("fail");
		break;

	case 0: //child	
		if (setpgrp() != -1)
		{
			// execute cmd on the outside shell
			execvp(args[0], args);
		}	
		perror("fail");
		exit(1);

	default: //father
		fg_process = true;
		fg_pid = pID;
		waitpid(pID, NULL, WUNTRACED | WCONTINUED); 
		fg_pid = PID_KILLED;
		break;
	}
}

//**************************************************************************************
// function name: BgCmd
// Description: if command is in background, insert the command to jobs
// Parameters: command string, pointer to jobs
// Returns: 0- BG command -1- if not
//**************************************************************************************

int BgCmd(char* lineSize, list<job>* jobs)
{
	char* Command;
	char delmtr[] = " \t\n";
	char* args[MAX_ARGS];
	int i = 0, num_arg = 0;
	char cmd_buffer[MAX_LINE_SIZE];
	if (lineSize[strlen(lineSize) - 2] == '&')
	{
		lineSize[strlen(lineSize) - 2] = '\0';
		strcpy(cmd_buffer, lineSize); 
		Command = strtok(lineSize, delmtr);
		
		if (Command == NULL) 
		{
			perror("error");
			return 1;
		}
		
		else
		{
			args[0] = Command;
		}

		for (i = 1; i < MAX_ARGS; i++)
		{
			args[i] = strtok(NULL, delmtr);
			if (args[i] != NULL)
				num_arg++;
		}
		int pID = fork();
		
		if (pID == 0)
		{
			setpgrp();
			if (execvp(args[0], args) == -1) {
				perror("error");
				return 1;
			}
		}
		
		else if (pID > 0)
		{
			fg_process = false;
			fg_pid = pID;
			add_to_jobs(pID, STATUS_RUN, 0, args[0], jobs);
			return 0;
		}
		
		else
		{
			perror("error");
			return 1;
		}
	}
	
	return -1;
}
