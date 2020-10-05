#define _XOPEN_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/shm.h>
#include <unistd.h>

pid_t *shm;
int sid;

static void SIGINT_handler(int sig)
{
    signal(sig, SIG_IGN);
    printf("From SIGINT: just got a %d (SIGINT ^C) signal\n", sig);
    signal(sig, SIGINT_handler);
}

static void SIGQUIT_handler(int sig)
{
    signal(sig, SIG_IGN);
    printf("From SIGQUIT: just got a %d (SIGQUIT ^\\) signal and is about to quit\n", sig);
    shmdt(shm);
    shmctl(sid, IPC_RMID, NULL);
    exit(3);
}

int main(void)
{
    pid_t pid = getpid();
    key_t key;

    if (signal(SIGINT, SIGINT_handler) == SIG_ERR)
    {
        printf("SIGINT install error\n");
        exit(1);
    }
    if (signal(SIGQUIT, SIGQUIT_handler) == SIG_ERR)
    {
        printf("SIGQUIT install error\n");
        exit(2);
    }
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
    *shm = pid;
    puts("Waiting the sender ...");            
    while (1)
    {
        sleep(1);
    }
    return 0;
}

