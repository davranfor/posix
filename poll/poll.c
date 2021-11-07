#include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <unistd.h>

int getkey()
{
    int key = 0;

    for (;;)
    {
        char c = 0;
        ssize_t size = read(STDIN_FILENO, &c, 1);

        if (size == -1)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }
        if ((size == 0) || (c == '\n'))
        {
            break;
        }
        else if (key == 0)
        {
            key = c;
        }
    }
    return key;
}

int main(void)
{
    printf("I'm going to sleep for 5 seconds | Press any key and then enter\n");
    sleep(5);

    struct pollfd pfd =
    {
        .fd = STDIN_FILENO,
        .events = POLLIN
    };

    /**
     * poll()
     * Waits for one of a set of file descriptors to become ready to perform I/O.
     * --------------------------------------------------------------------------
     * Arguments:
     *      1) Pointer to pollfd
     *      2) Number of pollfds
     *      3) Timeout in milliseconds
     * --------------------------------------------------------------------------
     * Returns:
     *      -1 on error
     *      0 on timeout
     *      Another value if "ready"
     */
    int ready = poll(&pfd, 1, 0);

    if (ready == -1)
    {
        perror("poll");
        exit(EXIT_FAILURE);
    }
    if (ready && (pfd.revents & POLLIN))
    {
        printf("'%c' was pressed while sleeping\n", getkey());
    }
    else
    {
        puts("\nPress enter");
        getchar();
    }
    return 0;
}

