#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
	while(1){
		pid_t pid;
		char *cmd[2];
		size_t bufsize = 512;
		int status;

		cmd[0] = (char *) malloc(bufsize * sizeof(char));
		fprintf(stdout, "sshell$ ");
		int len = getline(&cmd[0], &bufsize, stdin);
		cmd[0][len - 1] = '\0';
		cmd[1] = NULL;
		pid = fork();

		if(pid == 0){
			execvp(cmd[0], cmd);
			perror("execvp");
			exit(1);
		} else {
			waitpid(-1, &status, 0);
			fprintf(stderr, "+ completed '%s' [%d]\n", cmd[0], WEXITSTATUS(status));
		}
	}
}
