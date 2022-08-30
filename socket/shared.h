#ifndef SHARED_H
#define SHARED_H

#include <sys/types.h>

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 8888
#define SERVER_LISTEN 10

#define BUFFER_SIZE 65534
#define BUFFER_SIZE_MAP 2

struct sockbuff { char data[BUFFER_SIZE + BUFFER_SIZE_MAP]; };
char *sockbuff_init(struct sockbuff *);

ssize_t sendall(int, char *);
ssize_t recvall(int, char *);

#endif /* SHARED_H */

