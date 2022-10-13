#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(void)
{
    int copy;

    if ((copy = dup(STDOUT_FILENO)) == -1)
    {
        perror("dup");
        exit(EXIT_FAILURE);
    }

    int fd;

    if ((fd = open("test.txt", O_CREAT | O_WRONLY | O_TRUNC, 0666)) == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }
    if (dup2(fd, STDOUT_FILENO) == -1)
    {
        perror("dup2");
        exit(EXIT_FAILURE);
    }
    close(fd);
    printf("Writing to file\n");
    fflush(stdout);

    if (dup2(copy, STDOUT_FILENO) == -1)
    {
        perror("dup2");
        exit(EXIT_FAILURE);
    }
    close(copy);
    printf("Writing to console\n");
}

