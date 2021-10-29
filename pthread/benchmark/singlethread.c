#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

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

void clean(void)
{
    free(primes.data);
}

int main(void)
{
    atexit(clean);

    int min = 1, max = 10000;

    add_prime(2);
    for (int number = min; number <= max; number += 2)
    {
        if (is_prime(number))
        {
            add_prime(number);
        }
    }
    printf("Nr of primes = %zu:\n", primes.size);
    for (size_t iter = 0; iter < primes.size; iter++)
    {
        printf("%d ", primes.data[iter]);
    }
    printf("\n");
    return 0;
}

