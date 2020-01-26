#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include "shell.h"

extern int child, stopped;

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
	if(!stopped) 
		printf("fg: current: no such job\n");
	else {
		kill(stopped, SIGCONT);
		waitpid(stopped, &ws, WUNTRACED);
		if(WIFEXITED(ws)) {
			child--;
			stopped = 0;
		}
		stopped = 0;
	}
}

void bg(char *cmd) {
	if(stopped)
		printf("[%d]\n", stopped);
	else
		printf("bg: current: no such job exist\n");
}

void jobsl(char *cmd) {
	
}


