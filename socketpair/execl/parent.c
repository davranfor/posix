#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(void)
{
    char str[128];
    char *words[] = {"one", "two", "three", "quit"};
    char **word = words;

    for (;;)
    {
        if (write(0, *word, strlen(*word)) == -1)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }
        if (read(1, str, sizeof str) <= 0)
        {
            break;
        }
        word++;
    }
    return 0;
}

