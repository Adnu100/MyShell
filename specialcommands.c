#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <ctype.h>
#include "shell.h"
#include "jobsmanager.h"

int startswith(char *line, char *starting) {
	while(*line == ' ' || *line == '\t')
		line++;
	if(strstr_start(line, starting)) {
		line += strlen(starting);
		if(*line == ' ' || *line == '\t' || *line == '\n')
			return 1;
	}
	return 0;
}

void mystrcpy(char *dest, char *src) {
	while((*(dest++) = *(src++)));
}

int strstr_start(char *bigstr, char *smallstr) {
	while(*smallstr && (*(bigstr++) == *(smallstr++)));
	return (!(*smallstr) && *(smallstr - 1) == *(bigstr - 1));
}

void cd(char *cmd) {
	char *tok;
	strtok(cmd, " \t\n");
	if((tok = strtok(NULL, " \t\n"))) {
		if(chdir(tok) == -1)
			perror("cd");			
	}
	else
		chdir(HOME);
}

void fg(char *cmd) {
	int ws;
	int pid;
	char *tok;
	if(!gettotaljobs()) 
		printf("fg: current: no such job exist\n");
	else {
		strtok(cmd, " \t\n");
		tok = strtok(NULL, " \t\n");
		if(tok) {
			if(isdigit((int)tok[0])) 
				pid = popbynumber(atoi(tok));
			else
				pid = popbyidentifier(tok);
			if(!pid) {
				printf("fg: %s: no such job exist\n", tok);
				return;
			}		
		}
		else 
			pid = popjob();
		child = pid;
		if(lstatus == STOPPED)
			kill(pid, SIGCONT);
		waitpid(pid, &ws, WUNTRACED);
		if(WIFSTOPPED(ws))
			remount(STOPPED);
		child = 0;
	}
}

void bg(char *cmd) {
	int pid;
	char *tok;
	if(!gettotaljobs()) 
		printf("bg: current: no such job exist\n");
	else {
		strtok(cmd, " \t\n");
		tok = strtok(NULL, " \t\n");
		if(tok) {
			if(isdigit((int)tok[0])) 
				pid = popbynumber(atoi(tok));
			else
				pid = popbyidentifier(tok);
			if(!pid) {
				printf("bg: %s: no such job exist\n", tok);
				return;
			}		
		}
		else
			pid = popjob();
		if(lstatus == STOPPED) 
			kill(pid, SIGCONT);
		else if(lstatus == RUNNING) 
			printf("bg: job already running in background\n");	
		remount(RUNNING);
	}
}

void jobsl(char *cmd) {
	char *tok;
	strtok(cmd, " \t\n");
	tok = strtok(NULL, " \t\n");
	if(tok)
		while(tok) {
			if(isdigit((int)tok[0])) {
				if(!printjobbynumber(atoi(tok))) 
					printf("jobs: %s: no such job exist\n", tok);
			}
			else {
				if(!printjobbyidentifier(tok))
					printf("jobs: %s: no such job exist\n", tok);
			}
			tok = strtok(NULL, " \t\n");
		}
	else if(!printalljobs())
		printf("jobs: current: no such job exist\n");
}


