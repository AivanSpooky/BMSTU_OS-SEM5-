#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define N_BUF 26
#define N_PROD 3
#define N_CONS 4


#define SEM_BIN 0
#define SEM_EMPTY 1
#define SEM_FULL 2

int fl = 1;

struct sembuf start_produce[2] = {{ SEM_EMPTY, -1, 0 },{ SEM_BIN, -1, 0 }};
struct sembuf stop_produce[2] = {{ SEM_FULL, 1, 0 },{ SEM_BIN, 1, 0 }};
struct sembuf start_consume[2] = {{ SEM_FULL, -1, 0 },{ SEM_BIN, -1, 0 }};
struct sembuf stop_consume[2] = {{ SEM_EMPTY, 1, 0 },{ SEM_BIN, 1, 0 }};

void sig_handler(int sig_num)
{
    fl = 0;
    printf("pid %d, signal %d\n", getpid(), sig_num);
}

void producer(char **prod_ptr, char *chr, int semid)
{
    srand(getpid());
    while (fl)
    {
        sleep(rand() % 2);

        if (semop(semid, start_produce, 2) == -1)
        {
            perror("semop start_produce");
            exit(1);
        }

        if (*chr > 'z')
            *chr = 'a';
        **prod_ptr = *chr;

        printf("Producer %d: %c\n", getpid(), **prod_ptr);

        (*prod_ptr)++;
        (*chr)++;

        if (semop(semid, stop_produce, 2) == -1)
        {
            perror("semop stop_produce");
            exit(1);
        }
    }
    exit(EXIT_SUCCESS);
}

void consumer(char **cons_ptr, int semid)
{
    srand(getpid());
    while (fl)
    {
        sleep(rand() % 3);

        if (semop(semid, start_consume, 2) == -1)
        {
            perror("semop start_consume");
            exit(1);
        }

        printf("Consumer %d: %c\n", getpid(), **cons_ptr);

        (*cons_ptr)++;

        if (semop(semid, stop_consume, 2) == -1)
        {
            perror("semop stop_consume");
            exit(1);
        }
    }
    exit(EXIT_SUCCESS);
}

int main()
{
    int shmid, semid;
    int perms = S_IRUSR | S_IWUSR | S_IRGRP;
    char* buff, **memcons_ptr, **memprod_ptr, *prod_ptr, *cons_ptr, *alpha;

    if (signal(SIGINT, sig_handler) == SIG_ERR)
    {
        perror("signal");
        exit(1);
    }
    if ((shmid = shmget(100, 1024, IPC_CREAT | perms)) == -1)
    {
        perror("shmget");
        exit(1);
    }
    if ((buff = shmat(shmid, NULL, 0)) == (char*)-1)
    {
        perror("shmat");
        exit(1);
    }

    memcons_ptr = (char **) (buff);
    memprod_ptr = memcons_ptr + sizeof(char **);
    prod_ptr = (char *)(memprod_ptr + sizeof(char **));
    cons_ptr = prod_ptr;

    *(memcons_ptr) = cons_ptr;
    *(memprod_ptr) = prod_ptr; 

    alpha = prod_ptr - 1;
    *alpha = 'a';

    if ((semid = semget(100, 3, IPC_CREAT | perms)) == -1)
    {
        perror("semget\n");
        exit(1);
    }
    if (semctl(semid, SEM_BIN, SETVAL, 1) == -1)
    {
        perror("semctl sem_bin\n");
        exit(1);
    }
    if (semctl(semid, SEM_EMPTY, SETVAL, N_BUF) == -1)
    {
        perror("semctl sem_empty\n");
        exit(1);
    }
    if (semctl(semid, SEM_FULL, SETVAL, 0) == -1)
    {
        perror("semctl sem_full\n");
        exit(1);
    }

    pid_t cpids[N_CONS + N_PROD];
    for (int i = 0; i < N_PROD; i++)
    {
        if ((cpids[i] = fork()) == -1)
        {
            fprintf(stderr, "fork producer %d\n", i);
            exit(1);
        }
        if (cpids[i] == 0)
        {
            producer(memprod_ptr, alpha, semid);
        }
    }
    for (int i = N_PROD - 1; i < N_CONS + N_PROD; i++)
    {
        if ((cpids[i] = fork()) == -1)
        {
            fprintf(stderr, "fork consumer %d\n", i);
            exit(1);
        }
        if (cpids[i] == 0)
        {
            consumer(memcons_ptr, semid);
        }
    }

    for (int i = 0; i < (N_CONS + N_PROD); i++)
    {
        int wait_status;
        pid_t child_pid = waitpid(cpids[i], &wait_status, WUNTRACED);

        if (child_pid == -1)
        {
            perror("waitpid");
            exit(1);
        }
        printf("cpid %d; ", child_pid);
        if (WIFEXITED(wait_status))
        {
            printf("exited, status=%d\n", WEXITSTATUS(wait_status));
        } else if (WIFSIGNALED(wait_status))
        {
            printf("killed by signal %d\n", WTERMSIG(wait_status));
        } else if (WIFSTOPPED(wait_status))
        {
            printf("stopped by signal %d\n", WSTOPSIG(wait_status));
        }
    }

    if (shmdt((void*)prod_ptr) == -1)
    {
        perror("shmdt");
        exit(1);
    }
    if (semctl(semid, 1, IPC_RMID, NULL) == -1)
    {
        perror("semctl delete");
        exit(1);
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1)
    {
        perror("shmctl delete");
        exit(1);
    }
}