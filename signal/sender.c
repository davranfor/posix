#define _XOPEN_SOURCE

#include <stdio.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

int main(void)
{
    pid_t pid;
    key_t MyKey;
    int ShmID;
    pid_t *ShmPTR;

    MyKey = ftok(".", 's');
    ShmID = shmget(MyKey, sizeof(pid_t), 0666);
    ShmPTR = (pid_t *)shmat(ShmID, NULL, 0);
    pid = *ShmPTR;                
    shmdt(ShmPTR);
    while (1)
    {
        puts("Press 'i' to send 'SIGINT' or 'q' to send 'SIGQUIT'");

        int c = fgetc(stdin);

        if ((c == 'q') || (c == 'Q'))
        {
            kill(pid, SIGQUIT);
            break;
        }
        if ((c == 'i') || (c == 'I'))
        {
            kill(pid, SIGINT);
        }
        while (((c = getc(stdin)) != '\n') && (c != EOF));
    }
    return 0;
}

