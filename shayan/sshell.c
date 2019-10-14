#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>

struct input
{
	char input[512];
	char rawInput[512];
	char* command[1];
	char* arguments[16];

};


int inRedirect(struct input* in, char* left, char* right)
{
	char* tokenize; // used to get the command info from the command line

        tokenize = (char*)malloc(512 * sizeof(char)); // allocate the strings
	strcpy(tokenize, in->rawInput);
	tokenize = strtok(tokenize, "<"); // tokenize the input to split before and after the <
	int iterator = 0;
	while (tokenize != NULL)
	{
		if (iterator == 0)
		{
			strcpy(left, tokenize);
		}
		else if (iterator == 1)
		{
			strcpy(right, tokenize);
			
		}
		tokenize = strtok(NULL, "<");
		tokenize = strtok(tokenize, " ");
		iterator++;
	}

	if (iterator == 1) // IF NO SOURCE WAS PRESENT, ERROR
	{
		fprintf(stderr, "Error: no input file\n");
		return 1;
	}

	return 0;
}


int builtin(struct input* in)
{
	if (!strcmp(in->input, "exit"))
	{
		fprintf(stderr, "Bye...\n");
		exit(0);
		return 0;
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
	char* left;
	char* right;
	
	cmd[0] = (char*)malloc(size);
	cmd[1] = NULL;
	struct input *userIn = malloc(sizeof(struct input));
	
	while (1)
	{
		int cont = 0; // USED TO FLAG A CONTINUE IN A NESTED LOOP
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
				continue;
			}
			else
			{
				userIn->arguments[counter] = cmd[0];
				if (!strcmp(cmd[0], "<"))
				{
					left = (char*)malloc(512 * sizeof(char));
        				right = (char*)malloc(512 * sizeof(char));
					if (inRedirect(userIn, left, right) != 1)
					{
						//printf("left: %s\n", left);
						//printf("right: %s\n", right);
						cont = 2; // NEED TO CHANGE REDIRECTION
						break;
					}
					else
					{
						cont = 1; // IF inRedirect PASSES 1, MEANING ERROR, CONTINUE
					}
					
				}
			}
			cmd[0] = strtok(NULL, " ");
			counter++;
		}

		if (builtin(userIn) == 1 || cont == 1)
		{
			continue;
		}		

		pid = fork();
		if(pid == 0)
		{
			//printf("cont: %d\n", cont);
			if (cont == 2)
			{
				int inFile = open(right, O_RDONLY);      
				if (inFile == -1)
				{
					fprintf(stderr, "Error: cannot open input file\n");
					close(inFile);
					continue;
				}
				
				dup2(inFile, 0);
				close(inFile);
				for (int j = 0; j < 16; j++)
				{
					if (userIn->arguments[j] != NULL && !strcmp(userIn->arguments[j], "<"))
					{
						userIn->arguments[j] = NULL;
					}
					//printf("%d: %s\n", j, userIn->arguments[j]);
				}
				execvp(userIn->command[0], userIn->arguments);
				exit(1);

			}
			else
			{
				execvp(userIn->command[0], userIn->arguments);
				exit(1);
			}	
			
		}
		else if(pid != 0)
		{
			int status = waitpid(-1, &retval, 0);
			if  (status == 256)
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
