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
#include <fstream>
#include <dirent.h>
#include <fcntl.h>
#include <string>
#include <sys/syscall.h>
#include <sys/stat.h>

#define CMD_SUCCESS 0
#define CMD_ERROR 1

#define BUFFERSIZE 1024
#define MAX_LINE_SIZE 80
#define MAX_ARG 20

//typedef enum { FALSE , TRUE } bool;

// functions
int ExeComp(char* lineSize);
int BgCmd(char* lineSize, void* jobs);
int ExeCmd(void* jobs, char* lineSize, char* cmdString);
void ExeExternal(char *args[MAX_ARG], char* cmdString);

#endif

