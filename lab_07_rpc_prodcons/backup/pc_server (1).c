#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include "pc.h"

#define SIZE_BUF 256

#define BIN_SEM 0
#define BUFF_FULL 1
#define BUFF_EMPTY 2

#define P -1
#define V  1

struct sembuf start_produce[2] = {{BUFF_EMPTY, P, 0}, {BIN_SEM, P, 0}};
struct sembuf stop_produce[2] =  {{BIN_SEM, V, 0}, {BUFF_FULL, V, 0}};
struct sembuf start_consume[2] = {{BUFF_FULL, P, 0}, {BIN_SEM, P, 0}};
struct sembuf stop_consume[2] =  {{BIN_SEM, V, 0}, {BUFF_EMPTY, V, 0}};

static char buffer[SIZE_BUF];
static int produce_index = 0;
static int consume_index = 0;
static char alpha_produce = 'a';

void *produce_1_svc(void *argp, struct svc_req *rqstp)
{
	static char * result;

    if (semop(semid, start_produce, 2) == -1)
    {
        perror("Can't semop (start_produce)\n");
        exit(1);
    }

    *(buffer + produce_index) = alpha_produce;
    produce_index++;
    if (alpha_produce == 'z')
        alpha_produce = 'a';
    else
        alpha_produce++;
    printf("Producer -> %c\n", alpha_produce);

    if (semop(semid, stop_produce, 2) == -1)
    {
        perror("Can't semop (stop_produce)\n");
        exit(1);
    }

	return (void *) &result;
}

char *consume_1_svc(void *argp, struct svc_req *rqstp)
{
	static char result;

    if (semop(semid, start_consume, 2) == -1)
    {
        perror("Can't semop (start_consume)\n");
        exit(1);
    }

	result = *(buffer + consume_index);
    printf("Consumer -> %c\n", result);
    consume_index++;

    if (semop(semid, stop_consume, 2) == -1)
    {
        perror("Can't semop (stop_consume)\n");
        exit(1);
    }

	return &result;
}
