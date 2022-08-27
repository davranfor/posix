#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>

#define SERVER_PORT 8888
#define SERVER_LISTEN 10
#define CLIENT_BUFFER 1024

static void *handler(void *arg)
{
    int clientfd = *(int *)arg;
    char str[CLIENT_BUFFER];

    while (1)
    {
        ssize_t len = recv(clientfd, str, sizeof(str) - 1, 0);

        if (len == 0)
        {
            break;
        }
        if (len == -1)
        {
            perror("recv");
            exit(EXIT_FAILURE);
        }
        str[len] = '\0';
        if (send(clientfd, str, strlen(str), 0) == -1)
        {
            perror("send");
            exit(EXIT_FAILURE);
        }
        printf("Length = %05zd | Client says: %s\n", len, str);
    }
    pthread_exit(NULL);
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

        if (pthread_create(&thread, NULL, handler, &clientfd) != 0)
        {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }
    close(serverfd);
    puts("Server exits");
    return 0;
}

