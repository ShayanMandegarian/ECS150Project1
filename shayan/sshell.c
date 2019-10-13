#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <limits.h>

struct input
{
	char input[512];
	char rawInput[512];
	char* command[1];
	char* arguments[16];

};

int builtin(struct input* in)
{
	if (!strcmp(in->input, "exit"))
	{
		fprintf(stderr, "Bye...\n");
		exit(0);
		return(0);
	}
	else if(!strcmp(in->input, "pwd"))
	{
		size_t size = PATH_MAX*sizeof(char);
		char* buffer = (char*)malloc(size);
		if (getcwd(buffer, size) == buffer)
		{
			fprintf(stdout, "%s\n", buffer);
			fprintf(stderr, "+ completed '%s' [0]\n", in->input);
			return 1;
		}
		else
		{
			return 0;
		}	
	}
	else if(!strcmp(in->command[0], "cd"))
	{
		int retval = chdir(in->arguments[1]);
		if (retval == -1)
		{
			fprintf(stderr, "Error: no such directory\n");
			return 1;
		}
		fprintf(stderr, "+ completed '%s' [%d]\n", in->rawInput, retval);
		return 1;
	}
	else
	{
		return 0;
	}
}

int main(int argc, char *argv[])
{
	int retval;
	char* cmd[2];
	size_t size = 512 * sizeof(char);
	pid_t pid;
	
	cmd[0] = (char*)malloc(size);
	struct input *userIn = malloc(sizeof(struct input));
	
	while (1)
	{
		for (int i = 0; i < 16; i++)
		{
			userIn->arguments[i] = NULL;
		}
		printf("sshell$ ");
		//printf("arg 1: %s\n", userIn->arguments[1]);
		int lineSize = getline(&cmd[0], &size - 1, stdin);
		if (lineSize < 0)
		{
			perror("getline");
			exit(1);
		}

		if (!isatty(STDIN_FILENO))
	       	{
     			printf("%s", cmd[0]);
        		fflush(stdout);
    		}
	
		cmd[0] = strtok(cmd[0], "\n");
		if (cmd[0] == NULL)
        	{
			//free(cmd[0]);
			//free(userIn);
			continue;
		}
		strcpy(userIn->input, cmd[0]);
		strcpy(userIn->rawInput, userIn->input);
		userIn->arguments[15] = NULL;
		int counter = 0;
		cmd[0] = strtok(userIn->input, " ");
		while (cmd[0] != NULL)
		{
			if (counter == 0)
			{
				userIn->command[counter] = cmd[0];
				userIn->arguments[counter] = cmd[0];
			}
			else if (counter > 16)
			{
				fprintf(stderr, "Error: too many process arguments\n");
				//free(cmd[0]);
				//free(userIn);
				main(argc, argv);
			}
			else
			{
				userIn->arguments[counter] = cmd[0];
			}
			cmd[0] = strtok(NULL, " ");
			counter++;
		}

		if (builtin(userIn) == 1)
		{
			continue;
		}		

		pid = fork();
		if(pid == 0)
		{
			execvp(userIn->command[0], userIn->arguments);
			exit(1);
		}
		else if(pid != 0)
		{
			waitpid(-1, &retval, 0);
			if (retval == 256)
			{
				fprintf(stderr, "Error: command not found\n");
			}
			else
			{
				fprintf(stderr, "+ completed '%s' [%d]\n", userIn->rawInput, retval);
			}
		}
	}

	return EXIT_SUCCESS;
}
