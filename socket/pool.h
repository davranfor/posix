#ifndef POOL_H
#define POOL_H

struct poolfd
{
    char *data;
    size_t size, sent;
    unsigned type;
};

enum {POOL_BUFFERED = 1, POOL_ALLOCATED};

void pool_set(struct poolfd *, char *, size_t);
int pool_add(struct poolfd *, const char *, size_t);
void pool_sync(struct poolfd *, size_t);
void pool_reset(struct poolfd *);

#endif

