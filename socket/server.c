#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>
#include "shared.h"

static void *handler(void *arg)
{
    int clientfd = (int)(intptr_t)arg;
    struct sockbuff buff;
    char *str = sockbuff_init(&buff);

    while (1)
    {
        ssize_t len = recvall(clientfd, str);

        if (len == 0)
        {
            break;
        }
        if (len == -1)
        {
            perror("recvall");
            exit(EXIT_FAILURE);
        }
        printf("Client: %d | Length = %05zd | Client says: %s\n", clientfd, len, str);
        if (sendall(clientfd, str) == -1)
        {
            perror("sendall");
            exit(EXIT_FAILURE);
        }
    }
    close(clientfd);
    return NULL;
}

int main(void)
{
    struct sockaddr_in server;

    memset(&server, 0, sizeof server);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(SERVER_PORT);

    int serverfd;

    if ((serverfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    if (bind(serverfd, (struct sockaddr *)&server, sizeof server) == -1)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    if (listen(serverfd, SERVER_LISTEN) == -1)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    pthread_attr_t attr;

    if (pthread_attr_init(&attr) != 0)
    {
        perror("pthread_attr_init");
        exit(EXIT_FAILURE);
    }
    if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0)
    {
        perror("pthread_attr_setdetachstate");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in client;
    socklen_t socklen = sizeof client;

    while (1)
    {
        int clientfd = accept(serverfd, (struct sockaddr *)&client, &socklen);

        if (clientfd == -1)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        pthread_t thread;

        if (pthread_create(&thread, &attr, handler, (void *)(intptr_t)clientfd) != 0)
        {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }
    if (pthread_attr_destroy(&attr) != 0)
    {
        perror("pthread_attr_destroy");
        exit(EXIT_FAILURE);
    }
    close(serverfd);
    puts("Server exits");
    return 0;
}

