// gcc -std=c11 -Wall -pedantic -o demo atomic.c -lpthread

#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <pthread.h>

static signed int normal_counter;
static atomic_int atomic_counter;

void *thread_handler(void *arg)
{
    (void)arg;
    for (int i = 0; i < 10000; i++)
    {
        ++normal_counter; // Undefined behavior
        ++atomic_counter;
    }
    return NULL;
}

int main(void)
{
    enum {NTHREADS = 10};
    pthread_t thread[NTHREADS];

    for (int i = 0; i < NTHREADS; i++)
    {
        if (pthread_create(&thread[i], NULL, thread_handler, NULL) != 0)
        {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }
    for (int i = 0; i < NTHREADS; i++)
    {
        if (pthread_join(thread[i], NULL) != 0)
        {
            perror("pthread_join");
            exit(EXIT_FAILURE);
        }
    }
    printf("Normal counter: %d\n", normal_counter);
    printf("Atomic counter: %d\n", atomic_counter);
    return 0;
}

