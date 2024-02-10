#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NTHREADS 2

#define pthread_call(func, ...)         \
    do if (pthread_##func(__VA_ARGS__)) \
    {                                   \
        perror("pthread_"#func);        \
        exit(EXIT_FAILURE);             \
    } while (0)

static pthread_mutex_t mutex;
static pthread_cond_t cond;
static int shared_data;

static void *handler(void *arg)
{
    int thread_id = *(int *)arg;

    pthread_call(mutex_lock, &mutex);
    if (thread_id == 1)
    {
        printf("Thread 1 is waiting for data to be modified...\n");
        while (shared_data == 0)
        {
            pthread_call(cond_wait, &cond, &mutex);
        }
        printf("Thread 1 detected data modification: %d\n", shared_data);
    }
    else
    {
        printf("Thread 2 is modifying the data...\n");
        shared_data = 42;
        printf("Thread 2 modified the data to: %d\n", shared_data);
        pthread_call(cond_signal, &cond);
    }
    pthread_call(mutex_unlock, &mutex);
    pthread_exit(NULL);
}

int main(void)
{
    int thread_id[NTHREADS] = {1, 2};
    pthread_t thread[NTHREADS];

    pthread_call(mutex_init, &mutex, NULL);
    pthread_call(cond_init, &cond, NULL);
    for (int i = 0; i < NTHREADS; i++)
    {
        pthread_call(create, &thread[i], NULL, handler, &thread_id[i]);
    }
    for (int i = 0; i < NTHREADS; i++)
    {
        pthread_call(join, thread[i], NULL);
    }
    pthread_call(mutex_destroy, &mutex);
    pthread_call(cond_destroy, &cond);
    return 0;
}

