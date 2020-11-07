#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int data = 0;
    pid_t pid;

    if ((pid = fork()) == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        /* start of child process */
        printf("I'm the child\n");

        // Do the child stuff ...
        data = 1;
    }
    else
    {
        /* start of parent process */
        printf("I'm the parent\n");

        // Do the parent stuff ...
        data = 2;

        int status;

        /* Wait for child process */
        if ((pid = wait(&status)) == -1)
        {
            perror("wait");
            exit(EXIT_FAILURE);
        }
        else
        {
            printf("I'm again the parent\n");
        }
    }
    // Code executed from both the parent and the child
    printf("Press intro ");
    getchar();
    printf("The %s filled data with %d\n", pid != 0 ? "parent" : "child", data);
    printf("Bye\n");
    return 0;
}

