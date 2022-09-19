/*
gcc -std=c11 -Wpedantic -Wall -Wextra -Wconversion -Wcast-qual -o msg main.c -l mq
# Examples:

*/

#define _POSIX_C_SOURCE 200809L // getline

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

#define MAX_SIZE 128
#define KEY 1234

struct msgbuf
{
    long type;
    char text[MAX_SIZE];
};

void msg_send(const char *msg)
{
    int id = msgget(KEY, IPC_CREAT | 0666);

    if (id == -1)
    {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    struct msgbuf buf;

    buf.type = 1;
    snprintf(buf.text, sizeof buf.text, "%s", msg);
    if (msgsnd(id, &buf, sizeof buf.text, IPC_NOWAIT) == -1)
    {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }
}

void msg_recv(void)
{
    int id = msgget(KEY, IPC_CREAT | 0666);

    if (id == -1)
    {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    struct msgbuf buf;

    buf.type = 1;
    if (msgrcv(id, &buf, sizeof buf.text, buf.type, MSG_NOERROR | IPC_NOWAIT) == -1)
    {
        if (errno != ENOMSG)
        {
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        puts(buf.text);
    }
}

void msg_getline(void)
{
    char *msg = NULL;
    size_t size = 0;

    if (getline(&msg, &size, stdin) == -1)
    {
        if (errno)
        {
            perror("getline");
        }
        exit(EXIT_FAILURE);
    }
    msg[strcspn(msg, "\n")] = '\0';
    msg_send(msg);
    free(msg);
}

static void print_usage(const char *path)
{
    printf("usage: %s [srn] text\n", path);
    exit(EXIT_FAILURE);
}

static void print_version(void)
{
    printf("msg [Version 1.0]\n");
    exit(EXIT_SUCCESS);
}

static void print_help(void)
{
    printf("msg\n"
            "  -s, --send[=TEXT]\tSave text to a file\n"
            "  -r, --receive\t\tPrint text n times\n"
            "      --version\t\t\tShow the program version and exit\n"
            "      --help\t\t\tShow this text and exit\n"
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
    const char *msg = NULL;
    enum {SEND = 1, RECV};
    int action = 0;

    struct option long_options[] =
    {
        { "version", no_argument, NULL, 'v' },
        { "help", no_argument, NULL, 'h' },
        { "send", required_argument, NULL, 's' },
        { "recv", no_argument, NULL, 'r' },
        { 0, 0, 0, 0 }
    };
    int opt = 0;

    while ((opt = getopt_long(argc, argv, "-s:rn", long_options, NULL)) != -1)
    {
        switch (opt)
        {
            case 'v':
                print_version();
                break;
            case 'h':
                print_help();
                break;
            case 1:
            case 's':
                set_action(argv[0], &action, SEND);
                msg = optarg;
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
    switch (action)
    {
        case SEND:
            msg_send(msg);
            break;
        case RECV:
            msg_recv();
            break;
        default:
            msg_getline();
            break;
    }
    return 0;
}

