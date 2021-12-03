#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(void)
{
    char *myfifo = "/tmp/myfifo";

    if (mkfifo(myfifo, 0666) == -1)
    {
        if (errno != EEXIST)
        {
            perror("mkfifo");
            exit(EXIT_FAILURE);
        }
    }
    puts("Waiting writer");

    int fd;

    if ((fd = open(myfifo, O_RDONLY)) == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    char str[128] = {0};

    while (1)
    {
        ssize_t len;

        if ((len = read(fd, str, sizeof(str) - 1)) == -1)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }
        str[len] = '\0';
        printf("%s", str);
        if (strcmp(str, "quit\n") == 0)
        {
            break;
        }
    }
    if (close(fd) == -1)
    {
        perror("close");
        exit(EXIT_FAILURE);
    }
    return 0;
}

