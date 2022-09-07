#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>
#include "shared.h"

static void handler(int clientfd)
{
    char str[BUFFER_SIZE];

    while (1)
    {
        ssize_t size;

        if ((size = recvstr(clientfd, str)) <= 0)
        {
            if (size == -1)
            {
                perror("recvstr");
            }
            break;
        }
        printf("Size: %05zd | Client says: %s\n", size, str);
        if ((size = sendstr(clientfd, str)) <= 0)
        {
            if (size == -1)
            {
                perror("sendstr");
            }
            break;
        }
    }
    close(clientfd);
}

static void sigchld_handler(int signum)
{
    (void)signum;

    // waitpid() might overwrite errno, so we save and restore it
    int saved_errno = errno;

    while (waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

int main(void)
{
    struct sigaction sa;

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

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
    while (1)
    {
        int clientfd = accept(serverfd, NULL, NULL);

        if (clientfd == -1)
        {
            perror("accept");
            break;
        }

        pid_t pid = fork();

        if (pid == -1)
        {
            perror("fork");
            break;
        }
        if (pid == 0)
        {
            // Child
            close(serverfd);
            handler(clientfd);
            _exit(EXIT_SUCCESS);
        }
        else
        {
            // Parent
            close(clientfd);
        }
    }
    // Never reached
    close(serverfd);
    puts("Server exits");
    return 0;
}

