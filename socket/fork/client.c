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
    ssize_t size;

    if ((size = sendstr(serverfd, str)) <= 0)
    {
        if (size == -1)
        {
            perror("sendstr");
        }
        return 0;
    }
    if ((size = recvstr(serverfd, str)) <= 0)
    {
        if (size == -1)
        {
            perror("recvstr");
        }
        return 0;
    }
    printf("Size: %05zd | Server says: %s\n", size, str);
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
        str[strcspn(str, "\n")] = '\0';
        if (!handler(serverfd, str))
        {
            break;
        }
    }
    close(serverfd);
    puts("Client exits");
    return 0;
}

