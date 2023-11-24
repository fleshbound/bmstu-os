#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
int fl = 1;
#define WAIT_R 0
#define ACT_R 1
#define WAIT_W 2
#define BS 3
struct sembuf start_read[5] = {{WAIT_R, 1, 0}, {BS, 0, 0}, {WAIT_W, 0, 0}, {WAIT_R, -1, 0}, {ACT_R, 1, 0}};
struct sembuf stop_read[1] = {{ACT_R, -1, 0}};
struct sembuf start_write[4] = {{WAIT_W, 1, 0}, {ACT_R, 0, 0}, {BS, -1, 0}, {WAIT_W, -1, 0}};
struct sembuf stop_write[1] = {{BS, 1, 0}};
void check_w(pid_t w, const int wstatus)
{
    if (WIFEXITED(wstatus))
    {
        printf("%d exit, status=%d\n", w, WEXITSTATUS(wstatus));
    }
    else if (WIFSIGNALED(wstatus))
    {
        printf("%d terminate by signal %d\n", w, WTERMSIG(wstatus));
    }
    else if (WIFSTOPPED(wstatus))
    {
        printf("%d stop by signal %d\n", w, WSTOPSIG(wstatus));
    }
}
void sig_handler(const int sig_num)
{
    printf("pid=%d signal %d\n", getpid(), sig_num);
    fl = 0;
}
void reader(int *const addr, const int semid)
{
    srand(time(NULL));
    while (fl)
    {
        usleep((double)rand() / RAND_MAX * 1000000);
        if (semop(semid, start_read, 5) == -1)
        {
            perror("semop start_read");
            exit(1);
        }
        printf("reader pid=%d read : \e[1;49;32m%d\e[0m\n", getpid(), *addr);
        if (semop(semid, stop_read, 1) == -1)
        {
            perror("semop stop_read");
            exit(1);
        }
    }
    exit(0);
}
void writer(int *const addr, const int semid)
{
    srand(time(NULL));
    while (fl)
    {
        usleep((double)rand() / RAND_MAX * 1000000 + 1000);
        if (semop(semid, start_write, 4) == -1)
        {
            perror("semop start_write");
            exit(1);
        }
        printf("writer pid=%d write: \e[1;49;34m%d\e[0m\n", getpid(), ++(*addr));
        if (semop(semid, stop_write, 1) == -1)
        {
            perror("semop stop_write");
            exit(1);
        }
    }
    exit(0);
}
int main()
{
    pid_t wpid[3], rpid[5];
    if (signal(SIGINT, sig_handler) == SIG_ERR)
    {
        perror("signal");
        exit(1);
    }
    key_t semkey = ftok("/dev/null", 1);
    if (semkey == (key_t)-1)
    {
        perror("sem ftok");
        exit(1);
    }
    int perms = S_IRWXU | S_IRWXG | S_IRWXO;
    int semid = semget(semkey, 4, IPC_CREAT | perms);
    if (semid == -1)
    {
        perror("semget");
        exit(1);
    }
    if (semctl(semid, ACT_R, SETVAL, 0) == -1)
    {
        perror("semctl setval NW");
        exit(1);
    }
    if (semctl(semid, WAIT_W, SETVAL, 0) == -1)
    {
        perror("semctl setval NR");
        exit(1);
    }
    if (semctl(semid, BS, SETVAL, 1) == -1)
    {
        perror("semctl setval BS");
        exit(1);
    }
    if (semctl(semid, WAIT_R, SETVAL, 0) == -1)
    {
        perror("semctl setval QW");
        exit(1);
    }
    key_t shmkey = ftok("/dev/null", 1);
    if (shmkey == (key_t)-1)
    {
        perror("shm ftok");
        exit(1);
    }
    int shmid = shmget(shmkey, 4, IPC_CREAT | perms);
    if (shmid == -1)
    {
        perror("shmget");
        exit(1);
    }
    int *addr = (int *)shmat(shmid, 0, 0);
    if (addr == (int *)-1)
    {
        perror("shmat");
        exit(1);
    }
    *addr = 0;
    for (int i = 0; i < 5; i++)
    {
        if ((rpid[i] = fork()) == -1)
        {
            perror("reader fork");
            exit(1);
        }
        else if (rpid[i] == 0)
        {
            reader(addr, semid);
        }
    }
    for (int i = 0; i < 3; i++)
    {
        if ((wpid[i] = fork()) == -1)
        {
            perror("writer fork");
            exit(1);
        }
        else if (wpid[i] == 0)
        {
            writer(addr, semid);
        }
    }
    for (int i = 0; i < 8; i++)
    {
        int wstatus;
        pid_t w = wait(&wstatus);
        check_w(w, wstatus);
    }
    if (shmdt(addr) == -1)
    {
        perror("shmdt");
        exit(1);
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1)
    {
        perror("shmctl");
        exit(1);
    }
    if (semctl(semid, 0, IPC_RMID) == -1)
    {
        perror("semctl");
        exit(1);
    }
    return 0;
}
