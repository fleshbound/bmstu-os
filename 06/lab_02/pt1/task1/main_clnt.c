#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/unistd.h>
#define handle_error(msg) \
           do { perror(msg); exit(1); } while (0)
int main()
{
	int sock;
	struct sockaddr srvr_addr;
	socklen_t srvr_addr_size;
	char buf[14];
	ssize_t bytes;
	sock = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (sock == -1)
		handle_error("socket");
	memset(&srvr_addr, 0, sizeof(srvr_addr));
	srvr_addr.sa_family = AF_UNIX;
	strncpy(srvr_addr.sa_data, "socket_srv.soc", sizeof(srvr_addr.sa_data) - 1);
	sprintf(buf, "%d%c", getpid(), '\0');
	if (sendto(sock, buf, sizeof(buf), 0, &srvr_addr, sizeof(srvr_addr)) == -1)
		   handle_error("sendto");
	printf("client %d send: %s\n", getpid(), buf);
	if (close(sock) == -1)
		handle_error("close");
	return 0;
}
