#ifndef SHARED_H
#define SHARED_H

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 1234 
#define MAX_CLIENTS 10
#define BUFFER_SIZE 32768

struct poolfd
{
    char *data;
    size_t size, sent;
};

int pool_add(struct poolfd *, const char *, size_t);
void pool_sync(struct poolfd *, size_t);
void pool_reset(struct poolfd *);

int unblock(int);

#endif

