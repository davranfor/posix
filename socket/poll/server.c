#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
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

    enum {MAX_CLIENTS = SERVER_LISTEN + 1};
    struct pollfd fds[MAX_CLIENTS] = {0};
    nfds_t clients = 0;

    fds[0].fd = serverfd;
    fds[0].events = POLLIN;
    while (1)
    {
        int ready = poll(fds, clients + 1, -1);

        if (ready == -1)
        {
            perror("poll");
            exit(EXIT_FAILURE);
        }
        if (ready > 0)
        {
            if (fds[0].revents & POLLIN)
            {
                int clientfd;

                if ((clientfd = accept(serverfd, NULL, NULL)) == -1)
                {
                    perror("accept");
                    exit(EXIT_FAILURE);
                }
                for (nfds_t client = 1; client < MAX_CLIENTS; client++)
                {
                    if (fds[client].fd == 0)
                    {
                        fds[client].fd = clientfd;
                        fds[client].events = POLLIN;
                        clients++;
                        break;
                    }
                }
            }
            for (nfds_t client = 1; client < MAX_CLIENTS; client++)
            {
                if ((fds[client].fd > 0) && (fds[client].revents & POLLIN))
                {
                    if (!handler(fds[client].fd))
                    {
                        fds[client].fd = 0;
                        fds[client].events = 0;
                        fds[client].revents = 0;
                        clients--;
                    }
                }
            }
        }
    }
    // Never reached
    close(serverfd);
    puts("Server exits");
    return 0;
}

