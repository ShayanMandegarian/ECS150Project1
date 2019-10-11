#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

struct input
{
	char input[512];
	char* command[1];
	char* arguments[16];

};

int main(int argc, char *argv[])
{
	int retval;
	char* cmd[2];
	size_t size = 512 * sizeof(char);
	pid_t pid;
	
	cmd[0] = (char*)malloc(size);
	struct input *userIn = malloc(sizeof(struct input));

	printf("sshell$ ");
	int lineSize = getline(&cmd[0], &size - 1, stdin);
	if (lineSize < 0)
	{
		perror("getline");
		exit(1);
	}
	
	cmd[0] = strtok(cmd[0], "\n");
	if (cmd[0] == NULL)
        {
		//free(cmd[0]);
		//free(userIn);
                main(argc, argv);
        }
	strcpy(userIn->input, cmd[0]);
	//cmd[1] = NULL;
	userIn->arguments[15] = NULL;

	int counter = 0;
	cmd[0] = strtok(userIn->input, " ");
	while (cmd[0] != NULL)
	{
		//cmd[0] = strtok(NULL, " ");
		//printf("cmd[0]: %s\n", cmd[0]);
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
	

	pid = fork();
	if(pid == 0)
	{
		//execvp(cmd[0], cmd);
		//printf("command: %s\n", userIn->command[0]);
		execvp(userIn->command[0], userIn->arguments);
		exit(1);
	}
	else if(pid != 0)
	{
		waitpid(-1, &retval, 0);
		fprintf(stderr, "+ completed '%s' [%d]\n", userIn->input, retval);
		main(argc, argv); // Restart so user can continue inputting commands
	}

	return EXIT_SUCCESS;
}
