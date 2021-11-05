#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

#define ARRAY_MAX 100

static int array[ARRAY_MAX];
static int count = 1;

void *producer(void *args)
{
    (void)args;

    while (1)
    {
        int index = rand() % ARRAY_MAX;

        if (pthread_mutex_lock(&mutex) != 0)
        {
            perror("pthread_mutex_lock");
            exit(EXIT_FAILURE);
        }
        array[count++] = index;
        if (pthread_cond_wait(&cond, &mutex) != 0)
        {
            perror("pthread_cond_wait");
            exit(EXIT_FAILURE);
        }
        if (pthread_mutex_unlock(&mutex) != 0)
        {
            perror("pthread_mutex_unlock");
            exit(EXIT_FAILURE);
        }
    }
}

void *consumer(void *args)
{
    (void)args;
    while (1)
    {
        if (pthread_mutex_lock(&mutex) != 0)
        {
            perror("pthread_mutex_lock");
            exit(EXIT_FAILURE);
        }
        printf("%d\n", array[--count]);
        if (pthread_cond_signal(&cond) != 0)
        {
            perror("pthread_cond_signal");
            exit(EXIT_FAILURE);
        }
        if (pthread_mutex_unlock(&mutex) != 0)
        {
            perror("pthread_mutex_unlock");
            exit(EXIT_FAILURE);
        }
        sleep(1);
    }
}

int main(void)
{
    srand((unsigned)time(NULL));

    pthread_t th1, th2;

    if (pthread_create(&th1, NULL, &producer, NULL) != 0)
    {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&th2, NULL, &consumer, NULL) != 0)
    {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }
    if (pthread_join(th1, NULL) != 0)
    {
        perror("pthread_join");
        exit(EXIT_FAILURE);
    }
    if (pthread_join(th2, NULL) != 0)
    {
        perror("pthread_join");
        exit(EXIT_FAILURE);
    }
    return 0;
}

