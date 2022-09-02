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
    char str[BUFFER_SIZE];

    while (1)
    {
        ssize_t size;

        if ((size = recvstr(clientfd, str)) <= 0)
        {
            if (size == -1)
            {
                perror("recvstr");
            }
            break;
        }
        printf("Client: %d | Size: %05zd | Client says: %s\n", clientfd, size, str);
        if ((size = sendstr(clientfd, str)) <= 0)
        {
            if (size == -1)
            {
                perror("sendstr");
            }
            break;
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

    int serverfd, yes = 1;

    if ((serverfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1)
    {
        perror("setsockopt");
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
    while (1)
    {
        intptr_t clientfd = accept(serverfd, NULL, NULL);

        if (clientfd == -1)
        {
            perror("accept");
            break;
        }

        pthread_t thread;

        if (pthread_create(&thread, &attr, handler, (void *)clientfd) != 0)
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
    // Never reached
    close(serverfd);
    puts("Server exits");
    return 0;
}

