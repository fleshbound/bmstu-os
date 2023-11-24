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
#define SE 0
#define SF 1
#define SB 2
int fl = 1;
struct sembuf p_lock[2] = {{SE, -1, 0}, {SB, -1, 0}};
struct sembuf p_unlock[2] = {{SB, 1, 0}, {SF, 1, 0}};
struct sembuf c_lock[2] = {{SF, -1, 0}, {SB, -1, 0}};
struct sembuf c_unlock[2] = {{SB, 1, 0}, {SE, 1, 0}};
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
void sig_handler(int sig_num)
{
    printf("pid=%d signal %d\n", getpid(), sig_num);
    fl = 0;
}
void consumer(char **cur_cons, int semfd)
{
    srand(time(NULL));
    while (fl)
    {
        usleep((double)rand() / RAND_MAX * 1000000);
        if (semop(semfd, c_lock, 2) == -1)
        {
            perror("semop c_lock");
            exit(1);
        }
        char s = *(*cur_cons);
        (*cur_cons)++;
        printf("consumer pid=%d read  %c\n", getpid(), s);
        if (semop(semfd, c_unlock, 2) == -1)
        {
            perror("semop c_unlock");
            exit(1);
        }
    }
    exit(0);
}
void producer(char **cur_prod, char *sym, int semfd)
{
    srand(time(NULL));
    while (fl)
    {
        usleep((double)rand() / RAND_MAX * 1000000);
        if (semop(semfd, p_lock, 2) == -1)
        {
            perror("semop p_lock");
            exit(1);
        }
        **cur_prod = *sym;
        (*cur_prod)++;
        printf("producer pid=%d write %c\n", getpid(), *sym);
        (*sym)++;
        if (*sym > 'z')
            *sym = 'a';
        if (semop(semfd, p_unlock, 2) == -1)
        {
            perror("semop p_unlock");
            exit(1);
        }
    }
    exit(0);
}
int main()
{
    char **cur_prod, **cur_cons, *sym;
    pid_t conspid[3], prodpid[3];
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
    int semfd = semget(semkey, 3, IPC_CREAT | perms);
    if (semfd == -1)
    {
        perror("semget");
        exit(1);
    }
    if (semctl(semfd, SE, SETVAL, 26) == -1)
    {
        perror("semctl setval se");
        exit(1);
    }
    if (semctl(semfd, SF, SETVAL, 0) == -1)
    {
        perror("semctl setval sf");
        exit(1);
    }
    if (semctl(semfd, SB, SETVAL, 1) == -1)
    {
        perror("semctl setval sb");
        exit(1);
    }
    key_t shmkey = ftok("/dev/null", 1);
    if (shmkey == (key_t)-1)
    {
        perror("shm ftok");
        exit(1);
    }
    int shmfd = shmget(shmkey, 1024, IPC_CREAT | perms);
    if (shmfd == -1)
    {
        perror("shmget");
        exit(1);
    }
    char *addr = (char *)shmat(shmfd, 0, 0);
    if (addr == (char *)-1)
    {
        perror("shmat");
        exit(1);
    }
    cur_prod = addr;
    cur_cons = cur_prod + sizeof(char *);
    sym = cur_cons + sizeof(char *);
    *cur_prod = sym + sizeof(char);
    *cur_cons = *cur_prod;
    *sym = 'a';
    for (int i = 0; i < 3; i++)
    {
        if ((prodpid[i] = fork()) == -1)
        {
            perror("producer fork");
            exit(1);
        }
        else if (prodpid[i] == 0)
        {
            producer(cur_prod, sym, semfd);
        }
    }
    for (int i = 0; i < 3; i++)
    {
        if ((conspid[i] = fork()) == -1)
        {
            perror("consumer fork");
            exit(1);
        }
        else if (conspid[i] == 0)
        {
            consumer(cur_cons, semfd);
        }
    }
    for (int i = 0; i < 6; i++)
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
    if (shmctl(shmfd, IPC_RMID, NULL) == -1)
    {
        perror("shmctl");
        exit(1);
    }
    if (semctl(semfd, 0, IPC_RMID) == -1)
    {
        perror("semctl");
        exit(1);
    }
    return 0;
}
