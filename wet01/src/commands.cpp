//		commands.c
//********************************************
#include "commands.h"

#define KILLSIG_9 9
#define KILLSIG_15 15
//********************************************

// cd saved variables.


char prev_path[MAX_LINE_SIZE] = "";
bool bg_dont_wait_flag = false;
int jobIndex=0;



// function name: ExeCmd
// Description: interperts and executes built-in commands
// Parameters: pointer to jobs, command string
// Returns: 0 - success,1 - failure
//**************************************************************************************
int ExeCmd(std::list<job>* jobs, char* lineSize, char* cmdString)
{
	char* cmd; 
	char* args[MAX_ARG];
	char pwd[MAX_LINE_SIZE];
	const char* delimiters = " \t\n";  
	int i = 0, num_arg = 0;
	bool illegal_cmd = false;
    	cmd = strtok(lineSize, delimiters);
	if (cmd == NULL)
		return 0; 
   	args[0] = cmd;
	for (i=1; i<MAX_ARG; i++)
	{
		args[i] = strtok(NULL, delimiters); 
		if (args[i] != NULL)
		{ 
			num_arg++;
		}
	}
	bg_dont_wait_flag = false;
/*************************************************/
// Built in Commands PLEASE NOTE NOT ALL REQUIRED
// ARE IN THIS CHAIN OF IF COMMANDS. PLEASE ADD
// MORE IF STATEMENTS AS REQUIRED
/*************************************************/
	if (!strcmp(cmd, "cd") ) 
	{
		if (num_arg == 1) 
		{
			char current_path[MAX_LINE_SIZE];
			char dest_path[MAX_LINE_SIZE];
			int buffer_size = sizeof(current_path);
			strcpy(dest_path, args[1]);
			getcwd(current_path, buffer_size);
			
			/* if sestination path is "-" */
			if (!strcmp("-", dest_path)){
				if(!strcmp("", prev_path)){
					std::cout << "smash error: cd: OLDPWD not set" << std::endl;
					return CMD_ERROR;
				}
				if(!chdir(prev_path))
					strcpy(prev_path, current_path);
				else return CMD_ERROR;
			} else {
				if(!chdir(dest_path))
					strcpy(prev_path, current_path);
			}
		}
		else if (num_arg>1){
			std::cout << "smash error: cd: too many arguments"<< std::endl;
			return CMD_ERROR;
		}
		return CMD_SUCCESS;
	}
	/*************************************************/
	else if (!strcmp(cmd, "pwd")) 
	{
		char current_path[MAX_LINE_SIZE];
		int buffer_size = sizeof(current_path);
		getcwd(current_path, buffer_size);
		std::cout << current_path << std::endl;
		return CMD_SUCCESS;
	}
	
	/*************************************************/
	else if (!strcmp(cmd, "diff"))
	{
		if(num_arg !=2){
			std::cout<< "smash error: diff: invalid arguments" << std::endl;
			return CMD_ERROR;
		}
		unsigned char file_1_buffer[BUFFERSIZE];
		unsigned char file_2_buffer[BUFFERSIZE];

		/*open file 1*/
		std::ifstream file_1(args[1], std::ifstream::ate | std::ifstream::binary);
		/*open file 2*/
		std::ifstream file_2(args[2], std::ifstream::ate | std::ifstream::binary);
		
		/* if files are invalid */
		if((!file_1) || (!file_2)) {
			std::cout<< "1" << std::endl;
			file_1.close();
			file_2.close();
			return CMD_SUCCESS;
		}
		
		/* if files differ in size */
		const std::ifstream::pos_type fileSize = file_1.tellg();
		if(fileSize != file_2.tellg()){
			std::cout<< "1" << std::endl;
			file_1.close();
			file_2.close();
			return CMD_SUCCESS;
		}
		
		/* compare content */
		file_1.seekg(0);
		file_2.seekg(0);
		std::istreambuf_iterator<char> begin_1(file_1);
		std::istreambuf_iterator<char> begin_2(file_2);
		if(std::equal(begin_1, std::istreambuf_iterator<char>(), begin_2)){
			std::cout<< "0" << std::endl;
			file_1.close();
			file_2.close();
			return CMD_SUCCESS;
		}
		return CMD_SUCCESS;
	}
	/*************************************************/
	
	else if (!strcmp(cmd, "jobs")) 
	{			
		std::list<job>::iterator it;
		it = jobs->begin();
		while (it != jobs->end()) 
		{
			std::cout << "[" << it->jobid << "] " << it->command << " : " << it->pid << " " << 0 << " secs";
			if (it->status != STOPPED) 
				std::cout << std::endl;
			else
				std::cout << " (stopped)" << std::endl;
			it++;
		}
		return CMD_SUCCESS;
	}
	/*************************************************/
	else if (!strcmp(cmd, "showpid")) 
	{
		unsigned int pid = getpid();
		std::cout << "smash pid is " << pid << std::endl;
		return CMD_SUCCESS;
	}
	/*************************************************/
	else if (!strcmp(cmd, "fg")) 
	{
		// if more than 1 argument is given
		if(num_arg > 1){
			std::cout<< "smash error: fg: invalid arguments" << std::endl;
			return CMD_ERROR;
		}
		
		std::list<job>::iterator it;
		int pid;
		
		// if no job id is given
		if(args[1]==NULL){		
			// and list is empty then:
			if(!jobs->size()){
				std::cout<< "smash error: fg: jobs list is empty" << std::endl;
				return CMD_ERROR;
			}
			// and list is not empty then:
			// remove last pushed job and 
			// wait till job is done.
			else {
				it = --(jobs->end());
				pid = it->pid;
				std::cout << it->command << " : " << it->pid << std::endl;
				jobs->erase(it);							// remove from job list
				kill(pid, SIGCONT);       					// send sigcont signal for stopped jobs
				waitpid(pid, NULL, WUNTRACED | WCONTINUED); // untraced release.				
				return CMD_SUCCESS;
			}
		}
	
		// search for job in jobsList, 
		// remove and wait till done
		it = jobs->begin();
		while (it != jobs->end()) 
		{
			if(it->jobid == std::atoi(args[1])){
				pid = it->pid;
				std::cout << it->command << " : " << it->pid << std::endl;
				it = jobs->erase(it); 						// remove from job list
				kill(pid, SIGCONT);    						// send sigcont signal for stopped jobs
				waitpid(pid, NULL, WUNTRACED | WCONTINUED); // wait untill done.
				return CMD_SUCCESS;
			}
			it++;
		}
		std::cout << "smash error: fg: job-id " << args[1] << " does not exist" << std::endl;
		return CMD_SUCCESS;
	} 
	/*************************************************/
	else if (!strcmp(cmd, "bg")) 
	{
		// if more than 1 argument is given
		if(num_arg > 1){
			std::cout<< "smash error: bg: invalid arguments" << std::endl;
			return CMD_ERROR;
		}
		
		std::list<job>::reverse_iterator it;
		int pid;
		
		// if no job id is given
		if(args[1]==NULL){		
			// and list is empty then:
			if(!jobs->size()){
				std::cout<< "smash error: bg: jobs list is empty" << std::endl;
				return CMD_ERROR;
			}
			// and list is not empty then:
			// remove last pushed job that is stopped
			else {		
				it = jobs->rbegin();
				while (it != jobs->rend()){
					if(it->status == STOPPED){
						pid = it->pid;
						it->status = BACKGROUND;					// remove from job list
						std::cout << it->command << " : " << it->pid << std::endl;
						kill(pid, SIGCONT);       					// send sigcont signal for stopped jobs
						return CMD_SUCCESS;
					}
					else{
						std::cout << "smash error: bg: there are no stopped jobs to resume" << std::endl;													
						return CMD_SUCCESS;
					}
					it++;
				}		
			}
		}
		// search for job in jobsList, 
		// send continue signal
		// iterate backwards from last element
		it = jobs->rbegin();
		while (it != jobs->rend()){
			// if job id matched
			if(it->jobid == std::atoi(args[1])){
				// check if status stopped
				if(it->status == STOPPED){
					pid = it->pid;
					it->status = BACKGROUND;					// remove from job list
					std::cout << it->command << " : " << it->pid << std::endl;
					kill(pid, SIGCONT);       					// send sigcont signal for stopped jobs
					return CMD_SUCCESS;
				}
				else{
					std::cout << "smash error: bg: job-id " << it->pid << " is already running in the background" << std::endl;													
					return CMD_SUCCESS;
				}
			}
			it++;
		}				
		std::cout << "smash error: bg: job-id " << args[1] << " does not exist" << std::endl;
		return CMD_SUCCESS;
	}
	/*************************************************/
	else if (!strcmp(cmd, "quit"))
	{
		// performing standard quit
		if (arg[1] == NULL)
		{
		    int sig_pid = getpid();
		    if (kill(sig_pid, KILLSIG_9) == -1)
		    {
			perror("standard quit fail");
			return 1;
		    }
		}

		//performing kill quit
		const char* kill_string = "kill";
		if (!strcmp(kill_string, arg[1])) {
		    list<job>::iterator i;
		    for (i = jobs->begin(); i != jobs->end(); ++i) {
			int job_id = i->jobid;
			int job_pid = i->pid;
			string job_name = i->name;
			const char *send_str = " - Sending SIGTERM... ";

			cout << "[" << job_id << "]" << job_name << send_str << endl;
			if (kill(job_pid, KILLSIG_15)) {
			    perror("error - quit kill fail");
			    return 1;
			}
			int start = (int) time(NULL);
			while (1) {
			    int time_wasted = (int) time(NULL) - start;
			    int process_ended = waitpid(job_pid, NULL, WNOHANG);
			    if (process_ended && (time_wasted < 5)) {
				cout << "Done." << endl;
				break;
			    } else if (time_wasted > 5) {
				cout << "(5 sec passed) Sending SIGKILL... Done." << endl;
				if (kill(job_pid, KILLSIG_9 == -1)) {
				    perror("error - quit kill fail");
				    return 1;
				}
				break;
			    }
			}
		    }
		    if (kill(getpid(), KILLSIG_9) == -1) {
			perror("quit kill fail");
			return 1;
		    }
		    return 0;
		}
	}
	/*************************************************/
	else if (!strcmp(cmd, "kill"))
	{
		return CMD_SUCCESS;
	}
	/*************************************************/
	else // external command
	{
 		ExeExternal(args, cmdString, jobs);
	 	return CMD_SUCCESS;
	}
	if (illegal_cmd == true)
	{
		printf("smash error:  \"%s\"\n", cmdString);
		return CMD_ERROR;
	}
    return 0;
}
//**************************************************************************************
// function name: ExeExternal
// Description: executes external command
// Parameters: external command arguments, external command string
// Returns: void
//**************************************************************************************
void ExeExternal(char *args[MAX_ARG], char* cmdString, std::list<job>* jobsList)
{	
	int pID;
    	switch(pID = fork()) 
	{
    		case -1: 
				perror("fork");
				break;
					
        	case 0 :
                // Child Process
               	if(setpgrp()== -1){
               		perror("fork child proccess failed");
               		exit(1);
               	}
			    execvp(args[0], args);	    
			    perror("exec failed");
				break;
			
			default:
                
				if(!bg_dont_wait_flag){
					waitpid(pID, NULL, WUNTRACED | WCONTINUED); //ctrl-z untraced release.
				}
				else {
					if(add_job_to_jobs_list(jobsList, pID, BACKGROUND, cmdString, 0))
						std::cout << "smash error: reached job list limit (100 jobs)" << std::endl;
				}
				break;
	}
}
//**************************************************************************************
// function name: BgCmd
// Description: if command is in background, insert the command to jobs
// Parameters: command string, pointer to jobs
// Returns: 0- BG command -1- if not
//**************************************************************************************
int BgCmd(char* lineSize, std::list<job>* jobs)
{

	char* Command;
	const char* delimiters = " \t\n";
	char *args[MAX_ARG];
	char cmdString[MAX_LINE_SIZE];
	
	strcpy(cmdString, lineSize);
	cmdString[strlen(cmdString)-1] = '\0';
	
	if (lineSize[strlen(lineSize)-2] == '&')
	{
		lineSize[strlen(lineSize)-2] = '\0';
		/*copy original command*/
		Command = strtok(lineSize, delimiters);
		if(Command == NULL)
			return CMD_SUCCESS;	
		args[0] = Command;
        for (int i = 1; i < MAX_ARG; i++)
        	args[i] = strtok(NULL, delimiters);

        bg_dont_wait_flag = true;
        ExeExternal(args, cmdString, jobs);
		return CMD_SUCCESS;	
	}
	return -1;
}


//**************************************************************************************
// function name: add_job_to_jobs_list
// Description: adds a job to joblist
// Parameters: 
// Returns: 1 if job count is 100 else returns 0 
//**************************************************************************************
bool add_job_to_jobs_list(std::list<job>* jobsList, int pID, jobStatus status, char* cmdString, int startTime)
{	
	char cmd_buffer[MAX_LINE_SIZE];	
	strcpy(cmd_buffer, cmdString);
	
	int pid=pID;
	jobStatus stat = status;
	int startTim = startTime;
	
	int jobCount = jobsList->size();
	if (jobCount == 100) return CMD_ERROR;
	jobIndex++;
	job newJob(jobIndex, pid, stat, cmd_buffer/*, startTim*/);
	jobsList->push_back(newJob);
	
	return CMD_SUCCESS;
}


//**************************************************************************************
// function name: remove_finished_jobs
// Description: updates jobs list, removes finished jobs  
// Parameters: jobsList
// Returns: NONE 
//**************************************************************************************
void remove_finished_jobs(std::list<job>* jobsList)
{
	if(!jobsList->size()) return;
	std::list<job>::iterator it;
	it = jobsList->begin();
	while(it != jobsList->end()) {
		if (waitpid(it->pid, NULL, WNOHANG)!=0){
			it = jobsList->erase(it); // job done!
		}
		else it++;
	}
}


