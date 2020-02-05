/* basic functions, necessary typedefs, structs, enumerations 
 * and API for a list containing all the jobs which are either stopped
 * or running in background
 */

#ifndef JOBSLIST_H
#define JOBSLIST_H

#define MAX_COMMAND_LENGTH_COPY 1024

/* structure will only store stopped or background jobs */
typedef enum pstatus {STOPPED, RUNNING} pstatus;

/* contains information about a job */
typedef struct job_t {
	char identifier[MAX_COMMAND_LENGTH_COPY];
	pid_t pid;
	int job_number;
	pstatus status;
} job_t;

/* a basic node in a list  */
typedef struct node {
	job_t job;
	struct node *next, *prev;
} node;

/* a data structure like list to hold first and last jobs
 * which will be linked to each other with next and prev pointers
 * and also the list will record the total number of items
 * it is holding currently
 */
typedef struct jobs {
	int total;
	node *tail;
	node *head;
} jobs;

void initjobs(void);				//initialises global variable containing all jobs stopped or running in background
void appendjob(char *cmd, pid_t pid, pstatus status);	
						//append a process to the list (requires name of process cmd, process id pid, and the status of the process status)
pid_t popjob(void);				//pop the last in job and return pid
pid_t popbyidentifier(char *identifier);	//pop the job whose name matches with identifier
pid_t popbynumber(int n);			//pop the nth job
int gettotaljobs(void);				//get total number of jobs (just returns the number in structure but Abstraction!)
void remount(pstatus status);			//remount the last popped job into list again (in case someone does fg and then again Ctrl-Z)
int printalljobs(void);				//print all jobs :)
int printjobbynumber(int number);		//print nth job and status
int printjobbyidentifier(char *identifier);	//print job whose name matches with identifier and its status
void updatejobs(void);				//to be called if SIGCHLD received, linearly checks for all jobs if they have been terminated and then removes them out from the list
void jobsdestroyall(void);			//frees all the jobs, called when exiting

/* no need to create many lists, there is only one list required to store all jobs
 * so, we reduce the job of passing this to each function and declare it global
 * all the functions will work on this global now
 */
extern jobs alljobs;

/* when a job is popped out, programmer may want to check if its stopped
 * or running because I don't have two structures to hold running and stopped jobs
 * so the proper status of last updated job will be stored in this variable by all
 * popping functions
 * programmer is advised to check this only after popping operation
 */
extern pstatus lstatus;

#endif
