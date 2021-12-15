/*
gcc -Wall -Wextra -o regex regex.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <regex.h>

int main(void)
{
    regex_t regex;
    int result;

    // Compile regular expression
    result = regcomp(&regex, "^a[[:alnum:]]", 0);
    if (result != 0)
    {
        perror("regcomp");
        exit(EXIT_FAILURE);
    }
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
        char msg[100];

        regerror(result, &regex, msg, sizeof(msg));
        perror("regexec");
        exit(EXIT_FAILURE);
    }
    // Free regular expression
    regfree(&regex);
    return 0;
}

