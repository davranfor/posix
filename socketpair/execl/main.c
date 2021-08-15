#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(void)
{
    int fd[2];

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd) == -1)
    {
        perror("socketpair");
        exit(EXIT_FAILURE);
    }

    char arg[2] = {0};
    int pid;

    if ((pid = fork()) == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        puts("I'm, the child");

        close(fd[1]);
        arg[0] = (char)fd[0];

        execl("./child", "child", arg, (char *)NULL);
        perror("execl");
    }
    else
    {
        puts("I'm, the parent");

        close(fd[0]);
        arg[0] = (char)fd[1];

        execl("./parent", "parent", arg, (char *)NULL);
        perror("execl");
    }
    return 0;
}

