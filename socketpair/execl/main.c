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

    printf("fd[0] = %d\n", fd[0]);
    printf("fd[1] = %d\n", fd[1]);

    char args[2] = "";
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
        args[0] = (char)fd[0];

        execl("./child", "child", args, (char *)NULL);
        perror("execl");
    }
    else
    {
        puts("I'm, the parent");

        close(fd[0]);
        args[0] = (char)fd[1];

        execl("./parent", "parent", args, (char *)NULL);
        perror("execl");
    }
    return 0;
}

