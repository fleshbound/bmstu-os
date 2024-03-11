#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <sys/wait.h>
#define MY_SOCK_PATH "test.soc"
#define handle_error(msg) \
           do { perror(msg); exit(1); } while (0)
int sock;
void sig_handler(int code)
{
	printf("catch signal %d\n", code);
	close(sock);
	unlink(MY_SOCK_PATH);
	exit(0);
}
int main()
{
	struct sockaddr srvr_addr;
	char buf[16];
	if (signal(SIGINT, sig_handler) == SIG_ERR)
		handle_error("signal");
	sock = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (sock == -1)
		handle_error("socket");
	memset(&srvr_addr, 0, sizeof(srvr_addr));
	srvr_addr.sa_family = AF_UNIX;
	strncpy(srvr_addr.sa_data, MY_SOCK_PATH, sizeof(srvr_addr.sa_data) - 1);
	if (bind(sock, &srvr_addr, strlen(srvr_addr.sa_data) + sizeof(srvr_addr.sa_family)) == -1)
		handle_error("bind");
	while (1)
	{
		if (recvfrom(sock, buf, sizeof(buf), 0, NULL, NULL) == -1)
			handle_error("recvfrom");
		printf("server %d receive: %s\n", getpid(), buf);
	}
}
