#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
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

    enum {MAX_CLIENTS = 50};
    int clients[MAX_CLIENTS] = {0};
    fd_set set;

    while (1)
    {
        // clear the socket set
        FD_ZERO(&set);
        // Add server socket to set
        FD_SET(serverfd, &set);

        int maxfd = serverfd;
        int clientfd;

        // Add clients sockets to set
        for (int client = 0; client < MAX_CLIENTS; client++)
        {
            // Socket descriptor
            clientfd = clients[client];
            // If valid socket descriptor then add to set
            if (clientfd > 0)
            {
                FD_SET(clientfd, &set);
            }
            // Get the highest file descriptor number
            if (clientfd > maxfd)
            {
                maxfd = clientfd;
            }
        }
        // Wait for activity
        if (select(maxfd + 1, &set, NULL, NULL, NULL) == -1)
        {
            perror("select");
            exit(EXIT_FAILURE);
        }
        // If incoming connection
        if (FD_ISSET(serverfd, &set))
        {
            if ((clientfd = accept(serverfd, NULL, NULL)) == -1)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            // Add new client to array of sockets
            for (int client = 0; client < MAX_CLIENTS; client++)
            {
                // If position is empty
                if (clients[client] == 0)
                {
                    clients[client] = clientfd;
                    break;
                }
            }
        }
        for (int client = 0; client < MAX_CLIENTS; client++)
        {
            clientfd = clients[client];
            // If there is an IO pending
            // Handle connection and check for disconnection
            if (FD_ISSET(clientfd, &set) && !handler(clientfd))
            {
                close(clientfd);
                clients[client] = 0;
            }
        }
    }
    // Never reached
    close(serverfd);
    puts("Server exits");
    return 0;
}

