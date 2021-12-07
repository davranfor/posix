#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

static int server = -1;
static int client = -1;

static void die(const char *error)
{
    perror(error);
    exit(EXIT_FAILURE);
}

static void clean(void)
{
    if (server != -1)
    {
        close(server);
    }
    if (client != -1)
    {
        close(client);
    }
}

int main(void)
{
    atexit(clean);
    if ((server = open("server.fifo", O_WRONLY)) == -1)
    {
        die("open server");
    }
    if ((client = open("client.fifo", O_RDONLY)) == -1)
    {
        die("open client");
    }

    char str[128] = {0};

    puts("Please, type an expression e.g. 3*2 and press enter");
    while (fgets(str, sizeof str, stdin))
    {
        if (strcmp(str, "quit\n") == 0)
        {
            break;
        }
        if (write(server, str, strlen(str)) == -1)
        {
            die("write");
        }

        ssize_t len;

        len = read(client, str, sizeof(str) - 1);
        if (len == -1)
        {
            die("read");
        }
        str[len] = '\0';
        printf("%s", str);
    }
    return 0;
}

