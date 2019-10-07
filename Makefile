sshell : sshell.o

sshell.o : sshell.c 
	gcc -c sshell.c -Wall -Werror -o sshell.o

clean : 
	-rm sshell 
	-rm sshell.o