#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{	
	if (argc != 3)
	{
		printf("Error! Argc needs to be 3!\n");
		return EXIT_FAILURE;
	}
	pid_t childpid[2], w;
	int wstatus;
	static char* newenviron[] = { NULL };
	static char* newargv[] = { NULL };
	for (int i = 0; i < 2; i++)
	{
		if ((childpid[i] = fork()) == -1)
		{
			printf("Can't fork!\n");
			exit(1);
		}
		else if (childpid[i] == 0)
		{
			// printf("Child[%d]: (pid=%d)\n", i, getpid());
			if (execve(argv[i + 1], newargv, newenviron) == -1)
			{
				printf("Can't exec %d!\n", childpid[i]);
				exit(1);
			}
		}
	}
	for (int i = 0; i < 2; i++)
	{	
		waitpid(childpid[i], &wstatus, WUNTRACED | WCONTINUED);
		if (WIFEXITED(wstatus))
			printf("exited, status=%d\n", WEXITSTATUS(wstatus));
		else if (WIFSIGNALED(wstatus))
			printf("killed by signal %d\n", WTERMSIG(wstatus));
		else if (WIFSTOPPED(wstatus))
			printf("stopped by signal %d\n", WSTOPSIG(wstatus));
		else if (WIFCONTINUED(wstatus))
			printf("continued\n");
	}
	return 0;
}