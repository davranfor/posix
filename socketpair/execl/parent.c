#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    (void)argc;

    int fd = argv[1][0];
    char *words[] = {"one", "two", "three", "quit"};
    char **word = words;
    char str[128];

    for (;;)
    {
        if (write(fd, *word, strlen(*word)) == -1)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }
        if (read(fd, str, sizeof str) <= 0)
        {
            break;
        }
        word++;
    }
    return 0;
}

