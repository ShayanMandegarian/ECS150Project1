#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

typedef struct parsedCmd parsedCmd;

struct parsedCmd {
	char *cmd[16]; 
};

void parseCmd(struct parsedCmd *obj, char *cmd){
	int i = 0;
	char *temp = strtok(cmd, " ");
	
	while(temp != NULL){
		obj->cmd[i++] = temp;
		temp = strtok(NULL, " ");
	}
	obj->cmd[i] = NULL;
}

int main(int argc, char *argv[])
{
	pid_t pid;
	char *cmd;
	size_t bufsize = 512;
	int status;
	int len;
	char *origCmd;
	parsedCmd pCmd;
	cmd = (char *) malloc(bufsize * sizeof(char));
	
	while(1){
		fprintf(stdout, "sshell$ ");
		len = getline(&cmd, &bufsize, stdin);
		cmd[len - 1] = '\0';
		origCmd = strdup(cmd);
		parseCmd(&pCmd, cmd);
		pid = fork();

		if(pid == 0){
			execvp(pCmd.cmd[0], pCmd.cmd);
			perror("Error");
			exit(1);
		} else {
			waitpid(-1, &status, 0);
			fprintf(stderr, "+ completed '%s' [%d]\n", origCmd, WEXITSTATUS(status));
		}
	}
}
