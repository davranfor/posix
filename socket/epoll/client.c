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
    size_t size = strlen(str) + 1;
    size_t sent = 0;

    while (sent < size)
    {
        ssize_t bytes = send(serverfd, str + sent, size - sent, 0);

        if (bytes == -1)
        {
            perror("send");
            return 0;
        }
        if (bytes == 0)
        {
            return 0;
        }
        sent += (size_t)bytes;
    }
    size = 0;
    while (1)
    {
        ssize_t bytes = recv(serverfd, str + size, BUFFER_SIZE - size, 0);

        if (bytes == -1)
        {
            perror("recv");
            return 0;
        }
        if (bytes == 0)
        {
            return 0;
        }
        size += (size_t)bytes;
        if (str[size - 1] == '\0')
        {
            break;
        }
    }
    printf("Size: %05zu | Server says: %s", size, str);
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

