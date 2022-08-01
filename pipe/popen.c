#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    char result[1024];
    FILE *cmd;

    cmd = popen("ls -l", "r");
    if (cmd == NULL)
    {
        perror("popen");
        exit(EXIT_FAILURE);
    }
    while (fgets(result, sizeof(result), cmd))
    {
        printf("output: %s", result);
    }
    pclose(cmd);
    return 0;
}

