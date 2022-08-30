#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "shared.h"

static int handler(int serverfd, char *str)
{
    if (sendall(serverfd, str) == -1)
    {
        perror("sendall");
        exit(EXIT_FAILURE);
    }

    ssize_t len = recvall(serverfd, str);

    if (len == 0)
    {
        return 0;
    }
    if (len == -1)
    {
        perror("recvall");
        exit(EXIT_FAILURE);
    }
    printf("Length = %05zd | Server says: %s\n", len, str);
    return 1;
}

int main(void)
{
    struct sockaddr_in server;

    memset(&server, 0, sizeof server);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(SERVER_ADDR);
    server.sin_port = htons(SERVER_PORT);

    int serverfd;

    if ((serverfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    if (connect(serverfd, (struct sockaddr *)&server, sizeof server) == -1)
    {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    struct sockbuff buff;
    char *str = sockbuff_init(&buff);

    while (fgets(str, BUFFER_SIZE, stdin) != NULL)
    {
        str[strcspn(str, "\n")] = '\0';
        if (str[0] != '\0')
        {
            if (!handler(serverfd, str))
            {
                break;
            }
        }
    }
    close(serverfd);
    puts("Client exits");
    return 0;
}

