#ifndef SHARED_H
#define SHARED_H

#include <stdint.h>

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 1234 

#define CLIENT_TIMEOUT 60

#define MAX_CLIENTS 10
#define BUFFER_SIZE 32768

int unblock(int);
uint16_t string_to_uint16(const char *);

#endif

