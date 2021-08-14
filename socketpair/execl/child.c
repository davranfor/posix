#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    (void)argc;

    int fd = argv[1][0];
    char str[128];
    ssize_t n = 0;

    while ((n = read(fd, str, sizeof str)) != -1)
    {
        str[n] = '\0';
        printf("parent says %s\n", str);
        if (strcmp("quit", str) == 0)
        {
            puts("Bye!");
            close(fd);
            break;
        }
        if (write(fd, str, strlen(str)) == -1)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }
    }
    return 0;
}

