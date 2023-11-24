#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
int fl = 0;
void check_status(const int w)
{
    int s = 0;
    if (WIFEXITED(w))
    {
        printf("Child exited with code %d\n", WEXITSTATUS(w));
    }
    else if (WIFSIGNALED(w))
    {
        printf("Child terminated by signal %d\n", WTERMSIG(w));
    }
    else if (WIFSTOPPED(w))
    {
        printf("Child stopped by signal %d\n", WSTOPSIG(w));
    }
    else
    {
        printf("Child exited abnormally.\n");
        exit(EXIT_FAILURE);
    }
}
void sig_handler(int sig_num)
{
    fl = 1;
}
int main(void)
{
    int fd[2];
    pid_t childpid[2];
    if (signal(SIGINT, sig_handler) == SIG_ERR)
    {
        perror("Can't signal\n");
        exit(EXIT_FAILURE);
    }
    if (pipe(fd) == -1)
    {
        perror("Can't pipe\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < 2; i++)
    {
        if ((childpid[i] = fork()) == -1)
        {
            perror("Can't fork\n");
            exit(EXIT_FAILURE);
        }
        else if (childpid[i] == 0)
        {
            if (i == 0)
            {
                close(fd[0]);
                write(fd[1], "aaa\n", 4);
                printf("Child (pid=%d) sent msg: aaa\n", getpid());
            }
            else if (i == 1)
            {
                close(fd[0]);
                write(fd[1], "yyyyyyy\n", 8);
                printf("Child (pid=%d) sent msg: yyyyyyy\n", getpid());
            }
            pause();
            exit(EXIT_SUCCESS);
        }
        else
        {
        }
    }
    for (int i = 0; i < 2; i++)
    {
        int w;
        pid_t child_pid;
        child_pid = waitpid(childpid[i], &w, WUNTRACED | WCONTINUED);
        check_status(w);
    }
    for (int i = 0; i < 3; i++)
    {
        printf("Parent (pid=%d) recieved msg: ", getpid());
        char sym;
        if (fl == 1)
        {
            do
            {
                close(fd[1]);
                read(fd[0], &sym, 1);
                printf("%c", sym);
            }
            while (sym != '\n');
        }
    }
    return EXIT_SUCCESS;
}
