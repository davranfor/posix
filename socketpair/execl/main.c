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
        if (dup2(fd[0], STDIN_FILENO) != STDIN_FILENO) 
        {
            perror("dup2");
            exit(EXIT_FAILURE);
        }
        if (dup2(fd[0], STDOUT_FILENO) != STDOUT_FILENO) 
        {
            perror("dup2");
            exit(EXIT_FAILURE);
        }
        close(fd[0]);

        execl("./child", "./child", (char *)NULL);
        perror("execl");
    }
    else
    {
        puts("I'm, the parent");

        close(fd[0]);
        if (dup2(fd[1], STDIN_FILENO) != STDIN_FILENO) 
        {
            perror("dup2");
            exit(EXIT_FAILURE);
        }
        if (dup2(fd[1], STDOUT_FILENO) != STDOUT_FILENO) 
        {
            perror("dup2");
            exit(EXIT_FAILURE);
        }
        close(fd[1]);

        execl("./parent", "./parent", (char *)NULL);
        perror("execl");
    }
    return 0;
}

