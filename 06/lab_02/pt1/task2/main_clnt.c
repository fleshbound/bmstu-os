#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/unistd.h>
#define MY_SOCK_PATH "test.soc"
#define handle_error(msg) \
           do { perror(msg); exit(1); } while (0)
int main()
{
	int sock;
	struct sockaddr srvr_addr, clnt_addr;
	char name[16], buf[16], msg[32];
	sock = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (sock == -1)
		handle_error("socket");
	memset(&clnt_addr, 0, sizeof(clnt_addr));
	clnt_addr.sa_family = AF_UNIX;
	sprintf(name, "%d%c", getpid(), '\0');
	strncpy(clnt_addr.sa_data, name, sizeof(clnt_addr.sa_data) - 1);
	if (bind(sock, &clnt_addr, sizeof(clnt_addr)) == -1)
		handle_error("bind");
	memset(&srvr_addr, 0, sizeof(srvr_addr));
	srvr_addr.sa_family = AF_UNIX;
	strncpy(srvr_addr.sa_data, MY_SOCK_PATH, sizeof(srvr_addr.sa_data) - 1);
	sprintf(buf, "%d%c", getpid(), '\0');
	if (sendto(sock, buf, strlen(buf) + 1, 0, &srvr_addr, sizeof(srvr_addr)) == -1)
		handle_error("sendto");
	printf("client %d send: %s\n", getpid(), buf);
	if (recvfrom(sock, msg, sizeof(msg), 0, NULL, NULL) == -1)
		handle_error("recvfrom");
	printf("client %d receive: %s\n", getpid(), msg);
	if (close(sock) == -1)
		handle_error("close");
	if (unlink(name) == -1)
		handle_error("unlink");
	return 0;
}
