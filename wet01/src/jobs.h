#ifndef JOBS_H_
#define JOBS_H_
#include <list>
#include <iterator>
#include <string>
#include <iostream>
using namespace std;


class Job_Command
{
public:
    string commands_line;
    int creation_time;
    int pid;
    string status;
    int command_id;

};


class JOBS
{
public:
    list <Job_Command> commands_list;
    Job_Command fg_command;

    static int commands_id;

};


//Global
extern JOBS jobs_g;

extern void* jobs_gs;


#endif /* JOBS_H_ */
