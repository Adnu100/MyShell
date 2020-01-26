#ifndef PROMPT_H
#define PROMPT_H

#define HEADER "\033[95m"
#define OKBLUE "\033[94m"
#define OKGREEN "\033[92m"
#define WARNING "\033[93m"
#define BOLD "\033[1m"
#define ENDC "\033[0m"
#define FAIL "\033[91m"

extern char *LOGIN;
extern int LENHOME;

void initprompt(void);
void prompt(void);

#endif
