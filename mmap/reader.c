#define _XOPEN_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <unistd.h>
#include "shared.h"

int main(void) 
{
    int shm_fd = shm_open(SHARED_MAP_NAME, O_RDONLY, 0666);

    if (shm_fd == -1)
    {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    char *map = mmap(0, SHARED_MAP_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);

    if (map == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    printf("%s\n", map);
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
    if (shm_unlink(SHARED_MAP_NAME) == -1)
    {
        perror("shm_unlink");
        exit(EXIT_FAILURE);
    }
    return 0; 
}

