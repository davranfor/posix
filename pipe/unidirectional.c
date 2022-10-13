#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void child_handle(int fd[])
{
    close(fd[0]);
    for (char c = 'a'; c <= 'z'; c++)
    {
        if (write(fd[1], &c, 1) != 1)
        {
            break;
        }
    }
    close(fd[1]);
}

void parent_handle(int fd[])
{
    close(fd[1]);
    while (1)
    {
        char c;

        if (read(fd[0], &c, 1) != 1)
        {
            close(fd[0]);
            break;
        }
        write(STDOUT_FILENO, &c, 1);
    }
    write(STDOUT_FILENO, (char[]){'\n'}, 1);
}

int main(void)
{
    int fd[2];

    if (pipe(fd) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid;

    if ((pid = fork()) == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pid == 0)
    {
        child_handle(fd);
    }
    else
    {
        parent_handle(fd);
        waitpid(pid, NULL, 0);
    }
    return 0;
}

