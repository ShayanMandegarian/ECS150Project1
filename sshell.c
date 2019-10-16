#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>


/*

pipeline function to set input of one command as output of another
void pipeline(struct input *in){
	int fd[2];

	int status = pipe(fd);
	if(status == 0){
		if(fork() == 0){
			close(fd[0]);
			dup2(fd[1], STDOUT_FILENO);
			close(fd[1]);
			execvp(in->command[0], in->arguments);
		} else {
			close(fd[1]);
			dup2(fd[0], STDIN_FILENO);
			close(fd[0]);
		}
	}
}
*/
struct input // this struct stores infomation about each user input
{
	char input[512]; // largely place holder
	char rawInput[512]; // stores the exact input for the completed message
	char* command[1]; // stores the COMMAND, aka the first argument, ie "date"
	char* arguments[16]; // stores the command and all the arguments, ie "date -u..."
	pid_t pid; // stores the PID of the child process running the command
	int done; // stores if the process running the command is done or not
};

struct charArray
{
	char* string[50];
};


int inRedirect(struct input* in, char* left, char* right)
{ // this function handles input redirection when a "<" character is detected
	char* tokenize; // used to get the command info from the command line
        tokenize = (char*)malloc(512 * sizeof(char)); // allocate the strings
	strcpy(tokenize, in->rawInput); 
	tokenize = strtok(tokenize, "<"); // tokenize the input to split before and after the <
	int iterator = 0;
	while (tokenize != NULL)
	{
		if (iterator == 0) // since iterator is expected only to be 0 or 1...
		{
			strcpy(left, tokenize); //... before the < is "left" ...
		}
		else if (iterator == 1)
		{
			strcpy(right, tokenize); // ... after the < is "right".
			
		}
		tokenize = strtok(NULL, "<");
		tokenize = strtok(tokenize, " "); // remove spaces after the "<".
		iterator++;
	}

	if (iterator == 1) // IF NO SOURCE WAS PRESENT, ERROR
	{
		fprintf(stderr, "Error: no input file\n");
		return 1; // RETURN VAL OF 1 MEANS ERROR
	}

	return 0; // RETURN VAL OF 0 MEANS SUCCESS
}

int outRedirect(struct input* in, char* left, char* right){
	char *token = strdup(in->rawInput);
	int iterator = 0;

	token = strtok(token, ">");
	while(token != NULL)
  {
		if(iterator == 0)
			strcpy(left, token);
		else if(iterator == 1)
			strcpy(right, token);
		token = strtok(NULL, ">");
		token = strtok(token, " ");
		iterator++;
	}

	if(iterator == 1)
  {
		fprintf(stderr, "Error: no output file\n");
		return 1;
	}
	return 0;
}

int getCount(char* cmd)
{
	int count = 0;
	char *token = strdup(cmd);

	token = strtok(cmd, "|");
	while(token != NULL)
  {
		count++;
	}
	return count;
}

int builtin(struct input* in, int error)
{ // this function handles built in commands: pwd, cd, exit

	if (!strcmp(in->input, "exit"))
	{
		if (error == 0)
		{
			fprintf(stderr, "Error: active jobs still running\n");

			return 1; // RETURN VAL 1 MEANS BUILT IN FUNCTION DIDN'T HAPPEN
		}
		fprintf(stderr, "Bye...\n");
		exit(0);
		return 0; // RETURN VAL 0 MEANS BUILT IN FUNCTION *DID* HAPPEN
	}
	else if(!strcmp(in->input, "pwd")) // basically just prints result of getcwd.
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
	else if(!strcmp(in->command[0], "cd")) // Basically just calls chdir and checks if it succeeded
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
	//struct input inputs[16]; used to keep the '|' delimited pipe commands
	
	cmd[0] = (char*)malloc(size);
	cmd[1] = NULL;
	int userInArraycounter = 0;

	struct input userInArray[500];
	
	while (1) // after creating the needed variables, start the main while loop
	{         // this loop repeats until the program is exitted by the user, or crashes.

		struct input *userIn = malloc(sizeof(struct input)); 
		int cont = 0; // USED TO FLAG A CONTINUE IN A NESTED LOOP
		int background = 0; // USED TO FLAG A BACKGROUND COMMAND
		int error = 0; // USED TO FLAG AN ERROR

		int test = 1; // = waitpid(-1, &retval, WNOHANG);
		for (int m = 0; m < userInArraycounter; m++)
		{ // this for loop checks for completed background functions
			if (userInArray[m].done == 1)
			{
				continue; // if the current background function is already complete, move on
			}
			test = waitpid(-1, &retval, WNOHANG);
			if (test > 0) // if waitpid returns other than 0 or -1, it is completed.
			{
				userInArray[m].done = 1;
				fprintf(stderr, "+ completed '%s' [%d]\n", userInArray[m].rawInput, retval);
			}
		}

		for (int i = 0; i < 16; i++) // resets all of the arguments of userIn to NULL for new command
		{
			userIn->arguments[i] = NULL;
		}
		printf("sshell$ "); // PRINTS THE PROMPT
		//getline(&cmd[0], &size - 1, stdin);

		if (getline(&cmd[0], &size - 1, stdin) < 0) // READS INPUT FROM STDIN
		{
			perror("getline");
			exit(1);
		}

		if (!isatty(STDIN_FILENO)) // FOR THE TESTER SCRIPT TO WORK
	       	{
     			printf("%s", cmd[0]);
        		fflush(stdout);
    		}
	
		cmd[0] = strtok(cmd[0], "\n"); // removes the new line character from the input
		if (cmd[0] == NULL) // if no input given, continue to the next loop
        	{
			free(userIn);
			continue;
		}
		
		strcpy(userIn->input, cmd[0]);
		strcpy(userIn->rawInput, userIn->input); // store the raw input for completed message
		userIn->arguments[15] = NULL; // set the final argument to null
		int counter = 0;
		cmd[0] = strtok(userIn->input, " "); // start tokenizing with spaces

		while (cmd[0] != NULL) // repeat the loop while the token isn't null, ie there are more arguments
		{
			
			char* temp = (char*)malloc(sizeof(size)); // temp variable used to parsing special characters
			temp = cmd[0];
			temp = strtok_r(temp, "<", &temp);
			if (temp != NULL && strcmp(temp, cmd[0])) // if "<" detected in the argument...
			{
				left = (char*)malloc(512 * sizeof(char));
                                right = (char*)malloc(512 * sizeof(char));
				if (inRedirect(userIn, left, right) != 1) // call the indirect function
				{
					cont = 2; // FLAG THAT INDIRECT WORKED
					break;
				}
				else
				{
					cont = 1; // FLAG THAT INDIRECT FAILED
				}
			}
			char* temp2 = (char*)malloc(sizeof(size));
			temp2 = cmd[0];
			if (temp2[strlen(temp2)-1] == *"&" && strcmp(temp2, "&")) // if "&" detected...
      {
				background = 2; // FLAG THAT "&" WAS DETECTED AS PART OF AN ARGUMENT, NOT BY ITSELF
				cmd[0][strlen(cmd[0]) -1] = *""; // replace the & character with nothing
				userInArray[userInArraycounter] = *userIn; // store the current input into an array of background commands
				userInArraycounter++;	
      }	
			if (counter == 0) // the first word is both the command, and the first argument
			{
				userIn->command[counter] = cmd[0];
				userIn->arguments[counter] = cmd[0];
			}
			else if (counter > 16) // if there are more than 16 arguments, there are too many
			{
				if (!error)
				{
					fprintf(stderr, "Error: too many process arguments\n");
				}
				error = 1;
			}
			else
			{
				if (background == 1)
				{
					if (!error) // & NOT PLACED AT END
					{
						fprintf(stderr, "Error: mislocated background sign\n");
					}
					error = 1;
				}
				userIn->arguments[counter] = cmd[0];
				if (!strcmp(cmd[0], "&")) // IF & DETECTED BY ITSELF, similar to above but does not replace the "&" char
				{
					userIn->arguments[counter] = NULL;
					background = 1;
					userInArray[userInArraycounter] = *userIn;
					userInArraycounter++;
				}

				if (!strcmp(cmd[0], "<")) // similar to above command but with "<"
				{
					left = (char*)malloc(512 * sizeof(char));
        				right = (char*)malloc(512 * sizeof(char));
					if (inRedirect(userIn, left, right) != 1)
					{
						cont = 2; // NEED TO CHANGE REDIRECTION
						break;
					}
					else
					{
						cont = 1; // IF inRedirect PASSES 1, MEANING ERROR, CONTINUE
					}
					
				}
				else if(!strcmp(cmd[0], ">")) 
        {
                        		left = (char*)malloc(512 * sizeof(char));
                        		right = (char*)malloc(512 * sizeof(char));
                        		if (outRedirect(userIn, left, right) != 1) {
                               			cont = 3;
                                		break;
        }
					else {
						cont = 1;
					}
                		}
			}
			cmd[0] = strtok(NULL, " "); // GET THE NEXT TOKEN, and repeat the loop
			counter++;
		}

		if (builtin(userIn, test) == 1 || cont == 1 || error == 1 || userIn->command[0] == NULL)
		{ // IF ANY OF THESE FLAGS MATCH, CONTINUE TO THE NEXT LOOP WITHOUT FORKING AND EXECVP'ING
			free(userIn);
			continue;
		}		

		pid = fork(); // create the child process...
		if(pid == 0)
		{
			if (cont == 2) // what happens if the input redirect flag is marked
			{
				int inFile = open(right, O_RDONLY);
				if (inFile == -1) // input file can't be openned
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
				}
				execvp(userIn->command[0], userIn->arguments);
				exit(1);

			}

                        else if(cont == 3) {
				int outFile = open(right, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
				if(outFile == -1)
				{
					perror("Error: cannot open output file\n");
					close(outFile);
					continue;
				}
				dup2(outFile, 1);
				close(outFile);
				for (int j = 0; j < 16; j++)
				{
					if (userIn->arguments[j] != NULL && !strcmp(userIn->arguments[j], ">"))
					{
						userIn->arguments[j] = NULL;
					}
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
			userIn->pid = pid;
			int status;
			if (background == 0) // if the current command is not a background command
			{
				status = waitpid(pid, &retval, 0);
				for (int m = 0; m < userInArraycounter; m++)
                                { // first check for any completed background functions
                                        if (userInArray[m].done == 1)
                                        {
                                                continue;
                                        }
                                        test = waitpid(-1, NULL, WNOHANG);
                                        if (test > 0)
                                        {
                                                userInArray[m].done = 1;
                                                fprintf(stderr, "+ completed '%s' [%d]\n", userInArray[m].rawInput, retval);
                                        }
                                }
				if  (retval == 256)
				{
					fprintf(stderr, "Error: command not found\n");
				}
				else if (status == pid)
				{
					fprintf(stderr, "+ completed '%s' [%d]\n", userIn->rawInput, retval);
				}
			}
			else if (background >= 1) // if it IS a background command, use the WNOHANG option
                        {
                                status = waitpid(-1, &retval, WNOHANG);
                                if  (retval == 256)
                                {
                                        fprintf(stderr, "Error: command not found\n");
                                }
                                else if (status > 0)
                                {
                                        fprintf(stderr, "+ completed '%s' [%d]\n", userIn->rawInput, retval);
                                }
                        }
		}
		free(userIn);
	}
	return EXIT_SUCCESS;
}