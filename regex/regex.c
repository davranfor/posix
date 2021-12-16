/*
gcc -Wall -Wextra -o regex regex.c

https://man7.org/linux/man-pages/man3/regcomp.3.html
*/

#include <stdio.h>
#include <stdlib.h>
#include <regex.h>

int main(void)
{
    regex_t regex;
    char msg[100];
    int result;

    // Compile regular expression
    result = regcomp(&regex, "^a[[:alnum:]]", 0);
    if (result != 0)
    {
        regerror(result, &regex, msg, sizeof(msg));
        fprintf(stderr, "%s\n", msg);
    }
    else
    {
        // Execute regular expression
        result = regexec(&regex, "abc", 0, NULL, 0);
        if (result == 0)
        {
            puts("Match");
        }
        else if (result == REG_NOMATCH)
        {
            puts("No match");
        }
        else
        {
            regerror(result, &regex, msg, sizeof(msg));
            fprintf(stderr, "%s\n", msg);
        }
    }
    // Free regular expression
    regfree(&regex);
    return 0;
}

