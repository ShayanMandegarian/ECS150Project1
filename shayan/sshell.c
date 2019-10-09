#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char *argv[])
{
	int retval;
	char* cmd[2];
	size_t size = 512 * sizeof(char);
	pid_t pid;
	
	cmd[0] = (char*)malloc(size);
	

	printf("sshell$ ");
	getline(&cmd[0], &size, stdin);
	cmd[0] = strtok(cmd[0], " \n");
	cmd[1] = NULL;
	/*
	int counter = 0;
	while (tokenized != NULL)
	{
		inputArray[counter] = (char*)malloc(strlen(tokenized + 1));
		inputArray[counter] = tokenized;
		tokenized = strtok(userInput, " ");
		counter++;
	}
	*/

	pid = fork();
	if(pid == 0)
	{
		execvp(cmd[0], cmd);
		exit(1);
	}
	else if(pid != 0)
	{
		waitpid(-1, &retval, 0);
		fprintf(stderr, "+ completed '%s' [%d]\n", cmd[0], retval);
		main(argc, argv); // Restart so user can continue inputting commands
	}

	return EXIT_SUCCESS;
}
