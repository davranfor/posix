#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NTHREADS 2

#define call(func, ...) \
    do if (func(__VA_ARGS__)) { perror(#func); exit(EXIT_FAILURE); } while (0)

static pthread_mutex_t mutex;
static pthread_cond_t cond;
static int shared_data;

static void *handler(void *arg)
{
    int thread_id = *(int *)arg;

    call(pthread_mutex_lock, &mutex);
    if (thread_id == 1)
    {
        printf("Thread 1 is waiting for data to be modified...\n");
        while (shared_data == 0)
        {
            call(pthread_cond_wait, &cond, &mutex);
        }
        printf("Thread 1 detected data modification: %d\n", shared_data);
    }
    else
    {
        printf("Thread 2 is modifying the data...\n");
        shared_data = 42;
        printf("Thread 2 modified the data to: %d\n", shared_data);
        call(pthread_cond_signal, &cond);
    }
    call(pthread_mutex_unlock, &mutex);
    pthread_exit(NULL);
}

int main(void)
{
    int thread_id[NTHREADS] = {1, 2};
    pthread_t thread[NTHREADS];

    call(pthread_mutex_init, &mutex, NULL);
    call(pthread_cond_init, &cond, NULL);
    for (int i = 0; i < NTHREADS; i++)
    {
        call(pthread_create, &thread[i], NULL, handler, (void *)&thread_id[i]);
    }
    for (int i = 0; i < NTHREADS; i++)
    {
        call(pthread_join, thread[i], NULL);
    }
    call(pthread_mutex_destroy, &mutex);
    call(pthread_cond_destroy, &cond);
    return 0;
}

