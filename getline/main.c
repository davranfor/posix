#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

int main(void)
{
    FILE *file = fopen("main.c", "rb");

    if (file == NULL)
    {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    char *str = NULL;
    size_t size = 0;
    ssize_t len = 0;
    int line = 1;

    while ((len = getline(&str, &size, file)) != -1)
    {
        printf("%2d) length = %2zd | %s", line++, len, str);
    }
    free(str);
    fclose(file);
    return 0;
}

