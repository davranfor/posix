/*
gcc -std=c11 -Wpedantic -Wall -Wextra -Wconversion -Wcast-qual -o msg main.c
# Examples:
./msg -k 1234 "Hello world!"
./msg --key=1234 --send "Bye bye world!"
./msg --key=1234 --recv
./msg --key=1234 -r
*/

#define _POSIX_C_SOURCE 200809L // getline

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

#define FLAGS (IPC_CREAT | 0666)
#define MAX_SIZE 128

struct msgbuf
{
    long type;
    char text[MAX_SIZE];
};

void msg_send(key_t key, long type, const char *text)
{
    int qid = msgget(key, FLAGS);

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

void msg_recv(key_t key, long type)
{
    int qid = msgget(key, FLAGS);

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

void msg_getline(key_t key, long type)
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
        msg_send(key, type, text);
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
        "msg: send messages to, and receive messages from, a System V message queue.\n"
        "  -k  --key[=KEY]  \tKey of the message (default = 0)\n"
        "  -t  --type[=TYPE]\tType of the message (default = 1)\n"
        "                   \t  0 = first message in the queue is read\n"
        "                   \t> 0 = first message in the queue of type 'type' is read\n"
        "  -s, --send[=TEXT]\tSend a message\n"
        "  -r, --recv\t\tReceive a message\n"
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
    key_t key = 0;
    long type = 1;
    const char *text = NULL;

    enum {SEND = 1, RECV};
    int action = 0;

    struct option long_options[] =
    {
        { "version", no_argument, NULL, 'v' },
        { "help", no_argument, NULL, 'h' },
        { "key", required_argument, NULL, 'k' },
        { "type", required_argument, NULL, 't' },
        { "send", required_argument, NULL, 's' },
        { "recv", no_argument, NULL, 'r' },
        { 0, 0, 0, 0 }
    };
    int opt = 0;

    while ((opt = getopt_long(argc, argv, "-k:t:s:r", long_options, NULL)) != -1)
    {
        switch (opt)
        {
            case 'v':
                print_version();
                break;
            case 'h':
                print_help();
                break;
            case 'k':
                key = (key_t)strtol(optarg, NULL, 10);
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
            msg_send(key, type, text);
            break;
        case RECV:
            msg_recv(key, type);
            break;
        default:
            msg_getline(key, type);
            break;
    }
    return 0;
}

