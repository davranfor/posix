#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
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
    int (*recv)(struct msg *, int);
    int (*send)(struct msg *, int);
    size_t sent, size, room;
    char *text;
} msg;

static int msg_send(msg *data, int);

static int msg_skip(msg *data, int fd)
{
    (void)data;
    (void)fd;
    return 1;
}

static int msg_recv(msg *data, int fd)
{
    while (1)
    {
        if (data->size == data->room)
        {
            char *text = realloc(data->text, data->room + BUFFER_SIZE);

            if (text == NULL)
            {
                perror("malloc");
                return 0;
            }
            data->text = text;
            data->room += BUFFER_SIZE;
        }

        ssize_t size = recv(fd, data->text + data->size, data->room - data->size, 0);

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
        fwrite(data->text, sizeof(char), data->size, stdout);
        data->recv = msg_skip;
        data->send = msg_send;
    }
    return 1;
}

static int msg_send(msg *data, int fd)
{
    while (data->sent < data->size)
    {
        ssize_t sent = send(fd, data->text + data->sent, data->size - data->sent, 0);

        if (sent == -1)
        {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
            {
                return 1;
            }
            perror("send");
            return 0;
        }
        if (sent == 0)
        {
            return 0;
        }
        data->sent += (size_t)sent;
    }
    data->send = msg_skip;
    data->recv = msg_recv;
    data->sent = 0;
    data->size = 0;
    return 1;
}

static void msg_clear(msg *data)
{
    data->recv = msg_recv;
    data->send = msg_skip;
    data->sent = 0;
    data->size = 0;
}

static msg *pool_create(size_t events)
{
    msg *pool = malloc(events * sizeof *pool);

    if (pool == NULL)
    {
        return NULL;
    }
    for (size_t id = 0; id < events; id++)
    {
        msg_clear(&pool[id]);
        pool[id].room = BUFFER_SIZE;
        pool[id].text = malloc(BUFFER_SIZE);
        if (pool[id].text == NULL)
        {
            return NULL;
        }
    }
    return pool;
}

static void pool_free(msg *pool, size_t events)
{
    for (size_t id = 0; id < events; id++)
    {
        free(pool[id].text);
    }
    free(pool);
}

static uint32_t map_set(uint8_t *map, uint32_t max)
{
    for (uint32_t id = 0; id < max; id++)
    {
        if (map[id] == 0)
        {
            map[id] = 1;
            return id;
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

    struct epoll_event *events = calloc(MAX_EVENTS, sizeof *events);

    if (events == NULL)
    {
        perror("calloc");
        exit(EXIT_FAILURE);
    }

    uint8_t *map = calloc(MAX_EVENTS, sizeof *map);

    if (map == NULL)
    {
        perror("calloc");
        exit(EXIT_FAILURE);
    }

    msg *pool = pool_create(MAX_EVENTS);

    if (pool == NULL)
    {
        perror("pool_create");
        exit(EXIT_FAILURE);
    }

    int epollfd;

    if ((epollfd = epoll_create1(0)) == -1)
    {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }
    event_add(epollfd, serverfd, EPOLLIN, (uint32_t)serverfd);

    while (1)
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
            msg *data = &pool[id];

            if ((event->events & EPOLLERR) ||
                (event->events & EPOLLHUP) ||
                (!(event->events & (EPOLLIN | EPOLLOUT))))
            {
                fprintf(stderr, "epoll: Bad event %u\n", event->events);
                event_del(epollfd, fd, event);
                if (fd != serverfd)
                {
                    map_unset(map, data, id);
                }
                continue;
            }
            if (event->events & EPOLLIN)
            {
                if (fd == serverfd)
                {
                    int clientfd = accept(serverfd, NULL, NULL);

                    if (clientfd == -1)
                    {
                        perror("accept");
                        exit(EXIT_FAILURE);
                    }
                    u32 = (map_set(map, MAX_EVENTS) << 16) | (uint32_t)clientfd;
                    event_add(epollfd, clientfd, EPOLLIN | EPOLLOUT | EPOLLET, u32);
                    continue;
                }
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
    // Never reached
    puts("Server exits");
    close(serverfd);
    pool_free(pool, MAX_EVENTS);
    free(events);
    free(map);
    return 0;
}

