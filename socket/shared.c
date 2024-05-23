#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "shared.h"

void pool_set(struct poolfd *pool, char *data, size_t size)
{
    if (pool->type == POOL_ALLOCATED)
    {
        free(pool->data);
    }
    pool->type = POOL_BUFFERED;
    pool->data = data;
    pool->size = size;
}

int pool_add(struct poolfd *pool, const char *data, size_t size)
{
    if (pool->type == POOL_BUFFERED)
    {
        pool->data = NULL;
        pool->size = 0;
    }
    pool->type = POOL_ALLOCATED;

    char *temp = realloc(pool->data, pool->size + size);

    if (temp == NULL)
    {
        return 0;
    }
    pool->data = temp;
    memcpy(pool->data + pool->size, data, size);
    pool->size += size;
    return 1;
}

void pool_sync(struct poolfd *pool, size_t sent)
{
    pool->sent += sent;
}

void pool_reset(struct poolfd *pool)
{
    if (pool->type == POOL_ALLOCATED)
    {
        free(pool->data);
    }
    pool->data = NULL;
    pool->size = 0;
    pool->sent = 0;
    pool->type = 0;
}

uint16_t string_to_uint16(const char *str)
{
    char *end;
    unsigned long result = strtoul(str, &end, 10);

    if ((result > 65535) || (end[strspn(end, " \f\n\r\t\v")] != '\0'))
    {
        return 0;
    }
    return (uint16_t)result;
}

int unblock(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);

    if (flags == -1)
    {
        return -1;
    }
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

