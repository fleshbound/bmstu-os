#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/unistd.h>
int main()
{
	int sock;
	struct sockaddr srvr_addr, clnt_addr;
	char name[14], msg[32];
	sock = socket(AF_UNIX, SOCK_DGRAM, 0);

	if (sock == -1)
	{
		perror("socket");
		exit(1);
	}
		
	memset(&clnt_addr, 0, sizeof(clnt_addr));
	clnt_addr.sa_family = AF_UNIX;

	char buf[14];
	sprintf(buf, "%d.cln", getpid());
	strncpy(clnt_addr.sa_data, buf, sizeof(clnt_addr.sa_data) - 1);

	if (bind(sock, &clnt_addr, sizeof(clnt_addr)) == -1)
	{
		perror("bind");
		exit(1);
	}
		
	memset(&srvr_addr, 0, sizeof(srvr_addr));
	srvr_addr.sa_family = AF_UNIX;
	strncpy(srvr_addr.sa_data, "socket.srv", sizeof(srvr_addr.sa_data) - 1);

	if (sendto(sock, buf, strlen(buf) + 1, 0, &srvr_addr, sizeof(srvr_addr)) == -1)
	{
		perror("sendto");
		exit(1);
	}
		
	printf("client %d send: %s\n", getpid(), buf);

	if (recvfrom(sock, msg, sizeof(msg), 0, NULL, NULL) == -1)
	{
		perror("recvfrom");
		exit(1);
	}	

	printf("client %d receive: %s\n", getpid(), msg);
	close(sock);
	unlink(buf);
	return 0;
}
