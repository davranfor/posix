#define _GNU_SOURCE // For MAP_ANONYMOUS

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>

#define N 5

int main(void)
{
    int *data = mmap(
        NULL, N * sizeof *data,
        PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS,
        0, 0
    );    

    if (data == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    printf("Initial values: ");
    for (int i = 0; i < N; i++)
    {
        data[i] = i + 1;
        printf("%d ", data[i]);
    }
    printf("\n");

    pid_t pid = fork();

    if (pid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pid == 0)
    {
        // Child updates the values
        for (int i = 0; i < N; i++)
        {
            data[i] = data[i] * 10;
        }
    }
    else
    {
        // Do the parent stuff ...

        // Parent waits for the child to finish
        if (wait(NULL) == -1)
        {
            perror("wait");
            exit(EXIT_FAILURE);
        }
        // Parent prints the updated values
        printf("  Final values: ");
        for (int i = 0; i < N; i++)
        {
            printf("%d ", data[i]);
        }
        printf("\n");
    }

    int error = munmap(data, N * sizeof *data);

    if (error != 0)
    {
        perror("munmap");
        exit(EXIT_FAILURE);
    }
    return 0;
}

