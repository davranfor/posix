#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

int main(void)
{
    int fd[2];

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd) == -1)
    {
        perror("socketpair");
        exit(EXIT_FAILURE);
    }

    char str[128] = {0};
    int pid;

    if ((pid = fork()) == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        puts("I'm the child");

        close(fd[0]);

        ssize_t len = 0;

        while ((len = read(fd[1], str, sizeof(str) - 1)) != -1)
        {
            str[len] = '\0';
            printf("Parent says %s\n", str);
            if (strcmp("quit", str) == 0)
            {
                puts("Bye!");
                close(fd[0]);
                break;
            }
            if (write(fd[1], str, strlen(str)) == -1)
            {
                perror("write");
                exit(EXIT_FAILURE);
            }
        }
    }
    else
    {
        puts("I'm the parent");

        close(fd[1]);

        char *words[] = {"one", "two", "three", "quit"};
        char **word = words;

        for (;;)
        {
            if (write(fd[0], *word, strlen(*word)) == -1)
            {
                perror("write");
                exit(EXIT_FAILURE);
            }
            if (read(fd[0], str, sizeof str) <= 0)
            {
                break;
            }
            word++;
        }
    }
    puts("Exit process");
    return 0;
}

