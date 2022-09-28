#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "shared.h"

static ssize_t handler(int clientfd)
{
    char str[BUFFER_SIZE];
    ssize_t size;

    if ((size = recvstr(clientfd, str)) <= 0)
    {
        if (size == -1)
        {
            perror("recvstr");
        }
        return 0;
    }
    printf("Client: %d | Size: %05zd | Client says: %s\n", clientfd, size, str);
    if ((size = sendstr(clientfd, str)) <= 0)
    {
        if (size == -1)
        {
            perror("sendstr");
        }
        return 0;
    }
    return size;
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
    struct epoll_event event, events[MAX_EVENTS];
    int epollfd;

    if ((epollfd = epoll_create1(0)) == -1)
    {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }
    event.events = EPOLLIN;
    event.data.fd = serverfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, serverfd, &event) == -1)
    {
        perror("epoll_ctl");
        exit(EXIT_FAILURE);
    }
    while (1)
    {
        int nevents;

        if ((nevents = epoll_wait(epollfd, events, MAX_EVENTS, -1)) == -1)
        {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }
        for (int iter = 0; iter < nevents; iter++)
        {
            if (events[iter].data.fd == serverfd)
            {
                int clientfd;

                if ((clientfd = accept(serverfd, NULL, NULL)) == -1)
                {
                    perror("accept");
                    exit(EXIT_FAILURE);
                }
                event.events = EPOLLIN | EPOLLET;
                event.data.fd = clientfd;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, clientfd, &event) == -1)
                {
                    perror("epoll_ctl");
                    exit(EXIT_FAILURE);
                }
            }
            else if (!handler(events[iter].data.fd))
            {
                close(events[iter].data.fd);
            }
        }
    }
    // Never reached
    close(serverfd);
    puts("Server exits");
    return 0;
}

