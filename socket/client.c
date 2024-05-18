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

    char buffer[BUFFER_SIZE];
    struct poolfd pool = {0};

    for (int i = 0; i < 50; i++)
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
                goto stop;
            }
            if (bytes == 0)
            {
                goto stop;
            }
            sent += (size_t)bytes;
        }

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
                    goto stop;
                }
                if (bytes == 0)
                {
                    goto stop;
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
            }
            else
            {
                if (!pool_add(&pool, buffer, rcvd))
                {
                    perror("pool_add");
                    goto stop;
                }
                data = pool.data;
                size = pool.size;
                rcvd = 0;
            }
        }
        fwrite(data, sizeof(char), size, stdout);
        pool_reset(&pool);
    }
stop:
    close(fd);
    pool_reset(&pool);
    return NULL;
}

int main(int argc, char *argv[])
{
    struct sockaddr_in server;

    memset(&server, 0, sizeof server);
    server.sin_family = AF_INET;

    const char *addr = argc > 1 ? argv[1] : SERVER_ADDR;
 
    if (inet_pton(AF_INET, addr, &server.sin_addr) <= 0)
    {
        fprintf(stderr, "Invalid address '%s'\n", addr);
        exit(EXIT_FAILURE);
    }
    if (argc > 2)
    {
        char *end;
        unsigned long port = strtoul(argv[2], &end, 10);

        if ((*end != '\0') || (port < 1) || (port > 65535))
        {
            fprintf(stderr, "Invalid port '%s'\n", argv[2]);
            exit(EXIT_FAILURE);
        }
        server.sin_port = htons((uint16_t)port);
    }
    else
    {
        server.sin_port = htons(SERVER_PORT);
    }

    pthread_t thread[MAX_CLIENTS];

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (pthread_create(&thread[i], NULL, handler, &server) != 0)
        {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (pthread_join(thread[i], NULL) != 0)
        {
            perror("pthread_join");
            exit(EXIT_FAILURE);
        }
    }
    return 0;
}

