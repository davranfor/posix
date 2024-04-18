#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

#define pthread_call(func, ...)         \
    do if (pthread_##func(__VA_ARGS__)) \
    {                                   \
        perror("pthread_"#func);        \
        exit(EXIT_FAILURE);             \
    } while (0)

static pthread_mutex_t mutex;
static pthread_cond_t cond_empty;
static pthread_cond_t cond_full;

#define BUFFER_SIZE 10

static int buffer[BUFFER_SIZE];
static int count = 0;

static void *producer(void *arg)
{
    (void)arg;

    int inc = 0, quit = 0;

    while (!quit)
    {
        int item = rand() % 100;

        pthread_call(mutex_lock, &mutex);
        while (count == BUFFER_SIZE)
        {
            // Buffer is full, wait for consumer to consume items
            pthread_call(cond_wait, &cond_full, &mutex);
        }
        buffer[inc++ % BUFFER_SIZE] = item;
        quit = item == 0;
        count++;
        printf("Produced: %d\n", item);
        // Signal consumer that buffer is not empty anymore
        pthread_call(cond_signal, &cond_empty);
        pthread_call(mutex_unlock, &mutex);
    }
    pthread_exit(NULL);
}

static void *consumer(void *arg)
{
    (void)arg;

    int inc = 0, quit = 0;

    while (!quit)
    {
        int item;

        pthread_call(mutex_lock, &mutex);
        while (count == 0)
        {
            // Buffer is empty, wait for producer to produce items
            pthread_call(cond_wait, &cond_empty, &mutex);
        }
        item = buffer[inc++ % BUFFER_SIZE];
        quit = item == 0;
        count--;
        printf("Consumed: %d\n", item);
        // Signal producer that buffer is not full anymore
        pthread_call(cond_signal, &cond_full);
        pthread_call(mutex_unlock, &mutex);
    }
    pthread_exit(NULL);
}

int main(void)
{
    srand((unsigned)time(NULL));

    pthread_t producer_thread, consumer_thread;

    pthread_call(mutex_init, &mutex, NULL);
    pthread_call(cond_init, &cond_empty, NULL);
    pthread_call(cond_init, &cond_full, NULL);
    pthread_call(create, &producer_thread, NULL, producer, NULL);
    pthread_call(create, &consumer_thread, NULL, consumer, NULL);
    pthread_call(join, producer_thread, NULL);
    pthread_call(join, consumer_thread, NULL);
    pthread_call(cond_destroy, &cond_empty);
    pthread_call(cond_destroy, &cond_full);
    pthread_call(mutex_destroy, &mutex);
    return 0;
}

