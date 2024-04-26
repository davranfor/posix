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
#include "shared.h"

static atomic_int msgno;

static void *handler(void *arg)
{
    struct sockaddr_in *server = arg;
    int fd;

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    if (connect(fd, (struct sockaddr *)server, sizeof *server) == -1)
    {
        perror("connect");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < 100; i++)
    {
        char str[128];

        snprintf(str, sizeof str, "%05d) %02d Hello from client %02d\n", msgno++, i, fd);

        size_t size = strlen(str) + 1;
        size_t sent = 0;

        while (sent < size)
        {
            ssize_t bytes = send(fd, str + sent, size - sent, 0);

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

        char buffer[BUFFER_SIZE];
        struct poolfd pool = {0};
        char *data = NULL;
        size_t rcvd = 0;
        int done = 0;

        while (!done)
        {
            while (rcvd < BUFFER_SIZE)
            {
                ssize_t bytes = recv(fd, buffer + rcvd, BUFFER_SIZE - rcvd, 0);

                if (bytes == -1)
                {
                    perror("recv");
                    return 0;
                }
                if (bytes == 0)
                {
                    return 0;
                }
                rcvd += (size_t)bytes;
                if (buffer[rcvd - 1] == '\0')
                {
                    done = 1;
                    break;
                }
            }
            if ((done == 1) && (pool.data == NULL))
            {
                data = buffer;
                size = rcvd;
                break;
            }
            if (!pool_add(&pool, buffer, rcvd))
            {
                perror("pool_add");
                return 0;
            }
            data = pool.data;
            size = pool.size;
            rcvd = 0;
        }
        fwrite(data, sizeof(char), size, stdout);
        pool_reset(&pool);
    }
    close(fd);
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

