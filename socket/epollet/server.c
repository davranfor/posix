#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include "shared.h"

typedef struct
{
    char str[BUFFER_SIZE];
    int fd;
} msg;

static void set_nonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);

    if ((flags == -1) || (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1))
    {
        perror("fcntl");
        exit(EXIT_FAILURE);
    }
}

static msg *event_add(int epollfd, int fd, unsigned events)
{
    set_nonblocking(fd);

    msg *data;

    if ((data = calloc(1, sizeof *data)) == NULL)
    {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
    data->fd = fd;

    struct epoll_event event;

    event.events = events;
    event.data.ptr = data;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event) == -1)
    {
        perror("epoll_ctl");
        exit(EXIT_FAILURE);
    }
    return data;
}

static void event_del(int epollfd, msg *data)
{
    struct epoll_event event;

    if (epoll_ctl(epollfd, EPOLL_CTL_DEL, data->fd, &event) == -1)
    {
        perror("epoll_ctl");
        exit(EXIT_FAILURE);
    }
    close(data->fd);
    free(data);
}

static ssize_t event_recv(msg *data)
{
    size_t bytes = 0;
    ssize_t size;

    while (1)
    {
        size = recv(data->fd, data->str + bytes, sizeof(data->str) - bytes, 0);
        if (size == -1)
        {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
            {
                break;
            }
            perror("recv");
            return 0;
        }
        if (size == 0)
        {
            return 0;
        }
        bytes += (size_t)size;
    }
    fwrite(data->str, sizeof(char), bytes, stdout);
    return 1;
}

static ssize_t event_send(msg *data)
{
    size_t len = strlen(data->str) + 1;
    size_t bytes = 0;
    ssize_t size;

    while (bytes != len)
    {
        size = send(data->fd, data->str + bytes, len - bytes, 0);
        if (size == -1)
        {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
            {
                break;
            }
            perror("recv");
            return 0;
        }
        if (size == 0)
        {
            return 0;
        }
        bytes += (size_t)size;
    }
    return 1;
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

    enum {MAX_EVENTS = SERVER_LISTEN};
    struct epoll_event events[MAX_EVENTS] = {0};
    int epollfd;

    if ((epollfd = epoll_create1(0)) == -1)
    {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    msg *serverev = event_add(epollfd, serverfd, EPOLLIN);

    while (1)
    {
        int nevents;

        if ((nevents = epoll_wait(epollfd, events, MAX_EVENTS, -1)) == -1)
        {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }
        for (int event = 0; event < nevents; event++)
        {
            if ((events[event].events & EPOLLERR) ||
                (events[event].events & EPOLLHUP))
            {
                fprintf(stderr, "epoll: Bad event %u\n", events[event].events);
                exit(EXIT_FAILURE);
            }

            msg *data = events[event].data.ptr;

            if (events[event].events & EPOLLIN)
            {
                int clientfd = data->fd;

                if (clientfd == serverfd)
                {
                    if ((clientfd = accept(serverfd, NULL, NULL)) == -1)
                    {
                        perror("accept");
                        exit(EXIT_FAILURE);
                    }
                    event_add(epollfd, clientfd, EPOLLIN | EPOLLET);
                }
                else
                {
                    if (event_recv(data))
                    {
                        events[event].events = EPOLLOUT;
                    }
                    else
                    {
                        event_del(epollfd, data);
                    }
                }
            }
            if (events[event].events & EPOLLOUT)
            {
                if (event_send(data))
                {
                    events[event].events = EPOLLIN | EPOLLET;
                }
                else
                {
                    event_del(epollfd, data);
                }
            }
        }
    }
    // Never reached
    event_del(epollfd, serverev);
    puts("Server exits");
    return 0;
}

