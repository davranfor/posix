#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include "shared.h"

ssize_t sendall(int sockfd, const void *buf, size_t size)
{
    size_t sent = 0;

    while (sent < size)
    {
        ssize_t bytes = send(sockfd, ((const unsigned char *)buf) + sent, size - sent, 0);

        if (bytes < 1)
        {
            return bytes;
        }
        sent += (size_t)bytes;
    }
    return (ssize_t)sent;
}

ssize_t recvall(int sockfd, void *buf, size_t size)
{
    size_t rcvd = 0;

    while (rcvd < size)
    {
        ssize_t bytes = recv(sockfd, ((unsigned char *)buf) + rcvd, size - rcvd, 0);

        if (bytes < 1)
        {
            return bytes;
        }
        rcvd += (size_t)bytes;
    }
    return (ssize_t)rcvd;
}

ssize_t sendstr(int sockfd, const char *str)
{
    size_t size = strlen(str) + 1;
    uint16_t map = htons((uint16_t)size);
    ssize_t bytes;

    bytes = sendall(sockfd, &map, sizeof map);
    if (bytes != sizeof map)
    {
        return bytes;
    }
    return sendall(sockfd, str, size);
}

ssize_t recvstr(int sockfd, char *str)
{
    ssize_t bytes;
    uint16_t map;

    bytes = recvall(sockfd, &map, sizeof map);
    if (bytes != sizeof map)
    {
        return bytes;
    }
    map = ntohs(map);
    bytes = recvall(sockfd, str, map);
    if (bytes < 1)
    {
        return bytes;
    }
    return bytes - 1;
}

