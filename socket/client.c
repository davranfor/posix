#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 8888
#define CLIENT_BUFFER 1024

int main(void)
{
    struct sockaddr_in server;

	memset(&server, 0, sizeof server);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(SERVER_ADDR);
    server.sin_port = htons(SERVER_PORT);

    int serverfd;

    if ((serverfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    if (connect(serverfd, (struct sockaddr *)&server, sizeof server) == -1)
    {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    char str[CLIENT_BUFFER];

    while (fgets(str, sizeof str, stdin) != NULL)
    {
        str[strcspn(str, "\n")] = '\0';
        if (str[0] == '\0')
        {
            continue;
        }
        if (send(serverfd, str, strlen(str), 0) == -1)
        {
            perror("send");
            exit(EXIT_FAILURE);
        }

        ssize_t len = recv(serverfd, str, sizeof(str) - 1, 0);

        if (len == -1)
        {
            perror("recv");
            exit(EXIT_FAILURE);
        }
        if (len == 0)
        {
            break;
        }
        str[len] = '\0';
        printf("Length = %05zd | Server says: %s\n", len, str);
    }
    puts("Client exits");
    return 0;
}

