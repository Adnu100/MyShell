#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define MAX_COMMAND_LENGTH 1024
#define MAX_ARGS ((MAX_COMMAND_LENGTH) / 2)
#define MAX_FILEPATH 256

extern char **environ;
const char PATH[] = "PATH=/home/adnesh/.local/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin";

int analyse_n_execute(char *cmd);
int checkexit(char *cmd);
void normalexec(char *cmd);
void pipenexec(char *cmd1, char * cmd2);
void rediroutputexec(char *cmd, char *file);
void execute_cmd(char *full_cmd);

int main(int argc, char *argv[]) {
	char *cmd = (char *)malloc(sizeof(char) * MAX_COMMAND_LENGTH);
	printf("--$ ");
	while(fgets(cmd, MAX_COMMAND_LENGTH, stdin)) {
		if(analyse_n_execute(cmd) == -1)
			return 0;
		printf("--$ ");
	}
	printf("exit\n");
	return 0;
}	

int checkexit(char *cmd) {
	char checkexittext[6];
	checkexittext[5] = '\0';
	int i = 0;
	while(cmd[i] == ' ' || cmd[i] == '\t')
		i++;
	strncpy(checkexittext, cmd + i, 5);
	if(!(strcmp(checkexittext, "exit ") && strcmp(checkexittext, "exit\t") && strcmp(checkexittext, "exit") && strcmp(checkexittext, "exit\n"))) {
		return 1;
	}
	return 0;
}

int analyse_n_execute(char *cmd) {
	int i, status = 0;
	char filepath[MAX_FILEPATH], *p;
	for(i = 0; cmd[i]; i++)
		if(cmd[i] == '|' || cmd[i] == '>' || cmd[i] == '<' || cmd[i] == '&' || cmd[i] == ';')
			break;
	switch(cmd[i]) {
		case '|':
			cmd[i] = '\0';
			pipenexec(cmd, cmd + i + 1);
			break;
		case '>':
			cmd[i] = ' ';
			strcpy(filepath,  (p = strtok(cmd + i + 1, " \t\n")));
			strcpy(cmd + i + 1, p + strlen(p) + 1);
			rediroutputexec(cmd, filepath);
			break;
		case '<':
			break;
		case '&':
			break;
		case ';':
			break;
		case '\0':
			normalexec(cmd);
			break;
		default:
			break;
	}
	return status;
}	

void normalexec(char *cmd) {
	int pid;
	if(checkexit(cmd))
		exit(0);
	else {
		pid = fork();
		if(pid == 0) {
			execute_cmd(cmd);
			exit(0);
		}
		else
			wait(0);
	}
}

void pipenexec(char *cmd1, char *cmd2) {
	int pid, pfd[2];
	pipe(pfd);
	pid = fork();
	if(pid == 0) {
		close(pfd[0]);
		dup2(pfd[1], 1);
		execute_cmd(cmd1);
		close(pfd[1]);
		exit(0);
	}
	else {
		wait(0);
		close(pfd[1]);
		pid = fork();
		if(pid == 0) {
			dup2(pfd[0], 0);
			analyse_n_execute(cmd2);
			exit(0);
		}
		else
			wait(0);
		close(pfd[0]);
	}
}

void rediroutputexec(char *cmd, char *file) {
	int fd = open(file, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	int pid;
	if(fd == -1) {
		perror("Error: ");
		return;
	}
	pid = fork();
	if(pid == 0) {
		dup2(fd, 1);
		analyse_n_execute(cmd);
		exit(0);
	}
	else 
		wait(0);
	close(fd);
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
		while((args[i++] = strtok(NULL, " \t\n")));
		if(execvpe(cmd_name, args, environ) == -1) 
			perror("Error: ");
	}
	free(args);
}
