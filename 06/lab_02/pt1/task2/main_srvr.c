#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <sys/wait.h>
int sock;
void sig_handler(int code)
{
	printf("catch signal %d\n", code);
	close(sock);
	unlink("socket.srv");
	exit(0);
}
int main()
{
	struct sockaddr srvr_addr, clnt_addr;
	socklen_t clnt_addr_size;
	char buf[14], rply[32];
	
	if (signal(SIGINT, sig_handler) == SIG_ERR)
	{
		perror("socket");
		exit(1);
	}
		
	sock = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (sock == -1)
	{
		perror("socket");
		exit(1);
	}
		
	memset(&srvr_addr, 0, sizeof(srvr_addr));
	srvr_addr.sa_family = AF_UNIX;
	strncpy(srvr_addr.sa_data, "socket.srv", sizeof(srvr_addr.sa_data) - 1);
	
	if (bind(sock, &srvr_addr, sizeof(srvr_addr)) == -1)
	{
		perror("bind");
		exit(1);
	}
		
	while (1)
	{
		if (recvfrom(sock, buf, sizeof(buf), 0, NULL, NULL) == -1)
		{
			perror("recvfrom");
			exit(1);
		}
			
		printf("server %d receive: %s\n", getpid(), buf);
		sprintf(rply, "%s %d", buf, getpid());
		clnt_addr.sa_family = AF_UNIX;
		strncpy(clnt_addr.sa_data, buf, sizeof(clnt_addr.sa_data));
		
		if (sendto(sock, rply, strlen(rply) + 1, 0, &clnt_addr, sizeof(clnt_addr)) == -1)
		{
			perror("sendto");
			exit(1);
		}
			
		printf("server %d send to %s: %s\n", getpid(), buf, rply);
	}
}
