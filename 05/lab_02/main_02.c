#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
int check_status(const int status)
{
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
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
int main(void)
{
    pid_t childpid[2];
    for (int i = 0; i < 2; i++)
    {
        if ((childpid[i] = fork()) == -1)
        {
            perror("Can't fork\n");
            exit(EXIT_FAILURE);
        }
        else if (childpid[i] == 0)
        {
            printf("Child: pid=%d, ppid=%d, grid=%d\n", getpid(), getppid(), getpgrp());
            // sleep(5);
	    exit(EXIT_SUCCESS);
        }
        else
        {
           printf("Parent: pid=%d, ppid=%d, grid=%d\n", getpid(), getppid(), getpgrp());
        }
    }
    int status;
    pid_t child_pid;
    for (int i = 0; i < 2; i++)
    {
        child_pid = waitpid(childpid[i], &status, WUNTRACED | WCONTINUED);
        printf("Child has finished: pid=%d\n", child_pid);
        if (check_status(status))
            exit(EXIT_FAILURE);
    }
    return 0;
}
