#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define pthread_call(func, ...)         \
    do if (pthread_##func(__VA_ARGS__)) \
    {                                   \
        perror("pthread_"#func);        \
        exit(EXIT_FAILURE);             \
    } while (0)

static pthread_mutex_t mutex;
static pthread_cond_t empty;
static pthread_cond_t full;

#define BUFFER_SIZE 10

static int buffer[BUFFER_SIZE];
static int size = 0;

static void *produce(void *arg)
{
    (void)arg;

    int quit = 0;

    while (!quit)
    {
        pthread_call(mutex_lock, &mutex);
        while (size == BUFFER_SIZE)
        {
            // Buffer is full, wait for consumer to consume items
            pthread_call(cond_wait, &full, &mutex);
        }
        while (size < BUFFER_SIZE)
        {
            int item = rand() % 100;

            buffer[size] = item;
            printf("%02d) Produced: %d\n", size++, item);
            if (item == 0)
            {
                quit = 1;
                break;
            }
        }
        // Signal consumer that buffer is full
        pthread_call(cond_signal, &empty);
        pthread_call(mutex_unlock, &mutex);
    }
    pthread_exit(NULL);
}

static void *consume(void *arg)
{
    (void)arg;

    int quit = 0;

    while (!quit)
    {
        pthread_call(mutex_lock, &mutex);
        while (size == 0)
        {
            // Buffer is empty, wait for producer to produce items
            pthread_call(cond_wait, &empty, &mutex);
        }
        for (int i = 0; i < size; i++)
        {
            int item = buffer[i];

            printf("%02d) Consumed: %d\n", i, item);
            if (item == 0)
            {
                quit = 1;
                break;
            }
        }
        size = 0;
        // Signal producer that buffer is empty
        pthread_call(cond_signal, &full);
        pthread_call(mutex_unlock, &mutex);
    }
    pthread_exit(NULL);
}

int main(void)
{
    srand((unsigned)time(NULL));

    pthread_t producer, consumer;

    pthread_call(mutex_init, &mutex, NULL);
    pthread_call(cond_init, &empty, NULL);
    pthread_call(cond_init, &full, NULL);
    pthread_call(create, &producer, NULL, produce, NULL);
    pthread_call(create, &consumer, NULL, consume, NULL);
    pthread_call(join, producer, NULL);
    pthread_call(join, consumer, NULL);
    pthread_call(cond_destroy, &empty);
    pthread_call(cond_destroy, &full);
    pthread_call(mutex_destroy, &mutex);
    return 0;
}

