#include <stdlib.h>
#include <string.h>
#include "shell.h"
#include "jobsmanager.h"

jobs alljobs;

void initjobs() {
	alljobs.total = 0;
	alljobs.list = NULL;
}

void appendjob(char *cmd, pid_t pid) {
	node *n = (node *)malloc(sizeof(node));
	strcpy(n->job.identifier, cmd);
	n->job.pid = pid;
	if(alljobs.total)
		n->job.job_number = alljobs.list->job.job_number + 1;
	else
		n->job.job_number = 1;
	n->prev = alljobs.list;
	alljobs.total++;
	alljobs.list = n;
}

pid_t popjob() {
	if(!alljobs.total)
		return 0;
	node *n = alljobs.list;
	pid_t p = alljobs.list->job.pid;
	alljobs.total--;
	alljobs.list = alljobs.list->prev;
	free(n);
	return p;
}

int strstr_start(char *bigstr, char *smallstr) {
	while(*smallstr && (*(bigstr++) == *(smallstr++)));
	return (!(*smallstr) && *(smallstr - 1) == *(bigstr - 1));
}

pid_t popbyidentifier(char *identifier) {
	node *prev = NULL;
	node *n = alljobs.list;
	pid_t pid;
	while(n) {
		if(strstr_start(n->job.identifier, identifier)) {
			if(prev) {
				prev->prev = n->prev;
				pid = n->job.pid;
				free(n);
				return pid;
			}
			else
				return popjob();
		}	
		prev = n;	
		n = n->prev;
	}
	return 0;
}

pid_t popbynumber(int number) {
	node *prev = NULL;
	node *n = alljobs.list;
	pid_t pid;
	while(n) {
		if(n->job.job_number == number) {
			if(prev) {
				prev->prev = n->prev;
				pid = n->job.pid;
				free(n);
				return pid;
			}	
			else
				return popjob();
		}
		prev = n;
		n = n->prev;
	}
	return 0;
}
