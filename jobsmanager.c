#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "shell.h"
#include "jobsmanager.h"

jobs alljobs = {0};
pstatus lstatus;
int INITIALISED = 0, NOPRINT = 0;
node *cached = NULL;

void initjobs() {
	if(!INITIALISED) {
		alljobs.total = 0;
		alljobs.tail = alljobs.head = NULL;
	}		
}

void appendjob(char *cmd, pid_t pid, pstatus status) {
	int n;
	if(alljobs.total) {
		n = alljobs.tail->job.job_number + 1;
		alljobs.tail->next = (node *)malloc(sizeof(node));
		alljobs.tail->next->prev = alljobs.tail;
		alljobs.tail->next->next = NULL;
		alljobs.tail = alljobs.tail->next;
	}
	else {
		n = 1;
		alljobs.head = (node *)malloc(sizeof(node));
		alljobs.tail = alljobs.head;
		alljobs.head->next = alljobs.head->prev = NULL;
	}
	strcpy(alljobs.tail->job.identifier, cmd);
	alljobs.tail->job.pid = pid;
	alljobs.tail->job.job_number = n;
	alljobs.tail->job.status = status;
	alljobs.total++;
	if(status == STOPPED) 
		printf("\n[%d]\tstopped\t\t%s", alljobs.tail->job.job_number, alljobs.tail->job.identifier);	
	else 
		printf("[%d] %d\n", alljobs.tail->job.job_number, pid);
}

pid_t popjob(void) {
	node *n;
	if(!alljobs.total)
		return 0;
	alljobs.total--;
	if(cached)
		free(cached);
	cached = alljobs.tail;
	if(alljobs.total) {
		n = alljobs.tail;
		n->prev->next = NULL;
		alljobs.tail = n->prev;
	}
	else
		alljobs.tail = alljobs.head = NULL;
	if(!NOPRINT)
		printf("%s", cached->job.identifier);
	fflush(stdout);
	lstatus = cached->job.status;
	return cached->job.pid;
}

pid_t popbyidentifier(char *identifier) {
	node *n = alljobs.tail;
	if(!alljobs.total)
		return 0;
	alljobs.total--;
	while(n) {
		if(strstr_start(n->job.identifier, identifier)) {
			if(cached)
				free(cached);
			cached = n;
			if(n->next) 
				n->next->prev = n->prev;
			else
				alljobs.tail = n->prev;
			if(n->prev)
				n->prev->next = n->next;
			else
				alljobs.head = n->next;
			break;
		}
		n = n->prev;
	}
	if(!NOPRINT)
		printf("%s", cached->job.identifier);
	lstatus = cached->job.status;
	return cached->job.pid;
}

pid_t popbynumber(int number) {
	node *n = alljobs.head;
	if(!alljobs.total)
		return 0;
	alljobs.total--;
	while(n) {
		if(n->job.job_number == number) {
			if(cached)
				free(cached);
			cached = n;
			if(n->next)
				n->next->prev = n->prev;
			else
				alljobs.tail = n->prev;
			if(n->prev)
				n->prev->next = n->next;
			else
				alljobs.head = n->next;
			break;

		}
		n = n->next;
	}
	if(!NOPRINT)
		printf("%s", cached->job.identifier);
	lstatus = cached->job.status;
	return cached->job.pid;
}

int gettotaljobs(void) {
	return alljobs.total;
}

void remount(pstatus running) {
	if(!cached)
		return;
	cached->job.status = running;
	cached->next = NULL;
	if(alljobs.total) {
		cached->prev = alljobs.tail;
		alljobs.tail->next = cached;
		alljobs.tail = cached;
		cached->job.job_number = cached->prev->job.job_number + 1;
	}
	else {
		alljobs.head = alljobs.tail = cached;
		cached->prev = NULL;
		cached->job.job_number = 1;
	}
	if(running == STOPPED)
		printf("\n[%d]\tstopped\t\t%s", cached->job.job_number, cached->job.identifier);
	alljobs.total++;
	cached = NULL;
}

int printalljobs(void) {
	node *n = alljobs.head;
	if(alljobs.total) {
		while(n) {
			if(n->job.status == STOPPED)
				printf("[%d]\tstopped\t\t%s", n->job.job_number, n->job.identifier);
			else if(n->job.status == RUNNING)
				printf("[%d]\trunning\t\t%s", n->job.job_number, n->job.identifier);
			n = n->next;
		}
	}
	return alljobs.total;
}

int printjobbynumber(int number) {
	node *n;
	if(!alljobs.total)
		return 0;
	n = alljobs.head;
	while(n) {
		if(n->job.job_number == number) {
			if(n->job.status == STOPPED)
				printf("[%d]\tstopped\t\t%s", n->job.job_number, n->job.identifier);
			else if(n->job.status == RUNNING)
				printf("[%d]\trunning\t\t%s", n->job.job_number, n->job.identifier);
			return 1;
		}
		n = n->next;
	}
	return 0;
}

int printjobbyidentifier(char *identifier) {
	node *n;
	if(!alljobs.total) 
		return 0;
	n = alljobs.tail;
	while(n) {
		if(strstr_start(n->job.identifier, identifier)) {
			if(n->job.status == STOPPED)
				printf("[%d]\tstopped\t\t%s", n->job.job_number, n->job.identifier);
			else if(n->job.status == RUNNING)
				printf("[%d]\trunning\t\t%s", n->job.job_number, n->job.identifier);
			return 1;
		}
		n = n->next;
	}
	return 0;
}

void updatejobs(void) {
	NOPRINT = 1;
	node *n = alljobs.head;
	int ws;
	while(n) {
		waitpid(n->job.pid, &ws, WNOHANG);
		if(WIFEXITED(ws)) {
			printf("[%d]\tdone\t\t%s", n->job.job_number, n->job.identifier);
			popbynumber(n->job.job_number);
		}
		n = n->next;
	}
	NOPRINT = 0;
}

void jobsdestroyall(void) {
	node *n = alljobs.head;
	node *prev = NULL;
	while(n) {
		prev = n;
		n = n->next;
		free(prev);
	}
}
