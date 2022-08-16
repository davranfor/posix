#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

int main(void)
{
    char *myfifo = "/tmp/myfifo";
    int fd;

    if ((fd = open(myfifo, O_WRONLY)) == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    char str[128];

    while (fgets(str, sizeof str, stdin))
    {
        if (write(fd, str, strlen(str)) == -1)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }
        if (strcmp(str, "quit\n") == 0)
        {
            break;
        }
    }
    close(fd);
    return 0;
}

