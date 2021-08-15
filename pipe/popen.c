#include <stdio.h>
#include <stdlib.h>

FILE *popen(const char *, const char *);
int pclose(FILE *);

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

