#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    (void)argc;

    char *words[] = {"one", "two", "three", "quit"};
    char **word = words;
    int fd = argv[1][0];
    char str[128] = {0};

    for (;;)
    {
        if (write(fd, *word, strlen(*word)) == -1)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }
        switch (read(fd, str, sizeof str))
        {
            case -1:
                perror("read");
                exit(EXIT_FAILURE);
            case 0:
                close(fd);
                puts("Writer says Bye!");
                exit(EXIT_SUCCESS);
            default:
                break;
        }
        word++;
    }
}

