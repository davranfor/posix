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
        if (dup2(a[0], STDIN_FILENO) == -1)
        {
            perror("dup2");
            _exit(EXIT_FAILURE);
        }
        close(a[0]);
        if (dup2(b[1], STDOUT_FILENO) == -1)
        {
            perror("dup2");
            _exit(EXIT_FAILURE);
        }
        close(b[1]);
        execl("/bin/bc", "bc", "--quiet", (char *)NULL);
        perror("execl");
    }
    else
    {
        /* start of parent process */
        close(a[0]); // close the reading side of the pipe
        close(b[1]); // close the writing side of the pipe

        const char *ops[] = {"123456789 * 10\n", "sqrt(91)\n", "13 ^ 42\n", "quit\n"};
        char str[1024];

        for (size_t op = 0; op < sizeof ops / sizeof ops[0]; op++)
        {
            if (write(a[1], ops[op], strlen(ops[op])) == -1)
            {
                perror("write");
                exit(EXIT_FAILURE);
            }

            ssize_t len = read(b[0], str, sizeof(str) - 1);

            if (len == -1)
            {
                perror("read");
                exit(EXIT_FAILURE);
            }
            if (len == 0)
            {
                break;
            }
            str[len] = '\0';
            printf("%s = %s", ops[op], str);
        }
        close(a[1]);
        close(b[0]);
        waitpid(pid, NULL, 0);
    }
    return 0;
}

