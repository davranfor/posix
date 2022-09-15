#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

    int pid;

    if ((pid = fork()) == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pid == 0)
    {
        puts("I'm the child");
        close(fd[0]);

        char arg[2];

        snprintf(arg, sizeof arg, "%d", fd[1]);
        execl("/usr/bin/python3", "python", "main.py", arg, (char *)NULL);
        perror("execl");
    }
    else
    {
        puts("I'm the parent");
        close(fd[1]);

        char *words[] = {"one", "two", "three", "quit"};
        char **word = words;
        char str[128] = {0};

        for (;;)
        {
            if (write(fd[0], *word, strlen(*word)) == -1)
            {
                perror("write");
                exit(EXIT_FAILURE);
            }
            switch (read(fd[0], str, sizeof str))
            {
                case -1:
                    perror("read");
                    exit(EXIT_FAILURE);
                case 0:
                    close(fd[0]);
                    waitpid(pid, NULL, 0);
                    puts("Parent says Bye!");
                    exit(EXIT_SUCCESS);
                default:
                    break;
            }
            word++;
        }
    }
}

