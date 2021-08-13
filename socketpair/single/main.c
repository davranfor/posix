#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
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
    char str[128];

    if ((pid = fork()) == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        puts("I'm, the child");

        close(fd[0]);

        ssize_t n = 0;

        while ((n = read(fd[1], str, sizeof str)) != -1)
        {
            str[n] = '\0';
            printf("parent says %s\n", str);
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
        puts("I'm, the parent");

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

