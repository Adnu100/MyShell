#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "shell.h"
#include "prompt.h"
#include "jobsmanager.h"

extern char **environ;
int child = 0, stopped = 0;

int analyse_n_execute(char *cmd);
int strstr_start(char *bigstr, char *smallstr);
int startswith(char *line, char *starting);
void normalexec(char *cmd, int w);
void pipenexec(char *cmd1, char * cmd2);
void ioredirexec(char *cmd, char *file, int redirection);
void execute_cmd(char *full_cmd);
void mystrcpy(char *dest, char *src);
void act(int signal_number);

void act(int signal_number) {
	if(!child) {
		putchar('\n');
		prompt();
		fflush(stdout);
	}
	else
		kill(stopped, signal_number);
}

int main(int argc, char *argv[]) {
	char *cmd = (char *)malloc(sizeof(char) * MAX_COMMAND_LENGTH);
	initjobs();
	signal(SIGINT, act);
	signal(SIGTSTP, act);
	initprompt();
	prompt();
	while(fgets(cmd, MAX_COMMAND_LENGTH, stdin)) {
		if(analyse_n_execute(cmd) == -1)
			return 0;
		prompt();
		fflush(stdout);
	}
	printf("exit\n");
	return 0;
}	

void mystrcpy(char *dest, char *src) {
	while((*(dest++) = *(src++)));
}

int analyse_n_execute(char *cmd) {
	int i, status = 0;
	char filepath[MAX_FILEPATH], *p;
	for(i = 0; cmd[i]; i++)
		if(cmd[i] == '|' || cmd[i] == '>' || cmd[i] == '<' || cmd[i] == '&' || cmd[i] == ';')
			break;
		else if(cmd[i] == '2' && cmd[i + 1] == '>')
			if(i != 0 && (cmd[i - 1] == ' ' || cmd[i - 1] == '\t'))
				break;
	switch(cmd[i]) {
		case '|':
			cmd[i] = '\0';
			pipenexec(cmd, cmd + i + 1);
			break;
		case '>':
			cmd[i] = ' ';
			strcpy(filepath,  (p = strtok(cmd + i + 1, " \t\n")));
			mystrcpy(cmd + i + 1, p + (strlen(p) + 1));
			ioredirexec(cmd, filepath, STDOUT_FILENO);
			break;
		case '<':
			cmd[i] = ' ';
			strcpy(filepath, (p = strtok(cmd + i + 1, " \t\n")));
			mystrcpy(cmd + i + 1, p + (strlen(p) + 1));
			ioredirexec(cmd, filepath, STDIN_FILENO);
			break;
		case '2':
			cmd[i] = ' ';
			cmd[i + 1] = ' ';
			strcpy(filepath, (p = strtok(cmd + i + 1, " \t\n")));
			strcpy(cmd + i + 1, p + strlen(p) + 1);
			ioredirexec(cmd, filepath, STDERR_FILENO);
			break;
		case '&':
			cmd[i] = '\0';
			normalexec(cmd, 0);
			break;
		case ';':
			cmd[i] = '\0';
			normalexec(cmd, 1);
			analyse_n_execute(cmd + i + 1);
			break;
		case '\0':
			normalexec(cmd, 1);
			break;
		default:
			break;
	}
	return status;
}	

void normalexec(char *cmd, int w) {
	int pid, ws;
	if(startswith(cmd, "exit"))
		exit(0);
	else if(startswith(cmd, "cd"))
		cd(cmd);
	else if(startswith(cmd, "fg")) 
		fg(cmd);	
	else if(startswith(cmd, "bg"))
		bg(cmd);
	else if(startswith(cmd, "jobs"))
		jobsl(cmd);
	else {
		pid = fork();
		if(pid == 0) {
			execute_cmd(cmd);
			exit(0);
		}
		else {
			child++;
			stopped = pid;
			if(w) 
				waitpid(pid, &ws, WUNTRACED);
			else 
				waitpid(pid, &ws, WNOHANG | WUNTRACED);
			if(WIFEXITED(ws))
				child--;
		}
	}
}

void pipenexec(char *cmd1, char *cmd2) {
	int pid, pid2, pfd[2], ws;
	pipe(pfd);
	pid = fork();
	if(pid == 0) {
		close(pfd[0]);
		dup2(pfd[1], STDOUT_FILENO);
		execute_cmd(cmd1);
		close(pfd[1]);
		exit(0);
	}
	else {
		child++;
		stopped = pid;
		waitpid(pid, &ws, WUNTRACED);
		close(pfd[1]);
		pid2 = fork();
		if(pid2 == 0) {
			waitpid(pid, &ws, 0);
			if(WIFEXITED(ws)) {
				dup2(pfd[0], STDIN_FILENO);
				analyse_n_execute(cmd2);
			}
			exit(0);
		}
		else {
			stopped = pid2;
			waitpid(pid2, &ws, WUNTRACED);
			if(WIFEXITED(ws))
				child--;
			close(pfd[0]);
		}		
	}
}

void ioredirexec(char *cmd, char *file, int redirection) {
	int fd, pid, ws;
	if(redirection == STDIN_FILENO)
		fd = open(file, O_RDONLY);
	else if(redirection == STDOUT_FILENO || redirection == STDERR_FILENO)
		fd = open(file, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	else {
		printf("Invalid redirection mode requested");
		return;
	}
	if(fd == -1) {
		perror("Error");
		return;
	}
	pid = fork();
	if(pid == 0) {
		dup2(fd, redirection);
		analyse_n_execute(cmd);
		exit(0);
	}
	else { 
		child++;
		stopped = pid;
		waitpid(pid, &ws, WUNTRACED);
		if(WIFEXITED(ws)) {
			child--;
			close(fd);
		}
	}
}

/* For the last free function call in execute_cmd:
 * 	if execvpe suceeds, then following line is unreachable,
 * 	but if it fails, then the malloced memory is needed to be freed
 * 	in case it suceeds, no need to free the args as the whole data segment,
 * 	head, stack, bss is newly loaded by the new process called by 
 * 	exec family of functions
 */
void execute_cmd(char *full_cmd) {
	int i;
	char *cmd_name;
	char **args = (char **)malloc(sizeof(char *) * MAX_ARGS);
	cmd_name = strtok(full_cmd, " \t\n");
	if(cmd_name != NULL) {
		args[0] = cmd_name;
		i = 1;
		/* tokeninse the full command into arguments */
		while((args[i++] = strtok(NULL, " \t\n")));
		if(execvpe(cmd_name, args, environ) == -1) 
			perror("Error");
	}
	free(args);
}
