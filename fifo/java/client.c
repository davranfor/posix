#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

int main(void)
{
    int server = open("server.fifo", O_WRONLY);

    if (server == -1)
    {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    int client = open("client.fifo", O_RDONLY);

    if (client == -1)
    {
        close(server);
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    char str[128] = {0};

    puts("echo C-Java, please, type something and press enter");
    while (fgets(str, sizeof str, stdin))
    {
        if (strcmp(str, "quit\n") == 0)
        {
            break;
        }
        if (write(server, str, strlen(str)) == -1)
        {
            perror("write");
            exit(EXIT_FAILURE);
        }

        ssize_t len;

        len = read(client, str, sizeof(str) - 1);
        if (len == -1)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }
        str[len] = '\0';
        printf("%s", str);
    }
    close(server);
    close(client);
    return 0;
}

