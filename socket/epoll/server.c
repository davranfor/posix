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
    char text[BUFFER_SIZE];
    size_t size, sent;
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
    msg *data;

    if ((data = calloc(1, sizeof *data)) == NULL)
    {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
    data->fd = fd;
    set_nonblocking(fd);

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

static void event_mod(int epollfd, struct epoll_event *event, unsigned events)
{
    msg *data = event->data.ptr;

    event->events = events;
    if (epoll_ctl(epollfd, EPOLL_CTL_MOD, data->fd, event) == -1)
    {
        perror("epoll_ctl");
        exit(EXIT_FAILURE);
    }
}

static void event_del(int epollfd, struct epoll_event *event)
{
    msg *data = event->data.ptr;

    event->events = 0;
    event->data.ptr = NULL;
    if (epoll_ctl(epollfd, EPOLL_CTL_DEL, data->fd, event) == -1)
    {
        perror("epoll_ctl");
        exit(EXIT_FAILURE);
    }
    close(data->fd);
    free(data);
}

static int msg_send(int epollfd, struct epoll_event *event)
{
    msg *data = event->data.ptr;

    if ((data->sent == 0) && (data->size > 0))
    {
        data->text[data->size - 1] = EOT;
    }
    while (1)
    {
        ssize_t size = send(data->fd, data->text + data->sent, data->size - data->sent, 0);

        if (size == -1)
        {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
            {
                if (!(event->events & EPOLLOUT))
                {
                    event_mod(epollfd, event, EPOLLOUT);
                }
                return 1;
            }
            perror("recv");
            return 0;
        }
        if (size == 0)
        {
            return 0;
        }
        data->sent += (size_t)size;
        if (data->sent == data->size)
        {
            if (event->events & EPOLLOUT)
            {
                event_mod(epollfd, event, EPOLLIN | EPOLLET);
            }
            break;
        }
    }
    data->size = 0;
    data->sent = 0;
    return 1;
}

static int msg_recv(int epollfd, struct epoll_event *event)
{
    msg *data = event->data.ptr;

    while (1)
    {
        ssize_t size = recv(data->fd, data->text + data->size, sizeof(data->text) - data->size, 0);

        if (size == -1)
        {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
            {
                return 1;
            }
            perror("recv");
            return 0;
        }
        if (size == 0)
        {
            return 0;
        }
        data->size += (size_t)size;
        if (data->text[data->size - 1] == EOT)
        {
            data->text[data->size - 1] = '\0';
            break;
        }
    }
    fwrite(data->text, sizeof(char), data->size, stdout);
    return msg_send(epollfd, event);
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
                (events[event].events & EPOLLHUP) ||
                (!(events[event].events & (EPOLLIN | EPOLLOUT))))
            {
                fprintf(stderr, "epoll: Bad event %u\n", events[event].events);
                exit(EXIT_FAILURE);
            }
            if (events[event].events & EPOLLIN)
            {
                msg *data = events[event].data.ptr;

                if (data->fd == serverfd)
                {
                    int clientfd;

                    if ((clientfd = accept(serverfd, NULL, NULL)) == -1)
                    {
                        perror("accept");
                        exit(EXIT_FAILURE);
                    }
                    event_add(epollfd, clientfd, EPOLLIN | EPOLLET);
                    continue;
                }
                if (!msg_recv(epollfd, &events[event]))
                {
                    event_del(epollfd, &events[event]);
                }
            }
            else if (events[event].events & EPOLLOUT)
            {
                if (!msg_send(epollfd, &events[event]))
                {
                    event_del(epollfd, &events[event]);
                }
            }
        }
    }
    // Never reached
    close(serverfd);
    free(serverev);
    puts("Server exits");
    return 0;
}

