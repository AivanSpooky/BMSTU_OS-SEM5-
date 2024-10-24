#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define str1 "xxxxx"
#define str2 "yyyyyyyyyyyyyyy"

int main()
{
	char* buf;
    char* msg[2] = {str1, str2};
	int lens[2] = {sizeof(str1) - 1, sizeof(str2) - 1};
    int fd[2];
	int childpid[2];
	int wstatus;

    if (pipe(fd) == -1)
    {
        perror("Can't pipe.");
        exit(EXIT_FAILURE);
    }

	for (int i = 0; i < 2; i++)
	{
		if ((childpid[i] = fork()) == -1)
		{
			printf("Can't fork!\n");
			exit(EXIT_FAILURE);
		}
		else if (childpid[i] == 0)
		{
            close(fd[0]);
            write(fd[1], msg[i], lens[i]);
            printf("(pid = %d) sent message %s\n", getpid(), msg[i]);
            exit(EXIT_SUCCESS);
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

	for (int i = 0; i < 2; i++)
	{
		buf = "";
		buf = malloc(lens[i]);
		close(fd[1]);
		read(fd[0], buf, lens[i]);
		printf("received: %s\n", buf);
	}

	buf = "";
	buf = malloc(lens[1]);
	close(fd[1]);
	read(fd[0], buf, lens[1]);
	printf("received: %s\n", buf);

	return 0;
}