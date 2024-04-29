#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "shared.h"

int pool_add(struct poolfd *pool, const char *data, size_t size)
{
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
    free(pool->data);
    pool->data = NULL;
    pool->size = 0;
    pool->sent = 0;
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

