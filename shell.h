#ifndef SHELL_H
#define SHELL_H

#define MAX_COMMAND_LENGTH 1024
#define MAX_ARGS ((MAX_COMMAND_LENGTH) / 2)
#define MAX_FILEPATH 256

extern char *HOME;

void mystrcpy(char *dest, char *src);
int strstr_start(char *bigstr, char *smallstr);
void cd(char *cmd);
void fg(char *cmd);
void bg(char *cmd);
void jobsl(char *cmd);

#endif
