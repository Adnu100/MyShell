/* basic functions which will be used by almost all the files
 * and some declarations like maximum command length or maximum number of arguments 
 * to any command or maximum file path that can be provided
 * you may change these macro values, but flexibility is not checked
 */

#ifndef SHELL_H
#define SHELL_H

#define MAX_COMMAND_LENGTH 1024
#define MAX_ARGS ((MAX_COMMAND_LENGTH) / 2)
#define MAX_FILEPATH 256

/* used to specify in which mode to open a file
 * to redirect standard output or standard error
 * IGN - ignore mode (when redirecting standard input)
 * ERASE - 'w' or 'w+' mode
 * APPEND = 'a' or 'a+' mode
 */
typedef enum redirection_mode_t {IGN, ERASE, APPEND} rdrn_mode_t;

extern char **environ;

extern char *HOME;

/* child contains the pid of current running process */
extern pid_t child;

void mystrcpy(char *dest, char *src);		//strcpy doesnt do well with overlapping strings so wrote my 1 line code (faster too!)
int strstr_start(char *bigstr, char *smallstr);	//checks if bigstr starts with smallstr
int startswith(char *line, char *starting);	//startswith carries some initialisation and invokes strstr_start
void cd(char *cmd);				//change directore
void fg(char *cmd);				//bring to foreground
void bg(char *cmd);				//allow execution in background
void jobsl(char *cmd);				//show jobs 
int analyse_n_execute(char *cmd);		//find tokens like <, >, &, ; in command and perform approriate action
void normalexec(char *cmd, int w);		//execute normal command
void pipenexec(char *cmd1, char * cmd2);	//recursive code for each pipe
void ioredirexec(char *cmd, char *file, int redirection, rdrn_mode_t rdrn_mode);
						//input or output redirection, redirection mode rdrn_mode in case of stdout and stderr should be either ERASE or APPEND, IGN would be taken as ERASE and in case of stdin this parameter is not considered but IGN should be given
void execute_cmd(char *full_cmd);		//carru out necessary initialisation and invoke exec
void act(int signal_number);			//signal handling function for SIGINT and SIGTSTP
void chact(int sig);				//signal handling function for SIGCHLD (sets approriate flag for checking terminated jobs)

#endif
