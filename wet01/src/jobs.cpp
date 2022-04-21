#include "jobs.h"

JOBS jobs_g;
void * jobs_gs= (void *)&jobs_g;

int JOBS::commands_id = 1;
