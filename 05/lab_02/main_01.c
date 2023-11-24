#include<sys/types.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
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
            sleep(3);
            printf("Child: pid=%d, ppid=%d, grid=%d\n", getpid(), getppid(), getpgrp());
            exit(EXIT_SUCCESS);
        }
        else
        {
           printf("Parent: pid=%d, ppid=%d, grid=%d\n", getpid(), getppid(), getpgrp());
        }
    }
    return 0;
}
