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

static ssize_t handler(int clientfd)
{
    char str[BUFFER_SIZE];
    ssize_t size;

    while (1)
    {
        size = recv(clientfd, str, sizeof str, 0);
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
        fwrite(str, sizeof(char), (size_t)size, stdout);
    }
    if ((size = sendstr(clientfd, str)) <= 0)
    {
        if (size == -1)
        {
            perror("sendstr");
        }
        return 0;
    }
    return 1;
}

static void set_nonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);

    if ((flags == -1) || (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1))
    {
        perror("fcntl");
        exit(EXIT_FAILURE);
    }
}

static void event_add(int epollfd, int fd, unsigned flags)
{
    set_nonblocking(fd);

    struct epoll_event event;

    event.events = flags;
    event.data.fd = fd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event) == -1)
    {
        perror("epoll_ctl");
        exit(EXIT_FAILURE);
    }
}

static void event_del(int epollfd, int fd)
{
    struct epoll_event event;

    if (epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &event) == -1)
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
    struct epoll_event events[MAX_EVENTS] = {0};
    int epollfd;

    if ((epollfd = epoll_create1(0)) == -1)
    {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }
    event_add(epollfd, serverfd, EPOLLIN);
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
                (!(events[event].events & EPOLLIN)))
            {
                perror("epoll");
                exit(EXIT_FAILURE);
            }

            int clientfd = events[event].data.fd;

            if (clientfd == serverfd)
            {
                if ((clientfd = accept(serverfd, NULL, NULL)) == -1)
                {
                    perror("accept");
                    exit(EXIT_FAILURE);
                }
                event_add(epollfd, clientfd, EPOLLIN | EPOLLET);
            }
            else if (!handler(clientfd))
            {
                event_del(epollfd, clientfd);
            }
        }
    }
    // Never reached
    event_del(epollfd, serverfd);
    puts("Server exits");
    return 0;
}

