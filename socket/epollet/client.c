#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "shared.h"

static int handler(int serverfd, char *str)
{
    size_t bytes = strlen(str) + 1, sent = 0;
    ssize_t size = 0;

    str[bytes - 1] = EOT;
    while (sent < bytes)
    {
        size = send(serverfd, str + sent, bytes - sent, 0);
        if (size == -1)
        {
            perror("send");
            return 0;
        }
        if (size == 0)
        {
            return 0;
        }
        sent += (size_t)size;
    }
    bytes = 0;
    while (1)
    {
        size = recv(serverfd, str + bytes, BUFFER_SIZE - bytes, 0);
        if (size == -1)
        {
            perror("recv");
            return 0;
        }
        if (size == 0)
        {
            return 0;
        }
        bytes += (size_t)size;
        if (str[bytes - 1] == EOT)
        {
            str[bytes - 1] = '\0';
            break;
        }
    }
    printf("Size: %05zu | Server says: %s", bytes, str);
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

    char str[BUFFER_SIZE];

    while (fgets(str, sizeof str, stdin) != NULL)
    {
        if (!handler(serverfd, str))
        {
            break;
        }
    }
    close(serverfd);
    puts("Client exits");
    return 0;
}

