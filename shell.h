#ifndef SHELL_H
#define SHELL_H

#define MAX_COMMAND_LENGTH 1024
#define MAX_ARGS ((MAX_COMMAND_LENGTH) / 2)
#define MAX_FILEPATH 256

extern char **environ;

extern char *HOME;

extern pid_t child;

void mystrcpy(char *dest, char *src);
int strstr_start(char *bigstr, char *smallstr);
void cd(char *cmd);
void fg(char *cmd);
void bg(char *cmd);
void jobsl(char *cmd);
int analyse_n_execute(char *cmd);
int strstr_start(char *bigstr, char *smallstr);
int startswith(char *line, char *starting);
void normalexec(char *cmd, int w);
void pipenexec(char *cmd1, char * cmd2);
void ioredirexec(char *cmd, char *file, int redirection);
void execute_cmd(char *full_cmd);
void mystrcpy(char *dest, char *src);
void act(int signal_number);

#endif
