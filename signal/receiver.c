#define _XOPEN_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

pid_t *ShmPTR;
int ShmID;

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
    shmdt(ShmPTR);
    shmctl(ShmID, IPC_RMID, NULL);
    exit(3);
}

int main(void)
{
    pid_t pid = getpid();
    key_t MyKey;

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
    MyKey = ftok(".", 's');    
    ShmID = shmget(MyKey, sizeof(pid_t), IPC_CREAT | 0666);
    ShmPTR = (pid_t *) shmat(ShmID, NULL, 0);
    *ShmPTR = pid;
    puts("Waiting the sender ...");            
    while (1)
    {
        sleep(1);
    }
    return 0;
}

