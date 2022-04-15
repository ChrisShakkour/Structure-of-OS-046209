//		commands.c
//********************************************
#include "commands.h"
//********************************************

// cd saved variables.
char prev_path[MAX_LINE_SIZE] = "";



// function name: ExeCmd
// Description: interperts and executes built-in commands
// Parameters: pointer to jobs, command string
// Returns: 0 - success,1 - failure
//**************************************************************************************
int ExeCmd(void* jobs, char* lineSize, char* cmdString)
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
			//printf("%s\n", args[i]); //TODO: remove
		}
	}
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
	else if (!strcmp(cmd, "mkdir"))
	{
		return CMD_SUCCESS;
	}
	/*************************************************/
	
	else if (!strcmp(cmd, "jobs")) 
	{
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
		return CMD_SUCCESS;
	} 
	/*************************************************/
	else if (!strcmp(cmd, "bg")) 
	{
		return CMD_SUCCESS;
	}
	/*************************************************/
	else if (!strcmp(cmd, "quit"))
	{
		return CMD_SUCCESS;
	} 
	/*************************************************/
	else // external command
	{
 		ExeExternal(args, cmdString);
	 	return CMD_SUCCESS;
	}
	if (illegal_cmd == true)
	{
		printf("smash error: > \"%s\"\n", cmdString);
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
void ExeExternal(char *args[MAX_ARG], char* cmdString)
{
	int pID;
    	switch(pID = fork()) 
	{
    		case -1: 
					// Add your code here (error)
					printf(" "); //TODO: remove
					/* 
					your code
					*/
        	case 0 :
                	// Child Process
               		setpgrp();
					
			        // Add your code here (execute an external command)
					
					/* 
					your code
					*/
			
			default:
                	// Add your code here
					printf(" "); //TODO: remove
					/* 
					your code
					*/
	}
}
//**************************************************************************************
// function name: ExeComp
// Description: executes complicated command
// Parameters: command string
// Returns: 0- if complicated -1- if not
//**************************************************************************************
int ExeComp(char* lineSize)
{
	char ExtCmd[MAX_LINE_SIZE+2];
	char *args[MAX_ARG];
    if ((strstr(lineSize, "|")) || (strstr(lineSize, "<")) || (strstr(lineSize, ">")) || (strstr(lineSize, "*")) || (strstr(lineSize, "?")) || (strstr(lineSize, ">>")) || (strstr(lineSize, "|&")))
    {
		// Add your code here (execute a complicated command)
					
		/* 
		your code
		*/
	} 
	return -1;
}
//**************************************************************************************
// function name: BgCmd
// Description: if command is in background, insert the command to jobs
// Parameters: command string, pointer to jobs
// Returns: 0- BG command -1- if not
//**************************************************************************************
int BgCmd(char* lineSize, void* jobs)
{

	char* Command;
	const char* delimiters = " \t\n";
	char *args[MAX_ARG];
	if (lineSize[strlen(lineSize)-2] == '&')
	{
		lineSize[strlen(lineSize)-2] = '\0';
		// Add your code here (execute a in the background)
					
		/* 
		your code
		*/
		
	}
	return -1;
}

