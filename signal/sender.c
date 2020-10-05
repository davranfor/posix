#define _XOPEN_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/shm.h>
#include <unistd.h>

int main(void)
{
    pid_t pid;
    key_t key;
    pid_t *shm;
    int sid;

    key = ftok(".", 's');
    if (key == -1)
    {
        perror("ftok");
        exit(EXIT_FAILURE);
    }
    sid = shmget(key, sizeof(pid_t), 0666);
    if (sid == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    shm = shmat(sid, NULL, 0);
    if (shm == (void *)-1)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    pid = *shm;
    if (shmdt(shm) == -1)
    {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }
    while (1)
    {
        puts("Press 'i' to send 'SIGINT' or 'q' to send 'SIGQUIT'");

        int c = fgetc(stdin);

        if ((c == 'q') || (c == 'Q') || (c == EOF))
        {
            kill(pid, SIGQUIT);
            break;
        }
        if ((c == 'i') || (c == 'I'))
        {
            kill(pid, SIGINT);
        }
        if (c != '\n')
        {
            while (((c = getc(stdin)) != '\n') && (c != EOF));
        }
    }
    return 0;
}

