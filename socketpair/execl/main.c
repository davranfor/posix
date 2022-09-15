#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    int fd[2];

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd) == -1)
    {
        perror("socketpair");
        exit(EXIT_FAILURE);
    }

    char arg[2] = {0};
    int pid[2];

    // Reader
    if ((pid[0] = fork()) == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pid[0] == 0)
    {
        puts("I'm the reader");
        close(fd[1]);
        arg[0] = (char)fd[0];
        execl("./reader", "reader", arg, (char *)NULL);
        perror("execl");
    }
    // Writer
    if ((pid[1] = fork()) == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pid[1] == 0)
    {
        puts("I'm the writer");
        close(fd[0]);
        arg[0] = (char)fd[1];
        execl("./writer", "writer", arg, (char *)NULL);
        perror("execl");
    }
    // Parent
    close(fd[0]);
    close(fd[1]);
    waitpid(pid[0], NULL, 0);
    waitpid(pid[1], NULL, 0);
    puts("Parent says Bye!");
    return 0;
}

