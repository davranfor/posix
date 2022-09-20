/*
gcc -std=c11 -Wpedantic -Wall -Wextra -Wconversion -Wcast-qual -o msg main.c
# Examples:
./msg "Hello world!"
./msg --send "Bye bye world!"
./msg --recv
./msg -r
*/

#define _POSIX_C_SOURCE 200809L // getline

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

#define KEY 1234
#define FLAGS (IPC_CREAT | 0666)
#define MAX_SIZE 128

struct msgbuf
{
    long type;
    char text[MAX_SIZE];
};

void msg_send(const char *text, long type)
{
    int qid = msgget(KEY, FLAGS);

    if (qid == -1)
    {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    struct msgbuf msg;

    msg.type = type;
    snprintf(msg.text, sizeof msg.text, "%s", text);
    if (msgsnd(qid, &msg, sizeof msg.text, IPC_NOWAIT) == -1)
    {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }
}

void msg_recv(long type)
{
    int qid = msgget(KEY, FLAGS);

    if (qid == -1)
    {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    struct msgbuf msg;

    if (msgrcv(qid, &msg, sizeof msg.text, type, IPC_NOWAIT | MSG_NOERROR) == -1)
    {
        if (errno != ENOMSG)
        {
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        puts(msg.text);
    }
}

void msg_getline(long type)
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
        msg_send(text, type);
        free(text);
    }
}

static void print_usage(const char *path)
{
    printf("usage: %s [sr] text\n", path);
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
        "msg\n"
        "  -t  --type[=TYPE]\tType of the POSIX message (default = 1)\n"
        "                   \t  0 = first message in the queue is read\n"
        "                   \t> 0 = first message in the queue of type 'type' is read\n"
        "  -s, --send[=TEXT]\tSend a POSIX message\n"
        "  -r, --recv\t\tReceive a POSIX message\n"
        "      --version\t\tShow the program version and exit\n"
        "      --help\t\tShow this text and exit\n"
    );
    exit(EXIT_SUCCESS);
}

static void set_action(const char *path, int *action, int value)
{
    if (*action != 0)
    {
        print_usage(path);
    }
    *action = value;
}

int main(int argc, char *argv[])
{
    const char *text = NULL;
    long type = 1;

    enum {SEND = 1, RECV};
    int action = 0;

    struct option long_options[] =
    {
        { "version", no_argument, NULL, 'v' },
        { "help", no_argument, NULL, 'h' },
        { "type", required_argument, NULL, 't' },
        { "send", required_argument, NULL, 's' },
        { "recv", no_argument, NULL, 'r' },
        { 0, 0, 0, 0 }
    };
    int opt = 0;

    while ((opt = getopt_long(argc, argv, "-t:s:r", long_options, NULL)) != -1)
    {
        switch (opt)
        {
            case 'v':
                print_version();
                break;
            case 'h':
                print_help();
                break;
            case 't':
                type = strtol(optarg, NULL, 10);
                break;
            case 1:
            case 's':
                set_action(argv[0], &action, SEND);
                text = optarg;
                break;
            case 'r':
                set_action(argv[0], &action, RECV);
                break;
            case '?':
                print_usage(argv[0]);
                break;
            default:
                break;
        }
    }
    if (type < 0)
    {
        fprintf(stderr, "Error: 'type' must be positive\n");
        exit(EXIT_FAILURE);
    }
    if ((type == 0) && (action != RECV)) 
    {
        fprintf(stderr, "Error: 'send' requires a 'type' greater than 0\n");
        exit(EXIT_FAILURE);
    }
    switch (action)
    {
        case SEND:
            msg_send(text, type);
            break;
        case RECV:
            msg_recv(type);
            break;
        default:
            msg_getline(type);
            break;
    }
    return 0;
}

