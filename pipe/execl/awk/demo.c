#define _POSIX_C_SOURCE 200809L // for getdelim()

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    int a[2], b[2];

    if ((pipe(a) == -1) || (pipe(b) == -1))
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid;

    if ((pid = fork()) == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pid == 0)
    {
        /* start of child process */
        close(a[1]); // close the writing side of the pipe
        close(b[0]); // close the reading side of the pipe

        char arg1[4], arg2[4];

        snprintf(arg1, sizeof arg1, "%d", a[0]);
        snprintf(arg2, sizeof arg2, "%d", b[1]);
        execl("/usr/bin/awk", "awk", "-f", "run.awk", arg1, arg2, (char *)NULL);
        perror("execl");
    }
    else
    {
        /* start of parent process */
        close(a[0]); // close the reading side of the pipe
        close(b[1]); // close the writing side of the pipe

        FILE *fdr = fdopen(b[0], "r");

        if (fdr == NULL)
        {
            perror("fdopen");
            exit(EXIT_FAILURE);
        }

        FILE *fdw = fdopen(a[1], "w");

        if (fdw == NULL)
        {
            perror("fdopen");
            exit(EXIT_FAILURE);
        }

        char *str = NULL;
        ssize_t len = 0;
        size_t size = 0;

        for (int i = 0; i <= 10; i++)
        {
            fprintf(fdw, "%d %d\v", i, 10);
            fflush(fdw);
            if ((len = getdelim(&str, &size, '\v', fdr)) < 1)
            {
                perror("getdelim");
                break;
            }
            str[len - 1] = '\0';
            printf("%s\n", str);
        }
        free(str);
        close(a[1]);
        close(b[0]);
        waitpid(pid, NULL, 0);
    }
    return 0;
}
