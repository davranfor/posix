#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    int a[2], b[2];
    char str[128];
    ssize_t len;
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
        while ((len = read(a[0], str, sizeof str)) > 0)
        {
            write(STDOUT_FILENO, str, (size_t)len);
        }
        close(a[0]);
        strcpy(str, "Child talking through a pipe.\n");
        write(b[1], str, strlen(str));
        close(b[1]);
    }
    else
    {
        /* start of parent process */
        close(a[0]); // close the reading side of the pipe
        close(b[1]); // close the writing side of the pipe
        strcpy(str, "Parent talking through a pipe.\n");
        write(a[1], str, strlen(str));
        close(a[1]);
        while ((len = read(b[0], str, sizeof str)) > 0)
        {
            write(STDOUT_FILENO, str, (size_t)len);
        }
        close(b[0]);
    }
    waitpid(pid, NULL, 0);
}

