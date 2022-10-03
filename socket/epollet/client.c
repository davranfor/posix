#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include "shared.h"

static atomic_int counter;

static void *handler(void *arg)
{
    struct sockaddr_in *server = arg;
    int serverfd;

    if ((serverfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    if (connect(serverfd, (struct sockaddr *)server, sizeof *server) == -1)
    {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    char str[BUFFER_SIZE];

    for (int i = 0; i < 100; i++)
    {
        snprintf(str, sizeof str, "%05d) %02d Hello from client %02d\n", counter++, i, serverfd);

        size_t len = strlen(str) + 1, sent = 0;
        ssize_t size = 0;

        while (sent < len)
        {
            size = send(serverfd, str + sent, len - sent, 0);
            if (size < 0)
            {
                if (size == -1)
                {
                    perror("send");
                }
                return NULL;
            }
            sent += (size_t)size;
        }
        size = recv(serverfd, str, sizeof(str), 0);
        if (size < 0)
        {
            if (size == -1)
            {
                perror("recv");
            }
            return NULL;
        }
        //fwrite(str, sizeof(char), (size_t)size, stdout);
printf("%s", str);
    }
    close(serverfd);
    return NULL;
}

int main(void)
{
    struct sockaddr_in server;

    memset(&server, 0, sizeof server);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(SERVER_ADDR);
    server.sin_port = htons(SERVER_PORT);

    enum {NTHREADS = SERVER_LISTEN};
    pthread_t thread[NTHREADS];

    for (int i = 0; i < NTHREADS; i++)
    {
        if (pthread_create(&thread[i], NULL, handler, &server) != 0)
        {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }
    for (int i = 0; i < NTHREADS; i++)
    {
        if (pthread_join(thread[i], NULL) != 0)
        {
            perror("pthread_join");
            exit(EXIT_FAILURE);
        }
    }
    puts("Client exits");
    return 0;
}

