#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    int data = 0;
    pid_t pid = fork();

    if (pid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pid == 0)
    {
        // start of child process
        printf("I'm the child\n");
        // Do the child stuff ...
        data = 1;
    }
    else
    {
        // start of parent process
        printf("I'm the parent\n");
        // Do the parent stuff ...
        data = 2;
        // Wait for child process
        if (wait(NULL) == -1)
        {
            perror("wait");
            exit(EXIT_FAILURE);
        }
        printf("I'm again the parent, the child has finished\n");
    }
    // Code executed from both the parent and the child
    printf("The %s process data is %d\n",
            pid != 0 ? "parent" : "child", data
    );
    return 0;
}

