#ifndef JOBSLIST_H
#define JOBSLIST_H

#define MAX_COMMAND_LENGTH_COPY 1024

typedef struct job_t {
	char identifier[MAX_COMMAND_LENGTH_COPY];
	pid_t pid;
	int job_number;
} job_t;

typedef struct node {
	job_t job;
	struct node *prev;
} node;

typedef struct jobs {
	int total;
	node *list;
} jobs;

void initjobs();
void appendjob(char *cmd, pid_t pid);
pid_t popjob();
pid_t popbyidentifier(char *identifier);
pid_t popbynumber(int n);

extern jobs alljobs;

#endif
