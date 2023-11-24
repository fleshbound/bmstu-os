#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
int fl = 0;
int check_status(const int status)
{
    int status = 0;
    if (WIFEXITED(status))
    {
        printf("Child exited with code %d\n", WEXITSTATUS(status));
    }
    else if (WIFSIGNALED(status))
    {
        printf("Child terminated by signal %d\n", WTERMSIG(status));
    }
    else if (WIFSTOPPED(status))
    {
        printf("Child stopped by signal %d\n", WSTOPSIG(status));
    }
    else
    {
        printf("Child exited abnormally.\n");
        status = 1;
    }
    return status;
}
int sig_handler(int sig_num)
{
    fl = 1;
}
int main(void)
{
    int fd[2];
    pid_t childpid[2];
    if (signal(SIGINT, sig_handler) == -1)
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
                write(fd[1], "aaa\n");
            }
            else if (i == 1)
            {
                close(fd[0]);
                write(fd[1], "yyyyyyy\n");
            }
            exit(EXIT_SUCCESS);
        }
        else
        {
            pause();
        }
    }
    for (int i = 0; i < 2; i++)
    {
        int status;
        pid_t child_pid;
        child_pid = waitpid(childpid[i], &status, WUNTRACED | WCONTINUED);
        if (check_status(status))
            exit(EXIT_FAILURE);
    }
    printf("Parent (pid=%d) recieved msg: ", getpid());
    char sym;
    if (flag == 1)
    {
        do
        {
            close(fd[1]);
            read(fd[0], &sym, 1);
            printf("%c", sym);
        }
        while (sym != '\n');
    }
    return EXIT_SUCCESS;
}
