#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    (void)argc;

    int fd = argv[1][0];
    char str[128] = {0};
    ssize_t len = 0;

    while ((len = read(fd, str, sizeof str)) != -1)
    {
        str[len] = '\0';
        printf("Parent says %s\n", str);
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

