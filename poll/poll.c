#include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <unistd.h>

int getkey()
{
    char buf[128];
    int key = 0;

    for (;;) // Loop until stdin is totally read
    {
        ssize_t size = read(STDIN_FILENO, buf, sizeof buf);

        if (size == -1)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }
        if (size > 0)
        {
            if (key == 0)
            {
                key = buf[0];
            }
            // Is there data pending to read in the buffer?
            if ((size == sizeof buf) && (buf[size - 1] != '\n'))
            {
                continue; // Consume the rest of the buffer
            }
        }
        break;
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

