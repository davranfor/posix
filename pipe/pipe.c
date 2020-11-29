#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    char buffer[512];
    int a[2], b[2];
    ssize_t bytes;
    pid_t pid;

    if (pipe(a) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    if (pipe(b) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    if ((pid = fork()) == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pid == 0)
    {
        /* start of child process */
        close(a[1]); // close the writing side of the pipe
        close(b[0]); // close the reading side of the pipe
        while ((bytes = read(a[0], buffer, sizeof buffer)) > 0)
        {
            write(STDOUT_FILENO, buffer, (size_t)bytes);
        }
        close(a[0]);
        strcpy(buffer, "Child talking through a pipe.\n");
        write(b[1], buffer, strlen(buffer));
        close(b[1]);
    }
    else
    {
        /* start of parent process */
        close(a[0]); // close the reading side of the pipe
        close(b[1]); // close the writing side of the pipe
        strcpy(buffer, "Parent talking through a pipe.\n");
        write(a[1], buffer, strlen(buffer));
        close(a[1]);
        while ((bytes = read(b[0], buffer, sizeof buffer)) > 0)
        {
            write(STDOUT_FILENO, buffer, (size_t)bytes);
        }
        close(b[0]);
    }
    waitpid(pid, NULL, 0);
}

