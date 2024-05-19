#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include "shared.h"

#define BACKLOG 64

static char buffer[BUFFER_SIZE];

static int conn_socket(uint16_t port)
{
    struct sockaddr_in server;

    memset(&server, 0, sizeof server);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);

    int fd, opt = 1;

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt) == -1)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    if (unblock(fd) == -1)
    {
        perror("unblock");
        exit(EXIT_FAILURE);
    }
    if (bind(fd, (struct sockaddr *)&server, sizeof server) == -1)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    if (listen(fd, BACKLOG) == -1)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    return fd;
}

static void conn_signal(int signum)
{
    printf("\nCaught signal %d (SIGINT)\n", signum);
}

static void conn_attach(struct pollfd *conn, int fd)
{
    conn->fd = fd;
    conn->events = POLLIN;
}

static void conn_reset(struct pollfd *conn)
{
    close(conn->fd);
    conn->fd = -1;
    conn->events = 0;
}

static void conn_handle(struct pollfd *conn, struct poolfd *pool)
{
    char *data = NULL;
    size_t size = 0;

    if (conn->revents & ~(POLLIN | POLLOUT))
    {
        goto reset;
    }
    if (conn->revents == POLLIN)
    {
        ssize_t bytes = recv(conn->fd, buffer, BUFFER_SIZE, 0);

        if (bytes == -1)
        {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
            {
                return;
            }
            perror("recv");
            goto reset;
        }
        if (bytes == 0)
        {
            goto reset;
        }
        size = (size_t)bytes;
        if ((pool->data == NULL) && (buffer[size - 1] == '\0'))
        {
            data = buffer;
        }
        else
        {
            if (!pool_add(pool, buffer, size))
            {
                perror("pool_add");
                goto reset;
            }
            if (buffer[size - 1] == '\0')
            {
                data = pool->data;
                size = pool->size;
            }
            else
            {
                return;
            }
        }
        fwrite(data, sizeof(char), size, stdout);
    }
    else if (pool->data != NULL)
    {
        data = pool->data + pool->sent;
        size = pool->size - pool->sent;
    }
    if (data != NULL)
    {
        ssize_t bytes = send(conn->fd, data, size, 0);

        if (bytes == 0)
        {
            goto reset;
        }
        if (bytes == -1)
        {
            if ((errno != EAGAIN) && (errno != EWOULDBLOCK))
            {
                perror("send");
                goto reset;
            }
            bytes = 0;
        }

        size_t sent = (size_t)bytes;

        if (sent == size)
        {
            pool_reset(pool);
            conn->events &= ~POLLOUT;
        }
        else
        {
            if (pool->data == NULL)
            {
                if (!pool_add(pool, data + sent, size - sent))
                {
                    perror("pool_add");
                    goto reset;
                }
            }
            else
            {
                pool_sync(pool, sent);
            }
            conn->events |= POLLOUT;
        }
        return;
    }
reset:
    conn_reset(conn);
    pool_reset(pool);
}

static void conn_close(struct pollfd *conn, struct poolfd *pool)
{
    conn_reset(conn);
    pool_reset(pool);
}

static void conn_loop(uint16_t port)
{
    enum {server = 0, maxfds = MAX_CLIENTS + 1};
    struct poolfd pool[maxfds] = {0};
    struct pollfd conn[maxfds] = {0};

    conn_attach(&conn[server], conn_socket(port));
    for (nfds_t client = 1; client < maxfds; client++)
    {
        conn[client].fd = -1;
    }
    if (signal(SIGINT, conn_signal) == SIG_ERR)
    {
        perror("signal");
        exit(EXIT_FAILURE);
    }
    while (1)
    {
        if (poll(conn, maxfds, -1) == -1)
        {
            perror("poll");
            break;
        }
        if (conn[server].revents & POLLIN)
        {
            int fd = accept(conn[server].fd, NULL, NULL);

            if (fd == -1)
            {
                if ((errno != EAGAIN) && (errno != EWOULDBLOCK))
                {
                    perror("accept");
                    break;
                }
            }
            else
            {
                int done = 0;

                for (nfds_t client = 1; client < maxfds; client++)
                {
                    if (conn[client].fd == -1)
                    {
                        if (unblock(fd) == -1)
                        {
                            perror("unblock");
                            break;
                        }
                        conn_attach(&conn[client], fd);
                        done = 1;
                        break;
                    }
                }
                if (!done)
                {
                    close(fd);
                }
            }
        }
        for (nfds_t client = 1; client < maxfds; client++)
        {
            if (conn[client].revents)
            {
                conn_handle(&conn[client], &pool[client]);
            }
        }
    }
    for (nfds_t fd = 0; fd < maxfds; fd++)
    {
        if (conn[fd].fd != -1)
        {
            conn_close(&conn[fd], &pool[fd]);
        }
    }
}

int main(int argc, char *argv[])
{
    uint16_t port = SERVER_PORT;

    if (argc > 1)
    {
        char *end;
        unsigned long num = strtoul(argv[1], &end, 10);

        if ((*end != '\0') || (num < 1) || (num > 65535))
        {
            fprintf(stderr, "Usage %s <port>\n", argv[0]);
            exit(EXIT_FAILURE);
        }
        port = (uint16_t)num;
    }
    conn_loop(port);
    return 0;
}

