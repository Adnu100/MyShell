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


pid_t child = 0;
int stopped = 0;
sighandler_t default_close = NULL, default_stop = NULL, default_continue = NULL;
int childflag = 0;

void act(int signal_number) {
	int ws;
	if(signal_number == SIGCONT) {
		if(child) {
			kill(child, SIGCONT);
			waitpid(child, &ws, WUNTRACED);
			if(WIFEXITED(ws))
				exit(0);
		}
	}
	else if(childflag) {
		kill(child, signal_number);
		if(signal_number == SIGINT)
			default_close(SIGINT);
		else if(signal_number == SIGTSTP)
			default_stop(SIGSTOP);
	}
	else {
		if(!child) {
			putchar('\n');
			prompt();
			fflush(stdout);
		}
		else
			kill(child, signal_number);
	}
}

int main(int argc, char *argv[]) {
	char *cmd = (char *)malloc(sizeof(char) * MAX_COMMAND_LENGTH);
	initjobs();
	default_close = signal(SIGINT, act);
	default_stop = signal(SIGTSTP, act);
	default_continue = signal(SIGCONT, act);
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
			child = pid;
			if(w) 
				waitpid(pid, &ws, WUNTRACED);
			else 
				waitpid(pid, &ws, WNOHANG | WUNTRACED);
			if(WIFSTOPPED(ws)) 
				appendjob(cmd, pid);
			child = 0;
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
		child = pid;
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
				child = 0;
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
		childflag = 1;
		dup2(fd, redirection);
		analyse_n_execute(cmd);
		exit(0);
	}
	else { 
		child = pid;
		waitpid(pid, &ws, WUNTRACED);
		if(WIFSTOPPED(ws) || WIFSIGNALED(ws)) 
			appendjob(cmd, pid);
		child = 0;
	}
	ftruncate(fd, lseek(fd, 0, SEEK_CUR));
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
		/* tokeninse the full command into arguments */
		while((args[i++] = strtok(NULL, " \t\n")));
		if(execvpe(cmd_name, args, environ) == -1) 
			perror("Error");
	}
	free(args);
}
