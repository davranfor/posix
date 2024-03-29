#ifndef SHARED_H
#define SHARED_H

#include <sys/types.h>

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 8888
#define SERVER_LISTEN 10

#define BUFFER_SIZE 65536

ssize_t sendall(int, const void *, size_t);
ssize_t recvall(int, void *, size_t);
ssize_t sendstr(int, const char *);
ssize_t recvstr(int, char *);

#endif /* SHARED_H */

