// gcc -std=c11 -Wall -pedantic -o demo demo.c -lpthread

#include <stdio.h>
#include <pthread.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static int count = 0;

void *thread_handler(void *arg)
{
	(void) arg;
	pthread_mutex_lock(&mutex);
	printf("%d\n", ++count);
	pthread_mutex_unlock(&mutex);
	return NULL;
}

int main(void)
{
    enum {NTHREADS = 20};
	pthread_t thread[NTHREADS];

	for (int i = 0; i < NTHREADS; i++)
    {
		pthread_create(&thread[i], NULL, thread_handler, NULL);
	}
	for (int i = 0; i < NTHREADS; i++)
    {
		pthread_join(thread[i], NULL);
	}
	return 0;
}

