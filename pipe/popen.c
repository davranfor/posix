#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    FILE *cmd;

    cmd = popen("ls -l", "r");
    if (cmd == NULL)
    {
        perror("popen");
        exit(EXIT_FAILURE);
    }

    char str[1024];

    while (fgets(str, sizeof str, cmd))
    {
        printf("output: %s", str);
    }
    pclose(cmd);
    return 0;
}

