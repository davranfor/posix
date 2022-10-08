#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
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

enum {SEND, RECV};

typedef struct
{
    char text[BUFFER_SIZE];
    size_t size, sent;
    int op, fd, count;
} msg;

static int openfds;
static int msgno;

static void set_nonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);

    if ((flags == -1) || (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1))
    {
        perror("fcntl");
        exit(EXIT_FAILURE);
    }
}

static void event_add(int epollfd, int fd, unsigned events)
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
    openfds++;
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
    openfds--;
}

static int msg_send(msg *data)
{
    if (data->op != SEND)
    {
        return 1;
    }
    if (data->count == 100)
    {
        return 0;
    }
    if ((data->sent == 0) && (data->size == 0))
    {
        snprintf(data->text, sizeof data->text, "%05d) %02d Hello from client %02d\n", msgno++, data->count++, data->fd);

        data->size = strlen(data->text) + 1;
        data->text[data->size - 1] = EOT;
    }
    while (data->sent < data->size)
    {
        ssize_t size = send(data->fd, data->text + data->sent, data->size - data->sent, 0);

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
        data->sent += (size_t)size;
    }
    data->op = RECV;
    data->sent = 0;
    data->size = 0;
    return 1;
}

static int msg_recv(msg *data)
{
    if (data->op != RECV)
    {
        return 1;
    }
    while (1)
    {
        ssize_t size = recv(data->fd, data->text + data->size, sizeof(data->text) - data->size, 0);

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
        data->size += (size_t)size;
    }
    if ((data->size > 0) && (data->text[data->size - 1] == EOT))
    {
        data->text[data->size - 1] = '\0';
        fwrite(data->text, sizeof(char), data->size, stdout);
        data->op = SEND;
        data->size = 0;
    }
    return 1;
}

int main(void)
{
    struct sockaddr_in server;

    memset(&server, 0, sizeof server);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(SERVER_ADDR);
    server.sin_port = htons(SERVER_PORT);

    enum {MAX_EVENTS = SERVER_LISTEN};
    struct epoll_event events[MAX_EVENTS] = {0};
    int epollfd;

    if ((epollfd = epoll_create1(0)) == -1)
    {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }
    for (int event = 0; event < MAX_EVENTS; event++)
    {
        int clientfd;

        if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
            perror("socket");
            exit(EXIT_FAILURE);
        }
        if (connect(clientfd, (struct sockaddr *)&server, sizeof server) == -1)
        {
            perror("connect");
            exit(EXIT_FAILURE);
        }
        event_add(epollfd, clientfd, EPOLLIN | EPOLLOUT | EPOLLET);
    }
    while (openfds > 0)
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

            msg *data = events[event].data.ptr;

            if (events[event].events & EPOLLIN)
            {
                if (!msg_recv(data))
                {
                    event_del(epollfd, &events[event]);
                }
            }
            if (events[event].events & EPOLLOUT)
            {
                if (!msg_send(data))
                {
                    event_del(epollfd, &events[event]);
                }
            }
        }
    }
    puts("Client exits");
    return 0;
}

