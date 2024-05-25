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

static char buffer[BUFFER_SIZE];
volatile sig_atomic_t stop;

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
    if (listen(fd, MAX_CLIENTS) == -1)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    return fd;
}

static void conn_signal(int signum)
{
    stop = 1;
    fprintf(stderr, "\nCaught signal %d (SIGINT)\n", signum);
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

static ssize_t conn_recv(struct pollfd *conn, struct poolfd *pool)
{
    ssize_t bytes = recv(conn->fd, buffer, BUFFER_SIZE, 0);

    if (bytes == 0)
    {
        return 0;
    }
    if (bytes == -1)
    {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
        {
            return -1;
        }
        perror("recv");
        return 0;
    }

    size_t rcvd = (size_t)bytes;

    if ((pool->data == NULL) && (buffer[rcvd - 1] == '\0'))
    {
        pool_set(pool, buffer, rcvd);
    }
    else
    {
        if (!pool_add(pool, buffer, rcvd))
        {
            perror("pool_add");
            return 0;
        }
        if (buffer[rcvd - 1] != '\0')
        {
            return -1;
        }
    }
    return bytes;
}

static ssize_t conn_send(struct pollfd *conn, struct poolfd *pool)
{
    char *data = pool->data + pool->sent;
    size_t size = pool->size - pool->sent;
    ssize_t bytes = send(conn->fd, data, size, 0);

    if (bytes == 0)
    {
        return 0;
    }
    if (bytes == -1)
    {
        if ((errno != EAGAIN) && (errno != EWOULDBLOCK))
        {
            perror("send");
            return 0;
        }
        bytes = 0;
    }

    size_t sent = (size_t)bytes;

    if (sent == size)
    {
        return bytes;
    }
    if (pool->type == POOL_BUFFERED)
    {
        if (!pool_add(pool, data + sent, size - sent))
        {
            perror("pool_add");
            return 0;
        }
    }
    else
    {
        pool_sync(pool, sent);
    }
    return -1;
}

static void conn_handle(struct pollfd *conn, struct poolfd *pool)
{
    if (!(conn->revents & ~(POLLIN | POLLOUT)))
    {
        if (conn->revents == POLLIN)
        {
            switch (conn_recv(conn, pool))
            {
                case 0:
                    goto reset;
                case -1:
                    return;
                default:
                    fwrite(pool->data, sizeof(char), pool->size, stdout);
                    break;
            }
        }
        if (pool->data != NULL)
        {
            switch (conn_send(conn, pool))
            {
                case 0:
                    goto reset;
                case -1:
                    conn->events |= POLLOUT;
                    return;
                default:
                    conn->events &= ~POLLOUT;
                    pool_reset(pool);
                    return;
            }
        }
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

static void conn_loop(int sockfd)
{
    enum {server = 0, maxfds = MAX_CLIENTS + 1};
    struct poolfd pool[maxfds] = {0};
    struct pollfd conn[maxfds] = {0};

    conn_attach(&conn[server], sockfd);
    for (nfds_t client = 1; client < maxfds; client++)
    {
        conn[client].fd = -1;
    }
    while (!stop)
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
    if ((argc == 2) && (strcmp(argv[1], "-h") == 0))
    {
        printf("Usage: %s [port]\n", argv[0]);
        return 0;
    }

    uint16_t port = argc > 1 ? string_to_uint16(argv[1]) : SERVER_PORT;

    if (port == 0)  
    {
        fprintf(stderr, "Invalid port\n");
        exit(EXIT_FAILURE);
    }

    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = conn_signal;
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    conn_loop(conn_socket(port));
    return 0;
}

