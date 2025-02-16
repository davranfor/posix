#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;
enum { N = 5 };
int value;

static void *reader_thread(void *arg)
{
    printf("Thread %ld trying to read\n", (long)arg);
    pthread_rwlock_rdlock(&rwlock);
    printf("Thread %ld reading value %d\n", (long)arg, value);
    sleep(1);
    pthread_rwlock_unlock(&rwlock);
    printf("Thread %ld is done reading\n", (long)arg);
    return NULL;
}

static void *writer_thread(void *arg)
{
    printf("Thread %ld trying to write\n", (long)arg + N);
    pthread_rwlock_wrlock(&rwlock);
    value = rand();
    printf("Thread %ld writing value %d\n", (long)arg + N, value);
    sleep(1);
    pthread_rwlock_unlock(&rwlock);
    printf("Thread %ld is done writing\n", (long)arg + N);
    return NULL;
}

int main(void)
{
    srand((unsigned)time(NULL));

    pthread_t readers[N], writers[N];

    for (long i = 0; i < N; i++)
    {
        pthread_create(&readers[i], NULL, reader_thread, (void *)i);
        pthread_create(&writers[i], NULL, writer_thread, (void *)i);
    }
    for (int i = 0; i < N; i++)
    {
        pthread_join(readers[i], NULL);
        pthread_join(writers[i], NULL);
    }
    pthread_rwlock_destroy(&rwlock);
    return 0;
}

