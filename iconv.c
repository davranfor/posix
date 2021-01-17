#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <iconv.h>

/*
Compile:
gcc -std=c11 -pedantic -Wall -Wextra -o demo iconv.c
*/

static iconv_t conv = (iconv_t)0;

static void close_conv(void)
{
    iconv_close(conv);
}

static int open_conv(void)
{
    conv = iconv_open("ASCII//TRANSLIT", "UTF-8");
    if (conv == (iconv_t)-1)
    {
        return 0;
    }
    atexit(close_conv);
    return 1;
}

char *decode(char *ptr, char *str)
{
    size_t len1 = strlen(str);
    size_t len2 = len1;
    char *new = NULL;

    if (ptr == NULL)
    {
        ptr = new = calloc(len2 + 1, 1);
        if (ptr == NULL)
        {
            return NULL;
        }
    }

    char *temp = ptr;

    if (iconv(conv, &str, &len1, &ptr, &len2) == (size_t)-1)
    {
        if (new != NULL)
        {
            free(new);
        }
        return NULL;
    }
    return temp;
}

int main(void)
{
    setlocale(LC_CTYPE, "");
    if (open_conv() == 0)
    {
        perror("open_conv");
        exit(EXIT_FAILURE);
    }

    char *str = "El cañón de María vale 1000 €";

    puts(str);
    str = decode(NULL, str);
    if (str == NULL)
    {
        perror("conv");
        exit(EXIT_FAILURE);
    }
    puts(str);
    free(str);
    return 0;
}

