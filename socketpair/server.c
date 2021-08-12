#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(void)
{
    int in[2], out[2];
    int pid;

    if ((socketpair(AF_UNIX, SOCK_STREAM, 0, in) == -1) ||
        (socketpair(AF_UNIX, SOCK_STREAM, 0, out) == -1))
    {
        perror("socketpair");
        exit(EXIT_FAILURE);
    }

    /* Or use a pipe instead of a socketpair
    if ((pipe(in) == -1) || (pipe(out) == -1))
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    */

    if ((pid = fork()) == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        /* This is the child process */

        /* Close stdin, stdout, stderr */
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);

        /* make our pipes, our new stdin, stdout */
        if ((dup2(in[0], 0) == -1) || (dup2(out[1], 1) == -1))
        {
            perror("dup2");
            exit(EXIT_FAILURE);
        }

        /* Close the other ends of the pipes that the parent will use, because if
         * we leave these open in the child, the child/parent will not get an EOF
         * when the parent/child closes their end of the pipe.
         */
        close(in[1]);
        close(out[0]);

        /* Over-write the child process with a bash command */
        execl("/bin/bash", "bash", "client.sh", (char *)NULL);
        perror("execl");
    }
    else
    {
        /* This is the parent process */

        /* Close the pipe ends that the child uses to read from / write to so
         * the when we close the others, an EOF will be transmitted properly.
         */
        close(in[0]);
        close(out[1]);

        char str[] = "Some input data\n"; // newline is important
        char buf[1024];

        for (int i = 0; i < 10; i++)
        {
            /* Write some data to the childs input */
            if (write(in[1], str, strlen(str)) == -1)
            {
                perror("write");
                exit(EXIT_FAILURE);
            }

            /* Read back any output */
            ssize_t n = read(out[0], buf, sizeof(buf) - 1);

            if (n == -1)
            {
                perror("read");
                exit(EXIT_FAILURE);
            }
            buf[n] = 0;
            printf("%s", buf);
        }
        close(in[1]);
        close(out[0]);
    }
    return 0;
}

