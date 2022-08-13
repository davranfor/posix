#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

static int done = 0;

void *handler(void *args)
{
    (void)args;
    puts("thread doing stuff ...");
    if (pthread_mutex_lock(&mutex) != 0)
    {
        perror("pthread_mutex_lock"); 
        exit(EXIT_FAILURE);
    }
    while (!done)
    {
        if (pthread_cond_wait(&cond, &mutex) != 0)
        {
            perror("pthread_cond_wait");
            exit(EXIT_FAILURE);
        }
    }
    if (pthread_mutex_unlock(&mutex) != 0)
    {
        perror("pthread_mutex_unlock");
        exit(EXIT_FAILURE);
    }
    puts("thread doing more stuff ...");
    return NULL;
}

int main(void)
{
    pthread_t thread;

    if (pthread_create(&thread, NULL, handler, NULL) != 0)
    {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }
    if (pthread_mutex_lock(&mutex) != 0)
    {
        perror("pthread_mutex_lock");
        exit(EXIT_FAILURE);
    }
    sleep(1);
    done = 1;
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
    if (pthread_join(thread, NULL) != 0)
    {
        perror("pthread_join");
        exit(EXIT_FAILURE);
    }
    return 0;
}

