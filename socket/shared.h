#ifndef SHARED_H
#define SHARED_H

#include <stdint.h>

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 1234 
#define MAX_CLIENTS 10
#define BUFFER_SIZE 32768

#define POOL_BUFFERED    1
#define POOL_ALLOCATED   2

struct poolfd
{
    char *data;
    size_t size, sent;
    unsigned type;
};

void pool_set(struct poolfd *, char *, size_t);
int pool_add(struct poolfd *, const char *, size_t);
void pool_sync(struct poolfd *, size_t);
void pool_reset(struct poolfd *);

uint16_t string_to_uint16(const char *);

int unblock(int);

#endif

