#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define SIZE_BUF 26
#define PRODUCERS_AMOUNT 3
#define CONSUMERS_AMOUNT 6

static int fl;
// void sig_handler(int sig_numb)
// {
//     fl = 0;
// }

#define SB 0
#define SE 1
#define SF 2

#define P -1
#define V  1

struct sembuf start_produce[2] = { {SE, P, 0}, {SB, P, 0} };
struct sembuf stop_produce[2] =  { {SB, V, 0}, {SF, V, 0} };
struct sembuf start_consume[2] = { {SF, P, 0}, {SB, P, 0} };
struct sembuf stop_consume[2] =  { {SB, V, 0}, {SE, V, 0} };

int semid, shmid;

void signal_handler(int sig)
{
    fl = 0;
    printf("Process %d received signal %d.\n", getpid(), sig);

    exit(0);
}

void producer(const int semid, char *buf)
{
    char **prod_ptr = (char **)buf;
    char *alpha_ptr = (char *)(buf + 2 * sizeof(char **));

    srand(getpid());
    while(*alpha_ptr <= 'z')
    {
        if (semop(semid, start_produce, 2) == -1)
        {
            fprintf(stderr, "Can't semop: %s\n", strerror(errno));
            exit(1);
        }

        **prod_ptr = *alpha_ptr;
        printf("Producer %d put %c\n", getpid(), **prod_ptr);

        (*prod_ptr)++;
        (*alpha_ptr)++;

        sleep(rand() % 2);

        if (semop(semid, stop_produce, 2) == -1)
        {
            fprintf(stderr, "Can't semop: %s\n", strerror(errno));
            exit(1);
        }
    }

    exit(0);
}

void consumer(const int semid, char *buf)
{
    char **cons_ptr = (char **)(buf + sizeof(char **));

    srand(getpid());
    while(**cons_ptr <= 'z')
    {
        if (semop(semid, start_consume, 2) == -1)
        {
            fprintf(stderr, "Can't semop: %s\n", strerror(errno));
            exit(1);
        }

        printf("Consumer %d got %c\n", getpid(), **cons_ptr);
        (*cons_ptr)++;
        sleep(rand() % 2);

        if (semop(semid, stop_consume, 2) == -1)
        {
            fprintf(stderr, "Can't semop: %s\n", strerror(errno));
            exit(1);
        }
    }

    exit(0);
}

int main()
{
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        fprintf(stderr, "Can't set SIGINT handler: %s\n", strerror(errno));
        exit(1);
    }
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        fprintf(stderr, "Can't set SIGTERM handler: %s\n", strerror(errno));
        exit(1);
    }

    int perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    key_t shmkey = ftok("./key.txt", 1);
    if (shmkey == -1) {
        fprintf(stderr, "Can't ftok for shm: %s\n", strerror(errno));
        exit(1);
    }

    if ((shmid = shmget(shmkey, (SIZE_BUF + 3) * sizeof(char), IPC_CREAT | perms)) == -1) {
        fprintf(stderr, "Can't shmget: %s\n", strerror(errno));
        exit(1);
    }

    char *buf = shmat(shmid, NULL, 0);
    if (buf == (char *) -1) {
        fprintf(stderr, "Can't shmat: %s\n", strerror(errno));
        exit(1);
    }

    char **prod_ptr = (char **)buf;
    char **cons_ptr = (char **)(buf + sizeof(char **));
    char *alpha_ptr = (char *)(buf + 2 * sizeof(char **));

    *alpha_ptr = 'a';
    char *buffer_start = (char *)(alpha_ptr + 1);
    *prod_ptr = buffer_start;
    *cons_ptr = buffer_start;

    key_t semkey = ftok("./key.txt", 1);
    if (semkey == -1) {
        fprintf(stderr, "Can't ftok for sem: %s\n", strerror(errno));
        exit(1);
    }

    if ((semid = semget(semkey, 3, IPC_CREAT | perms)) == -1) {
        fprintf(stderr, "Can't semget: %s\n", strerror(errno));
        exit(1);
    }

    if (semctl(semid, SB, SETVAL, 1) == -1) {
        fprintf(stderr, "Can't semctl for SB: %s\n", strerror(errno));
        exit(1);
    }
    if (semctl(semid, SE, SETVAL, SIZE_BUF) == -1) {
        fprintf(stderr, "Can't semctl for SE: %s\n", strerror(errno));
        exit(1);
    }
    if (semctl(semid, SF, SETVAL, 0) == -1) {
        fprintf(stderr, "Can't semctl for SF: %s\n", strerror(errno));
        exit(1);
    }

    pid_t chpid[CONSUMERS_AMOUNT + PRODUCERS_AMOUNT - 1];

    for (int i = 0; i < PRODUCERS_AMOUNT - 1; i++) {
        chpid[i] = fork();
        if (chpid[i] == -1) {
            fprintf(stderr, "Can't fork producer: %s\n", strerror(errno));
            exit(1);
        }

        if (chpid[i] == 0) {
            producer(semid, buf);
        }
    }

    for (int i = PRODUCERS_AMOUNT - 1; i < CONSUMERS_AMOUNT + PRODUCERS_AMOUNT - 1; i++) {
        chpid[i] = fork();
        if (chpid[i] == -1) {
            fprintf(stderr, "Can't fork consumer: %s\n", strerror(errno));
            exit(1);
        }

        if (chpid[i] == 0) {
            consumer(semid, buf);
        }
    }

    srand(getpid());
    while (*alpha_ptr <= 'z') {
        if (semop(semid, start_produce, 2) == -1) {
            fprintf(stderr, "Can't semop: %s\n", strerror(errno));
            exit(1);
        }

        **prod_ptr = *alpha_ptr;
        printf("Producer %d put %c\n", getpid(), **prod_ptr);

        (*prod_ptr)++;
        (*alpha_ptr)++;

        sleep(rand() % 2);

        if (semop(semid, stop_produce, 2) == -1) {
            fprintf(stderr, "Can't semop: %s\n", strerror(errno));
            exit(1);
        }
    }

    for (int i = 0; i < (CONSUMERS_AMOUNT + PRODUCERS_AMOUNT - 1); i++) {
        int status;
        if (waitpid(chpid[i], &status, WUNTRACED) == -1) {
            fprintf(stderr, "Can't waitpid: %s\n", strerror(errno));
            exit(1);
        }

        if (WIFEXITED(status))
            printf("Child %d has finished, code: %d\n", chpid[i], WEXITSTATUS(status));
        else if (WIFSIGNALED(status))
            printf("Child %d finished by unhandled signal, signum: %d\n", chpid[i], WTERMSIG(status));
    }

    if (shmdt((void *) buf) == -1) {
        fprintf(stderr, "Can't shmdt: %s\n", strerror(errno));
        exit(1);
    }

    if (semctl(semid, 1, IPC_RMID, NULL) == -1) {
        fprintf(stderr, "Can't delete semaphore: %s\n", strerror(errno));
        exit(1);
    }

    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        fprintf(stderr, "Can't mark a segment as deleted: %s\n", strerror(errno));
        exit(1);
    }

    return 0;
}