#ifndef JOBSLIST_H
#define JOBSLIST_H

#define MAX_COMMAND_LENGTH_COPY 1024

typedef enum pstatus {STOPPED, RUNNING} pstatus;

typedef struct job_t {
	char identifier[MAX_COMMAND_LENGTH_COPY];
	pid_t pid;
	int job_number;
	pstatus status;
} job_t;

typedef struct node {
	job_t job;
	struct node *next, *prev;
} node;

typedef struct jobs {
	int total;
	node *tail;
	node *head;
} jobs;

void initjobs(void);
void appendjob(char *cmd, pid_t pid, pstatus status);
pid_t popjob(void);
pid_t popbyidentifier(char *identifier);
pid_t popbynumber(int n);
int gettotaljobs(void);
void remount(pstatus status);
int printalljobs(void);
int printjobbynumber(int number);
int printjobbyidentifier(char *identifier);
void updatejobs(void);

extern jobs alljobs;
extern pstatus lstatus;

#endif
