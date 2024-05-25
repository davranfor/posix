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
#include <errno.h>
#include "shared.h"

static atomic_int msgno;

static void *handler(void *server)
{
    int fd;

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct timeval tv = {CLIENT_TIMEOUT, 0};

    if ((setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof tv) == -1) ||
        (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv) == -1))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    if (connect(fd, (struct sockaddr *)server, sizeof(struct sockaddr_in)) == -1)
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
    pool_reset(&pool);
    close(fd);
    return NULL;
}

int main(int argc, char *argv[])
{
    if ((argc == 2) && (strcmp(argv[1], "-h") == 0))
    {
        printf("Usage: %s [address] [port]\n", argv[0]);
        return 0;
    }

    struct sockaddr_in server;

    memset(&server, 0, sizeof server);
    server.sin_family = AF_INET;

    const char *addr = argc > 1 ? argv[1] : SERVER_ADDR;
 
    if (inet_pton(AF_INET, addr, &server.sin_addr) <= 0)
    {
        fprintf(stderr, "Invalid address\n");
        exit(EXIT_FAILURE);
    }

    uint16_t port = argc > 2 ? string_to_uint16(argv[2]) : SERVER_PORT;

    if (port == 0)
    {
        fprintf(stderr, "Invalid port\n");
        exit(EXIT_FAILURE);
    }
    server.sin_port = htons(port);

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

