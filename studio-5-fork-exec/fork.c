#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
    pid_t child_PID;

    char* child_argv[] = {"ls", "-l", NULL};
    char* child_prog = child_argv[0];

    child_PID = fork();

    if (child_PID == 0)
    {
        execvp(child_prog, child_argv);

        perror("execvp failed");
        return 1;
    }
    else if (child_PID > 0)
    {
        waitpid(child_PID, NULL, 0);

        printf("I am the parent. My PID is %d\n", getpid());
    }
    else
    {
        perror("fork failed");
        return 1;
    }

    return 0;
}
