#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "pc.h"

void pc_prog_consumer(char *host)
{
	CLIENT *clnt;
	char  *result_2;
	char *consume_1_arg;

    clnt = clnt_create (host, PC_PROG, PC_VER, "udp");
    if (clnt == NULL) {
        clnt_pcreateerror (host);
        exit (1);
    }

	while (1)
	{
		sleep(rand() % 5 + 1);
		result_2 = consume_1((void*)&consume_1_arg, clnt);
		if (result_2 == (char *) NULL)
			clnt_perror (clnt, "call failed");
		else
			printf("Consumer -> %c\n", *result_2);
	}

    clnt_destroy (clnt);
}

void pc_prog_produser(char *host)
{
	CLIENT *clnt;
	void  *result_1;
	char *produce_1_arg;

    clnt = clnt_create (host, PC_PROG, PC_VER, "udp");
    if (clnt == NULL) {
        clnt_pcreateerror (host);
        exit (1);
    }

	while (1)
	{
		sleep(rand() % 1 + 1);
        result_1 = produce_1((void*)&produce_1_arg, clnt);
		if (result_1 == (void *) NULL)
			clnt_perror (clnt, "call failed");
	}

    clnt_destroy (clnt);
}


int main (int argc, char *argv[])
{
	char *host;
	int role;

	if (argc < 2) {
		printf ("usage: %s server_host\n", argv[0]);
		exit (1);
	}
	if (argc < 3) {
		printf ("usage: %s client hasn't role\n", argv[0]);
		exit (1);
	}
	role = atoi(argv[2]);
	if (role != 0 && role != 1) {
		printf ("usage: %s wrong client's role\n", argv[0]);
		exit (1);
	}
	host = argv[1];
	srand(time(NULL));

	if (role == 0)
		pc_prog_consumer(host);
	else
		pc_prog_produser(host);

	exit(0);
}
