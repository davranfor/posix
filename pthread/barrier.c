#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NTHREADS 4

static pthread_barrier_t barrier;

void *handler(void *arg)
{
    int id = *(int *)arg;
    int a = id * 10;
    int b = a + 10;

    // Print pairs
    for (int number = a; number < b; number += 2)
    {
        printf("thread %d: %d\n", id, number);
    }

    // Wait until all the threads have printed the pairs 
    int error = pthread_barrier_wait(&barrier);

    if ((error != 0) && (error != PTHREAD_BARRIER_SERIAL_THREAD))
    {
        perror("pthread_barrier_wait");
        exit(EXIT_FAILURE);
    }

    // Print odds
    for (int number = a + 1; number < b; number += 2)
    {
        printf("thread %d: %d\n", id, number);
    }
    return NULL;
}

int main(void)
{
    pthread_t thread[NTHREADS];
    int thread_id[NTHREADS];

    if (pthread_barrier_init(&barrier, NULL, NTHREADS) != 0)
    {
        perror("pthread_barrier_init");
        exit(EXIT_FAILURE);
    }
    for (int id = 0; id < NTHREADS; id++)
    {
        thread_id[id] = id + 1;
        if (pthread_create(&thread[id], NULL, handler, &thread_id[id]) != 0)
        {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    // The main process can also use a barrier at this point in order
    // to synchronize with the rest of the threads, to do that:
    // - Pass `NTHREADS + 1` in pthread_barrier_init()
    // - Call pthread_barrier_wait() here

    for (int id = 0; id < NTHREADS; id++)
    {
        if (pthread_join(thread[id], NULL) != 0)
        {
            perror("pthread_join");
            exit(EXIT_FAILURE);
        }
    }
    if (pthread_barrier_destroy(&barrier) != 0)
    {
        perror("pthread_barrier_destroy");
        exit(EXIT_FAILURE);
    }
    return 0;
}

