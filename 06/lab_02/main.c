#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/socket.h>
#include <time.h>
#define N 1
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
int main(void)
{
    int sockets[2];
    pid_t childpid[N];
	char buf[64];
	if (socketpair(AF_UNIX, SOCK_DGRAM, 0, sockets) != 0)
	{
		perror("socketpair");
		return 1;
	}
    for (int i = 0; i < N; i++)
    {
        if ((childpid[i] = fork()) == -1)
        {
            perror("Can't fork");
            exit(1);
        }
        else if (childpid[i] == 0)
        {
			char msg[16];
			sprintf(msg, "%d%c", getpid(), '\0');
			close(sockets[0]);
			write(sockets[1], msg, sizeof(msg));
			printf("child  %d send: %s\n", getpid(), msg);
			sleep(1);
			read(sockets[1], buf, sizeof(buf));
			printf("child  %d read: %s\n", getpid(), buf);
			exit(0);
        }
    }
	close(sockets[1]);
    for (int i = 0; i < N; i++)
	{
		char msg[16];
		sprintf(msg, "%d%c", getpid(), '\0');
		close(sockets[1]);
		read(sockets[0], buf, sizeof(buf));
		printf("parent %d read: %s\n", getpid(), buf);
		write(sockets[0], msg, sizeof(msg));
		printf("parent %d send: %s\n", getpid(), msg);
	}
	close(sockets[0]);
	for (int i = 0; i < N; i++)
    {
        int wstatus;
        pid_t w = wait(&wstatus);
        check_w(w, wstatus);
    }
	return 0;
}
