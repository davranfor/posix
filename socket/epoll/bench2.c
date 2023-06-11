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

typedef struct msg
{
    int (*send)(struct msg *, int);
    int (*recv)(struct msg *, int);
    size_t iter, sent, size;
    char *text;
} msg;

static int openfds;
static int msgno;

static int msg_recv(msg *data, int);

static int msg_skip(msg *data, int fd)
{
    (void)data;
    (void)fd;
    return 1;
}

static int msg_send(msg *data, int fd)
{
    if (data->iter == 100)
    {
        return 0;
    }
    if ((data->sent == 0) && (data->size == 0))
    {
        snprintf(data->text, BUFFER_SIZE, "%05d) %02zu Hello from client %02d\n", msgno++, data->iter++, fd);
        data->size = strlen(data->text) + 1;
    }
    while (data->sent < data->size)
    {
        ssize_t size = send(fd, data->text + data->sent, data->size - data->sent, 0);

        if (size == -1)
        {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
            {
                return 1;
            }
            perror("send");
            return 0;
        }
        if (size == 0)
        {
            return 0;
        }
        data->sent += (size_t)size;
    }
    data->send = msg_skip;
    data->recv = msg_recv;
    data->sent = 0;
    data->size = 0;
    return 1;
}

static int msg_recv(msg *data, int fd)
{
    while (1)
    {
        ssize_t size = recv(fd, data->text + data->size, BUFFER_SIZE - data->size, 0);

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
    if ((data->size > 0) && (data->text[data->size - 1] == '\0'))
    {
        printf("%s", data->text);
        data->recv = msg_skip;
        data->send = msg_send;
        data->size = 0;
    }
    return 1;
}

static void msg_clear(msg *data)
{
    data->send = msg_send;
    data->recv = msg_skip;
    data->iter = 0;
    data->sent = 0;
    data->size = 0;
}

static uint32_t map_set(uint8_t *map, uint32_t max)
{
    for (uint32_t i = 0; i < max; i++)
    {
        if (map[i] == 0)
        {
            map[i] = 1;
            return i;
        }
    }
    return 0;
}

static void map_unset(uint8_t *map, msg *data, uint32_t id)
{
    msg_clear(data);
    map[id] = 0;
}

static void event_add(int epollfd, int fd, unsigned events, uint32_t data)
{
    int flags = fcntl(fd, F_GETFL, 0);

    if ((flags == -1) || (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1))
    {
        perror("fcntl");
        exit(EXIT_FAILURE);
    }

    struct epoll_event event;

    event.events = events;
    event.data.u32 = data;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event) == -1)
    {
        perror("epoll_ctl");
        exit(EXIT_FAILURE);
    }
    openfds++;
}

static void event_del(int epollfd, int fd, struct epoll_event *event)
{
    event->events = 0x0;
    event->data.u32 = 0;
    if (epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, event) == -1)
    {
        perror("epoll_ctl");
        exit(EXIT_FAILURE);
    }
    close(fd);
    openfds--;
}

int main(void)
{
    struct sockaddr_in server;

    memset(&server, 0, sizeof server);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(SERVER_ADDR);
    server.sin_port = htons(SERVER_PORT);

    enum {MAX_EVENTS = SERVER_LISTEN};

    struct epoll_event *events = calloc(MAX_EVENTS, sizeof *events);

    if (events == NULL)
    {
        perror("calloc");
        exit(EXIT_FAILURE);
    }

    msg *msgs = malloc(MAX_EVENTS * sizeof *msgs);

    if (msgs == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < MAX_EVENTS; i++)
    {
        msg_clear(&msgs[i]);
        msgs[i].text = malloc(BUFFER_SIZE);
        if (msgs[i].text == NULL)
        {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
    }

    uint8_t *map = calloc(MAX_EVENTS, sizeof *map);

    if (map == NULL)
    {
        perror("calloc");
        exit(EXIT_FAILURE);
    }

    int epollfd;

    if ((epollfd = epoll_create1(0)) == -1)
    {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < MAX_EVENTS; i++)
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

        uint32_t u32 = (map_set(map, MAX_EVENTS) << 16) | (uint32_t)clientfd;

        event_add(epollfd, clientfd, EPOLLIN | EPOLLOUT | EPOLLET, u32);
    }
    while (openfds > 0)
    {
        int nevents = epoll_wait(epollfd, events, MAX_EVENTS, -1);

        if (nevents == -1)
        {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }
        for (int i = 0; i < nevents; i++)
        {
            struct epoll_event *event = &events[i];
            uint32_t u32 = event->data.u32;
            uint32_t id = u32 >> 16;
            int fd = u32 & 0xffff;
            msg *data = &msgs[id];

            if ((event->events & EPOLLERR) ||
                (event->events & EPOLLHUP) ||
                (!(event->events & (EPOLLIN | EPOLLOUT))))
            {
                fprintf(stderr, "epoll: Bad event %u\n", event->events);
                event_del(epollfd, fd, event);
                map_unset(map, data, id);
                continue;
            }
            if (event->events & EPOLLIN)
            {
                if (!data->recv(data, fd))
                {
                    event_del(epollfd, fd, event);
                    map_unset(map, data, id);
                }
            }
            if (event->events & EPOLLOUT)
            {
                if (!data->send(data, fd))
                {
                    event_del(epollfd, fd, event);
                    map_unset(map, data, id);
                }
            }
        }
    }
    puts("Client exits");
    free(events);
    free(msgs);
    free(map);
    return 0;
}

