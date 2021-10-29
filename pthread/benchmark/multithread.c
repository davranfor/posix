#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

struct primes
{
    size_t size;
    int *data;
};

static struct primes primes;

static size_t next_size(size_t size)
{
    if ((size & (size - 1)) == 0)
    {
        return size == 0 ? 1 : size << 1;
    }
    return size;
}

static void add_prime(int number)
{
    size_t capacity = next_size(primes.size);
 
    if (primes.size != capacity)
    {
        int *data = realloc(primes.data, sizeof *data * capacity);

        if (data == NULL)
        {
            perror("realloc");
            exit(EXIT_FAILURE);
        }
        primes.data = data;
    }
    primes.data[primes.size++] = number;
}

static int compare_prime(const void *pa, const void *pb)
{
    int a = *(int *)pa;
    int b = *(int *)pb;

    return a < b ? -1 : a > b;
}
 
static bool is_prime(int number)
{
    if (number <= 1)
    {
        return false;
    }
    for (int iter = 3; iter < number / 2; iter += 2)
    {
        if ((number % iter) == 0)
        {
            return false;
        }
    }
    return true;
}

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct params { int min, max; };

void *thread_handler(void *args)
{
    struct params *params = args;

    for (int number = params->min; number <= params->max; number += 2)
    {
        if (is_prime(number))
        {
            if (pthread_mutex_lock(&mutex) != 0)
            {
                perror("pthread_mutex_lock");
                exit(EXIT_FAILURE);
            }
            add_prime(number);
            if (pthread_mutex_unlock(&mutex) != 0)
            {
                perror("pthread_mutex_unlock");
                exit(EXIT_FAILURE);
            }
        }
    }
    return NULL;
}

void clean(void)
{
    free(primes.data);
}

#define NTHREADS 4
 
int main(void)
{
    atexit(clean);

    struct params params[NTHREADS];
    pthread_t thread[NTHREADS];

    int min = 1, max = 10000;
    int len = (max - min + 1) / NTHREADS;

    add_prime(2);
    for (size_t task = 0; task < NTHREADS; task++)
    {
        params[task].min = min;
        params[task].max = min + len;
        if (pthread_create(&thread[task], NULL, thread_handler, &params[task]) != 0)
        {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
        min += len + 1;
        // Adjust min to an odd number
        if ((min % 2) == 0)
        {
            min++;
        }
        // Adjust len to not exceed max
        if ((min + len) > max)
        {
            len = max - min;
        }
    }
    for (size_t task = 0; task < NTHREADS; task++)
    {
        if (pthread_join(thread[task], NULL) != 0)
        {
            perror("pthread_join");
            exit(EXIT_FAILURE);
        }
    }
    qsort(primes.data, primes.size, sizeof(int), compare_prime);
    printf("Nr of primes = %zu:\n", primes.size);
    for (size_t iter = 0; iter < primes.size; iter++)
    {
        printf("%d ", primes.data[iter]);
    }
    printf("\n");
    return 0;
}

