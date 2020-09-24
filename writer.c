#define _XOPEN_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include "shared.h"

int main(void)
{
    int shm_fd = shm_open(SHARED_MAP_NAME, O_CREAT | O_RDWR, 0666);
  
    if (shm_fd == -1)
    {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    ftruncate(shm_fd, SHARED_MAP_SIZE);

    char *map = mmap(0, SHARED_MAP_SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);
  
    if (map == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    char *ptr = map;

    ptr += sprintf(ptr, "%s", "Hello world!");
    ptr += sprintf(ptr, "%s", "\nBye world!");

    if (munmap(map, SHARED_MAP_SIZE) == -1)
    {
        perror("munmap");
        exit(EXIT_FAILURE);
    }
    if (close(shm_fd) == -1)
    {
        perror("close");
        exit(EXIT_FAILURE);
    }
    return 0; 
}

