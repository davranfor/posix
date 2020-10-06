#define _XOPEN_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/shm.h>
#include <unistd.h>

static pid_t *shm;
static int sid;

static void SIGINT_handler(int sig)
{
    if (signal(sig, SIG_IGN) == SIG_ERR)
    {
        perror("signal");
        exit(EXIT_FAILURE);
    }
    printf("From SIGINT: just got a %d (SIGINT ^C) signal\n", sig);
    if (signal(sig, SIGINT_handler) == SIG_ERR)
    {
        perror("signal");
        exit(EXIT_FAILURE);
    }
}

static void SIGQUIT_handler(int sig)
{
    signal(sig, SIG_IGN);
    printf("From SIGQUIT: just got a %d (SIGQUIT ^\\) signal and is about to quit\n", sig);
    if (shmdt(shm) == -1)
    {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }
    if (shmctl(sid, IPC_RMID, NULL) == -1)
    {
        perror("shmctl");
        exit(EXIT_FAILURE);
    }
    exit(3);
}

int main(void)
{
    key_t key;

    key = ftok(".", 's');    
    if (key == -1)
    {
        perror("ftok");
        exit(EXIT_FAILURE);
    }
    sid = shmget(key, sizeof(pid_t), IPC_CREAT | 0666);
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
    *shm = getpid();
    if (signal(SIGINT, SIGINT_handler) == SIG_ERR)
    {
        perror("signal");
        exit(EXIT_FAILURE);
    }
    if (signal(SIGQUIT, SIGQUIT_handler) == SIG_ERR)
    {
        perror("signal");
        exit(EXIT_FAILURE);
    }
    puts("Waiting the sender ...");            
    while (1)
    {
        sleep(1);
    }
    return 0;
}

