#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    pid_t pid1, pid2;

    int fd[2];

    pipe(fd);

    
    pid1 = fork();

    if (pid1 == 0)
    {
        dup2(fd[1], STDOUT_FILENO);

	close(fd[0]);
        close(fd[1]);

        char* cmd = "./print";
        char* myargv[] = {"print", NULL};

        execvp(cmd, myargv);

        perror("Error exec'ing print");
        exit(1);
    }

    
    pid2 = fork();

    if (pid2 == 0)
    {
        dup2(fd[0], STDIN_FILENO);

        close(fd[0]);
        close(fd[1]);

        char* cmd = "./receive";
        char* myargv[] = {"receive", NULL};

        execvp(cmd, myargv);

        perror("Error exec'ing receive");
        exit(1);
    }

        close(fd[0]);
        close(fd[1]);

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    return 0;
}
