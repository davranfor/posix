#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    int fd[2];
    pid_t pid;

    if (pipe(fd) == -1)
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
        if (dup2(fd[1], STDOUT_FILENO) == -1)
        {
            perror("dup2");
            exit(EXIT_FAILURE);
        }
        close(fd[0]); // close the writing side of the pipe
        close(fd[1]); // close the reading side of the pipe
        execl("/bin/ls", "ls", "-l", (char *)NULL);
    }
    else
    {
        /* start of parent process */
        close(fd[1]); // close the writing side of the pipe

        FILE *cmd = fdopen(fd[0], "r");

        if (cmd == NULL)
        {
            perror("fdopen");
            exit(EXIT_FAILURE);
        }

        char str[1024];

        while (fgets(str, sizeof str, cmd))
        {
            printf("output: %s", str);
        }
        fclose(cmd); // close the reading side of the pipe
    }
    waitpid(pid, NULL, 0);
}

