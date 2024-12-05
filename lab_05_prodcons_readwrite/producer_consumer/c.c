#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#define PROD_CNT 3
#define CONS_CNT 4

#define SEMPTY 0
#define SFULL 1
#define SBINARY 2

#define P -1
#define V 1

struct sembuf producer_lock[2] = { {SEMPTY, P, 0}, {SBINARY, P, 0} };
struct sembuf producer_release[2] = { {SBINARY, V, 0}, {SFULL, V, 0} };
struct sembuf consumer_lock[2] = { {SFULL, P, 0}, {SBINARY, P, 0} };
struct sembuf consumer_release[2] = { {SBINARY, V, 0}, {SEMPTY, V, 0} };

int fl = 1;

void sig_handler(int sig_num)
{
    fl = 0;
    printf("Process %d caught signal %d\n", getpid(), sig_num);
}

void producer(char *addr, int semid, char ***prod_ptr, char ***cons_ptr)
{
    srand(getpid());
    while (fl)
    {
        sleep(rand() % 2);
        
        if (semop(semid, producer_lock, 2) == -1)
        {
            fprintf(stderr, "Producer %d: semop lock error: %s\n", getpid(), strerror(errno));
            exit(1);
        }

        char ch = 'a' + (**prod_ptr - (addr + 2 * sizeof(char *))) % 26;
        ***prod_ptr = ch;
        printf("Producer %d put: %c\n", getpid(), ch);
        (**prod_ptr)++;

        if (semop(semid, producer_release, 2) == -1)
        {
            fprintf(stderr, "Producer %d: semop release error: %s\n", getpid(), strerror(errno));
            exit(1);
        }
    }

    exit(0);
}

void consumer(char *addr, int semid, char ***prod_ptr, char ***cons_ptr)
{
    srand(getpid());
    while (fl)
    {
        sleep(rand() % 2);
        
        if (semop(semid, consumer_lock, 2) == -1)
        {
            fprintf(stderr, "Consumer %d: semop lock error: %s\n", getpid(), strerror(errno));
            exit(1);
        }

        char ch = ***cons_ptr;
        printf("Consumer %d got: %c\n", getpid(), ch);
        (**cons_ptr)++;

        if (semop(semid, consumer_release, 2) == -1)
        {
            fprintf(stderr, "Consumer %d: semop release error: %s\n", getpid(), strerror(errno));
            exit(1);
        }
    }

    exit(0);
}

int main()
{
    struct sigaction sa;
    sa.sa_handler = sig_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        fprintf(stderr, "Can't set SIGINT handler: %s\n", strerror(errno));
        exit(1);
    }

    int perms = S_IRWXU | S_IRWXG | S_IRWXO;

    int shm_fd = shmget(IPC_PRIVATE, 4096, IPC_CREAT | perms);
    if (shm_fd == -1)
    {
        fprintf(stderr, "shmget error: %s\n", strerror(errno));
        exit(1);
    }

    char *addr = (char *)shmat(shm_fd, NULL, 0);
    if (addr == (char *)-1)
    {
        fprintf(stderr, "shmat error: %s\n", strerror(errno));
        exit(1);
    }

    char **cons_cur = (char **)addr;
    char **prod_cur = (char **)(addr + sizeof(char *));
    char *buffer_start = addr + 2 * sizeof(char *);
    *cons_cur = buffer_start;
    *prod_cur = buffer_start;

    char *alpha_ptr = addr + 3 * sizeof(char *);
    *alpha_ptr = 'a';

    int sem_fd = semget(IPC_PRIVATE, 3, IPC_CREAT | perms);
    if (sem_fd == -1)
    {
        fprintf(stderr, "semget error: %s\n", strerror(errno));
        shmdt(addr);
        exit(1);
    }

    if (semctl(sem_fd, SBINARY, SETVAL, 1) == -1)
    {
        fprintf(stderr, "semctl SBINARY error: %s\n", strerror(errno));
        shmdt(addr);
        semctl(sem_fd, 0, IPC_RMID);
        exit(1);
    }
    if (semctl(sem_fd, SEMPTY, SETVAL, 26) == -1)
    {
        fprintf(stderr, "semctl SEMPTY error: %s\n", strerror(errno));
        shmdt(addr);
        semctl(sem_fd, 0, IPC_RMID);
        exit(1);
    }
    if (semctl(sem_fd, SFULL, SETVAL, 0) == -1)
    {
        fprintf(stderr, "semctl SFULL error: %s\n", strerror(errno));
        shmdt(addr);
        semctl(sem_fd, 0, IPC_RMID);
        exit(1);
    }

    pid_t child_pids[PROD_CNT + CONS_CNT];
    int idx = 0;

    for (short i = 0; i < PROD_CNT; ++i)
    {
        pid_t child_pid = fork();
        if (child_pid == -1)
        {
            fprintf(stderr, "can't fork producer: %s\n", strerror(errno));
            for (int j = 0; j < idx; ++j)
                kill(child_pids[j], SIGINT);
            shmdt(addr);
            semctl(sem_fd, 0, IPC_RMID);
            exit(1);
        }
        else if (child_pid == 0)
        {
            producer(addr, sem_fd, &prod_cur, &cons_cur);
        }
        else
        {
            child_pids[idx++] = child_pid;
        }
    }

    for (short i = 0; i < CONS_CNT; ++i)
    {
        pid_t child_pid = fork();
        if (child_pid == -1)
        {
            fprintf(stderr, "can't fork consumer: %s\n", strerror(errno));
            for (int j = 0; j < idx; ++j)
                kill(child_pids[j], SIGINT);
            shmdt(addr);
            semctl(sem_fd, 0, IPC_RMID);
            exit(1);
        }
        else if (child_pid == 0)
        {
            consumer(addr, sem_fd, &prod_cur, &cons_cur);
        }
        else
        {
            child_pids[idx++] = child_pid;
        }
    }

    int wstatus;
    for (short i = 0; i < PROD_CNT + CONS_CNT; ++i)
    {
        pid_t w = wait(&wstatus);
        if (w == -1)
        {
            fprintf(stderr, "wait error: %s\n", strerror(errno));
            exit(1);
        }

        if (WIFEXITED(wstatus))
            printf("Exited %d status=%d\n", w, WEXITSTATUS(wstatus));
        else if (WIFSIGNALED(wstatus))
            printf("Killed %d by signal %d\n", w, WTERMSIG(wstatus));
        else if (WIFSTOPPED(wstatus))
            printf("Stopped %d by signal %d\n", w, WSTOPSIG(wstatus));
    }

    if (shmdt(addr) == -1)
    {
        fprintf(stderr, "shmdt error: %s\n", strerror(errno));
    }

    if (shmctl(shm_fd, IPC_RMID, NULL) == -1)
    {
        fprintf(stderr, "shmctl error: %s\n", strerror(errno));
    }

    if (semctl(sem_fd, 0, IPC_RMID) == -1)
    {
        fprintf(stderr, "semctl IPC_RMID error: %s\n", strerror(errno));
        exit(1);
    }

    return 0;
}