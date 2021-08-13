#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(void)
{
    char str[128];
    ssize_t n = 0;

    while ((n = read(1, str, sizeof str)) != -1)
    {
        str[n] = '\0';
        fprintf(stderr, "parent says %s\n", str);
        if (strcmp("quit", str) == 0)
        {
            fprintf(stderr, "Bye!\n");
            close(0);
            break;
        }
        if (write(0, str, strlen(str)) == -1)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }
    }
    return 0;
}

