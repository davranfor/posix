/*
gcc -std=c11 -Wpedantic -Wall -Wextra -Wconversion -Wcast-qual -o msg main.c
# Examples:
./msg "Hello world!"
./msg --name="/myqueue" --send "Bye bye world!"
./msg --recv
./msg -r
*/

#define _POSIX_C_SOURCE 200809L // getline

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <mqueue.h>
#include <errno.h>

#define MAX_SIZE 128

static mqd_t msg_open(const char *name, int flags)
{
    struct mq_attr attr;

    // Specify message queue attributes
    // mq_maxmsg must be less or equal than /proc/sys/fs/mqueue/msg_max
    attr.mq_flags = flags;      // 0 = blocking read/write
    attr.mq_maxmsg = 10;        // maximum number of messages allowed in queue
    attr.mq_msgsize = MAX_SIZE; // messages are contents of string
    attr.mq_curmsgs = 0;        // number of messages currently in queue

    // attr.mq_flags is not consulted in mq_open, we need to pass the flags
    mqd_t mq = mq_open(name, O_RDWR | O_CREAT | flags, S_IRUSR | S_IWUSR, &attr);

    if (mq == -1)
    {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }
    return mq;
}

static void msg_close(mqd_t mq)
{
    if (mq_close(mq) == -1)
    {
        perror("mq_close");
        exit(EXIT_FAILURE);
    }
}

static void msg_send(const char *name, const char *text, unsigned prio)
{
    mqd_t mq = msg_open(name, 0);
    size_t len = strlen(text);

    if (len >= MAX_SIZE)
    {
        len = MAX_SIZE - 1;
    }
    if (mq_send(mq, text, len, prio) == -1)
    {
        perror("mq_send");
        exit(EXIT_FAILURE);
    }
    msg_close(mq);
}

static void msg_recv(const char *name, int flags)
{
    mqd_t mq = msg_open(name, flags);
    char text[MAX_SIZE] = {0};
    ssize_t len = mq_receive(mq, text, MAX_SIZE, NULL);

    if (len == -1)
    {
        // EAGAIN: O_NONBLOCK was set in mq_open and the message queue is empty
        int ok = (flags == O_NONBLOCK) && (errno == EAGAIN);

        if (!ok)
        {
            perror("mq_receive");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        text[len] = '\0';
        printf("message: %s\n", text);
    }
    msg_close(mq);
}

static void msg_getline(const char *name, unsigned prio)
{
    char *text = NULL;
    size_t size = 0;

    if (getline(&text, &size, stdin) == -1)
    {
        if (errno)
        {
            perror("getline");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        text[strcspn(text, "\n")] = '\0';
        msg_send(name, text, prio);
        free(text);
    }
}

static void msg_unlink(const char *name)
{
    // ENOENT: The named message queue does not exist
    if ((mq_unlink(name) == -1) && (errno != ENOENT))
    {
        perror("mq_unlink");
        exit(EXIT_FAILURE);
    }
}

static void print_usage(const char *path, const char *opts)
{
    fprintf(stderr, "usage: %s [%s] [TEXT]\n", path, opts);
    exit(EXIT_FAILURE);
}

static void print_version(void)
{
    printf("msg [Version 1.0]\n");
    exit(EXIT_SUCCESS);
}

static void print_help(void)
{
    printf(
        "msg: send messages to, and receive messages from, a POSIX message queue.\n\n"
        "  -n  --name=NAME    \tName of the message queue (default = /myqueue)\n"
        "  -p, --prio=PRIORITY\tPriority between 0 and 9 (default = 0)\n"
        "  -s, --send=TEXT    \tSend a message\n"
        "  -r, --recv         \tReceive a message\n"
        "  -w, --wait         \tReceive a message in blocking mode\n"
        "  -u, --unlink       \tUnlink/Delete a message queue\n"
        "      --version      \tShow the program version and exit\n"
        "      --help         \tShow this text and exit\n"
    );
    exit(EXIT_SUCCESS);
}

static void set_action(int *action, int value)
{
    if (*action != 0)
    {
        fprintf(stderr, "msg: An action is already set\n");
        exit(EXIT_FAILURE);
    }
    *action = value;
}

int main(int argc, char *argv[])
{
    const char *opts = "-n:p:s:rwu";
    const char *name = "/myqueue";
    const char *text = NULL;
    unsigned prio = 0;

    enum {SEND = 1, RECV, WAIT, UNLINK};
    int action = 0;

    struct option long_options[] =
    {
        { "version", no_argument, NULL, 'v' },
        { "help", no_argument, NULL, 'h' },
        { "name", required_argument, NULL, 'k' },
        { "prio", required_argument, NULL, 'p' },
        { "send", required_argument, NULL, 's' },
        { "recv", no_argument, NULL, 'r' },
        { "wait", no_argument, NULL, 'w' },
        { "unlink", no_argument, NULL, 'u' },
        { 0, 0, 0, 0 }
    };
    int opt = 0;

    while ((opt = getopt_long(argc, argv, opts, long_options, NULL)) != -1)
    {
        switch (opt)
        {
            case 'v':
                print_version();
                break;
            case 'h':
                print_help();
                break;
            case 'n':
                name = optarg;
                if (name[0] != '/')
                {
                    fprintf(stderr, "msg: 'name' must start with /\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'p':
                prio = (unsigned)atoi(optarg);
                if (prio > 9)
                {
                    fprintf(stderr, "msg: 'prio' must be between 0 and 9\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 1:
            case 's':
                set_action(&action, SEND);
                text = optarg;
                break;
            case 'r':
                set_action(&action, RECV);
                break;
            case 'w':
                set_action(&action, WAIT);
                break;
            case 'u':
                set_action(&action, UNLINK);
                break;
            case '?':
                print_usage(argv[0], opts);
                break;
            default:
                break;
        }
    }
    switch (action)
    {
        case SEND:
            msg_send(name, text, prio);
            break;
        case RECV:
            msg_recv(name, O_NONBLOCK);
            break;
        case WAIT:
            msg_recv(name, 0);
            break;
        case UNLINK:
            msg_unlink(name);
            break;
        default:
            msg_getline(name, prio);
            break;
    }
    return 0;
}

