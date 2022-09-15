#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    (void)argc;

    int fd = argv[1][0];
    char str[128] = {0};

    for (;;)
    {
        ssize_t len;

        if ((len = read(fd, str, sizeof(str) - 1)) == -1)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }
        str[len] = '\0';
        printf("Writer sent %s\n", str);
        if (strcmp("quit", str) == 0)
        {
            close(fd);
            puts("Reader says Bye!");
            exit(EXIT_SUCCESS);
        }
        if (write(fd, str, strlen(str)) == -1)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }
    }
}

