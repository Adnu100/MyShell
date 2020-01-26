shell: shell.o prompt.o jobsmanager.o specialcommands.o
	cc shell.c prompt.c jobsmanager.c specialcommands.c -Wall -o shell

shell.o: shell.c shell.h prompt.h jobsmanager.h
	cc shell.c -c -Wall 

prompt.o: prompt.c shell.h prompt.h
	cc prompt.c -c -Wall

jobsmanager.o: jobsmanager.c shell.h jobsmanager.h
	cc jobsmanager.c -c -Wall

specialcommands.o: specialcommands.c shell.h
	cc specialcommands.c -c -Wall

