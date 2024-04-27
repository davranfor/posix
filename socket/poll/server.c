#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include "shared.h"

static char buffer[BUFFER_SIZE];

static void conn_close(struct pollfd *conn)
{
    close(conn->fd);
    conn->fd = 0;
    conn->events = 0;
    conn->revents = 0;
}

static void conn_handle(struct pollfd *conn, struct poolfd *pool)
{
    size_t size = 0, sent = 0;
    char *data = NULL;

    if (!(conn->revents & POLLOUT))
    {
        while (size < BUFFER_SIZE)
        {
            ssize_t bytes = recv(conn->fd, buffer + size, BUFFER_SIZE - size, 0);

            if (bytes == -1)
            {
                if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
                {
                    break;
                }
                perror("recv");
                goto stop;
            }
            if (bytes == 0)
            {
                goto stop;
            }
            size += (size_t)bytes;
        }
        if (size == 0)
        {
            return;
        }
        if ((pool->data == NULL) && (buffer[size - 1] == '\0'))
        {
            data = buffer;
        }
        else
        {
            if (!pool_add(pool, buffer, size))
            {
                perror("pool_add");
                goto stop;
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
        while (sent < size)
        {
            ssize_t bytes = send(conn->fd, data + sent, size - sent, 0);

            if (bytes == -1)
            {
                if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
                {
                    break;
                }
                perror("send");
                goto stop;
            }
            if (bytes == 0)
            {
                goto stop;
            }
            sent += (size_t)bytes;
        }
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
                    goto stop;
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
stop:
    printf("Closing %d ...\n", conn->fd);
    conn_close(conn);
    pool_reset(pool);
}

static int unblock(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);

    if (flags == -1)
    {
        return -1;
    }
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
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
    if (unblock(serverfd) == -1)
    {
        perror("unblock");
        exit(EXIT_FAILURE);
    }
    if (listen(serverfd, SERVER_LISTEN) == -1)
    //if (listen(serverfd, 5) == -1)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    enum {MAX_CLIENTS = SERVER_LISTEN + 1};
    //enum {MAX_CLIENTS = 5 + 1};
    struct poolfd pool[MAX_CLIENTS] = {0};
    struct pollfd fds[MAX_CLIENTS] = {0};

    fds[0].fd = serverfd;
    fds[0].events = POLLIN;
    while (1)
    {
        int ready = poll(fds, MAX_CLIENTS, -1);

        if (ready == -1)
        {
            perror("poll");
            exit(EXIT_FAILURE);
        }
        if (ready > 0)
        {
            if (fds[0].revents & POLLIN)
            {
                int clientfd = accept(serverfd, NULL, NULL);

                if (clientfd == -1)
                {
                    if ((errno != EAGAIN) && (errno != EWOULDBLOCK))
                    {
                        perror("accept");
                        exit(EXIT_FAILURE);
                    }
                }
                else
                {
                    if (unblock(clientfd) == -1)
                    {
                        perror("unblock");
                        exit(EXIT_FAILURE);
                    }
                    for (nfds_t client = 1; client < MAX_CLIENTS; client++)
                    {
                        if (fds[client].fd == 0)
                        {
                            fds[client].fd = clientfd;
                            fds[client].events = POLLIN;
                            break;
                        }
                    }
                }
            }
            for (nfds_t client = 1; client < MAX_CLIENTS; client++)
            {
                if (fds[client].revents & (POLLIN | POLLOUT))
                {
                    conn_handle(&fds[client], &pool[client]);
                }
            }
        }
    }
    // Never reached
    close(serverfd);
    puts("Server exits");
    return 0;
}

