#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <pthread.h>

static atomic_int atomic_counter;

void *thread_handler(void *arg)
{
    (void)arg;
    for (int i = 0; i < 10000; i++)
    {
        atomic_fetch_add(&atomic_counter, 1);
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
    printf("Atomic counter: %d\n", atomic_counter);
    return 0;
}

