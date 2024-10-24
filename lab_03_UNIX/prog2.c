#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
	pid_t childpid[2], w;
	int wstatus;
	for (int i = 0; i < 2; i++)
	{
		if ((childpid[i] = fork()) == -1)
			exit(EXIT_FAILURE);
		else if (childpid[i] == 0)
		{
			printf("Child[%d]: pid=%d, ppid=%d, gid=%d\n", i, getpid(), getppid(), getgid());
			while (1)
                continue;
			printf("After sleep\nChild[%d]: pid=%d, ppid=%d, gid=%d\n", i, getpid(), getppid(), getgid());
			exit(EXIT_SUCCESS);
		}
		else
			printf("Parent: pid=%d, ppid=%d, gid=%d\n", getpid(), getppid(), getpgrp());
	}
	for (int i = 0; i < 2; i++)
	{	
            w = waitpid(childpid[i], &wstatus, WUNTRACED | WCONTINUED);
            if (w == -1)
            {
                perror("waitpid");
                exit(EXIT_FAILURE);
            }
            if (WIFEXITED(wstatus))
                printf("exited, status=%d\n", WEXITSTATUS(wstatus));
            else if (WIFSIGNALED(wstatus))
                printf("killed by signal %d\n", WTERMSIG(wstatus));
            else if (WIFSTOPPED(wstatus))
                printf("stopped by signal %d\n", WSTOPSIG(wstatus));
	}
    return 0;
}