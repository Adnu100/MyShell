#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "shell.h"
#include "prompt.h"

char *HOME = NULL, *LOGIN = NULL, *SESSION = NULL;
int LENHOME = 0;

void initprompt(void) {
	HOME = getenv("HOME");
	LENHOME = strlen(HOME);
	LOGIN = getlogin();
}

void prompt(void) {
	char *dname = get_current_dir_name();
	if(strstr_start(dname, HOME)) {
		dname[0] = '~';
		mystrcpy(dname + 1, dname + LENHOME);
	}
	printf(BOLD OKGREEN "%s@localhost" ENDC ":" BOLD OKBLUE "%s" ENDC HEADER "(myshell)" ENDC "$ ", LOGIN, dname);
	free(dname);
}


