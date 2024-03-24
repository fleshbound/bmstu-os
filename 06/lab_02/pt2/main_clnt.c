#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFF_SIZE 32

int main()
{
    int port = 5000;
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == 1)
    {
        perror("sock error");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serv_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port)
    };

    if (connect(sock, &serv_addr, sizeof(serv_addr)) == -1) 
    {
        perror("connect error");
        exit(EXIT_FAILURE);
    }

    char msg_to[BUFF_SIZE], msg_from[BUFF_SIZE];

    msg_to[BUFF_SIZE - 1] = 0;
    msg_from[BUFF_SIZE - 1] = 0;

    sprintf(msg_to, "%d", getpid());

    if (send(sock, msg_to, BUFF_SIZE, 0) == -1)
    {
        perror("send error");
        exit(EXIT_FAILURE);
    }

    printf("client %d send: %s\n", getpid(), msg_to);

    if (read(sock, msg_from, BUFF_SIZE) == -1) 
    {
        perror("read error");
        exit(EXIT_FAILURE);
    }

    printf("client %d received: %s\n", getpid(), msg_from);

    close(sock);

    return 0;
}
