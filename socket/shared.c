#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include "shared.h"

char *sockbuff_init(struct sockbuff *buff)
{
    memset(buff, 0, sizeof *buff);
    return buff->data + 2;
}

ssize_t sendall(int sockfd, char *str)
{
    ssize_t res = (ssize_t)strlen(str);
    uint16_t map = htons((uint16_t)res);

    str -= 2;
    memcpy(str, &map, 2);

    ssize_t len = res + 2;

    while (len > 0)
    {
        ssize_t bytes = send(sockfd, str, (size_t)len, 0);

        if (bytes < 1)
        {
            return bytes;
        }
        str += bytes;
        len -= bytes;
    }
    return res;
}

ssize_t recvall(int sockfd, char *str)
{
    uint16_t map;
    ssize_t len;

    if ((len = recv(sockfd, &map, sizeof map, 0)) != sizeof map)
    {
        return len == -1 ? -1 : 0;
    }
    len = ntohs(map);

    ssize_t res = 0;

    while (res < len)
    {
        ssize_t bytes = recv(sockfd, str + res, (size_t)(len - res), 0);

        if (bytes < 1)
        {
            return bytes;
        }
        res += bytes;
    }
    str[res] = '\0';
    return res;
}

