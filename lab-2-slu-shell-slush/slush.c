//SLUSH - The SLU SHELL
//
//This assignment reads user input, and executes that input as a sequence of
//programs to execute.
//
//Program execution commands have the form:
//
//prog_n [args] ( ... prog_3 [args] ( prog_2 [args] ( prog_1 [args]
//
//For example, "grep slush ( sort -r ( ls -l -a" returns a reverse sorted list
//of all files in the current directory that contain the string "slush" in
//
//See the lab writeup for more details and other requirements.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#define MAX_INPUT 256
#define MAX_ARGS 15
#define MAX_COMMANDS 16

// Structure to store a command and its arguments
struct command {
    char *argv[MAX_ARGS + 1];
    int argc;
};

// Handle Ctrl+C
void handle_sigint(int signum) {
 (void)signum;
    write(STDOUT_FILENO, "\n", 1);
}

// Parse one command into arguments
int parse_command(char *input, struct command *cmd) {

    char *token;
    char *saveptr;

    cmd->argc = 0;

    token = strtok_r(input, " \t\r\n", &saveptr);

    if (token == NULL) {
        return -1;
    }

    while (token != NULL) {

        if (cmd->argc >= MAX_ARGS) {
            return -2;
        }

        cmd->argv[cmd->argc] = token;
        cmd->argc++;

        token = strtok_r(NULL, " \t\r\n", &saveptr);
    }

    cmd->argv[cmd->argc] = NULL;

    return 0;
}

// Parse the entire input line
int parse_line(char *input, struct command commands[],
               int *command_count) {

    char *start = input;
    char *p = input;

    *command_count = 0;

    while (1) {

        if (*p == '(' || *p == '\0') {

            char delimiter = *p;

            *p = '\0';

            if (*command_count >= MAX_COMMANDS) {
                return -2;
            }

            int result = parse_command(
                start,
                &commands[*command_count]
            );

            if (result != 0) {
                return result;
            }

            (*command_count)++;

            if (delimiter == '\0') {
                break;
            }

            start = p + 1;
        }

        p++;
    }

    return 0;
}

// Execute commands connected by pipes
void execute_pipeline(struct command commands[], int count) {

    pid_t pids[MAX_COMMANDS];
    int started = 0;

    int previous_read = -1;

    for (int i = 0; i < count; i++) {

        int pipefd[2] = {-1, -1};

        // Create a pipe unless this is the last command
        if (i < count - 1) {

            if (pipe(pipefd) == -1) {
                perror("pipe");
                break;
            }
        }

        pid_t pid = fork();

        if (pid == -1) {

            perror("fork");

            if (pipefd[0] != -1) {
                close(pipefd[0]);
                close(pipefd[1]);
            }

            break;
        }

        if (pid == 0) {

            // Child process: restore normal Ctrl+C behavior
            signal(SIGINT, SIG_DFL);

            // Connect input from the previous process
            if (previous_read != -1) {

                if (dup2(previous_read, STDIN_FILENO) == -1) {
                    perror("dup2");
                    _exit(1);
                }
            }

            // Connect output to the next process
            if (pipefd[1] != -1) {

                if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
                    perror("dup2");
                    _exit(1);
                }
            }

            // Close unused file descriptors
            if (previous_read != -1) {
                close(previous_read);
            }

            if (pipefd[0] != -1) {
                close(pipefd[0]);
            }

            if (pipefd[1] != -1) {
                close(pipefd[1]);
            }

            // Commands are entered backwards,
            // so execute them in reverse order
            execvp(
                commands[count - 1 - i].argv[0],
                commands[count - 1 - i].argv
            );

            // Only reached if execvp fails
            perror(commands[count - 1 - i].argv[0]);

            _exit(127);
        }

        // Parent process
        pids[started] = pid;
        started++;

        // Close the previous pipe input
        if (previous_read != -1) {
            close(previous_read);
        }

        // Parent does not need the write end
        if (pipefd[1] != -1) {
            close(pipefd[1]);
        }

        previous_read = pipefd[0];
    }

    // Close any remaining pipe descriptor
    if (previous_read != -1) {
        close(previous_read);
    }

    // Wait for all child processes
    for (int i = 0; i < started; i++) {

        int status;

        while (waitpid(pids[i], &status, 0) == -1) {

            if (errno == EINTR) {
                continue;
            }

            perror("waitpid");
            break;
        }
    }
}

int main() {

    char input[MAX_INPUT];

    struct command commands[MAX_COMMANDS];

    int command_count;

    // Set up Ctrl+C handler
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));

    sa.sa_handler = handle_sigint;

    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    // Main shell loop
    while (1) {

        printf("slush> ");
        fflush(stdout);

        // Read user input
        errno = 0;

        if (fgets(input, sizeof(input), stdin) == NULL) {

            if (errno == EINTR) {
                clearerr(stdin);
                continue;
            }

            break;
        }

        // Check whether input exceeds the limit
        if (strchr(input, '\n') == NULL && !feof(stdin)) {

            int ch;

            while ((ch = getchar()) != '\n' && ch != EOF) {
            }

            printf("Input too long\n");

            continue;
        }

        // Remove newline
        input[strcspn(input, "\n")] = '\0';

        // Ignore empty input
        if (strlen(input) == 0) {
            continue;
        }

        // Parse commands
        int result = parse_line(input, commands, &command_count);

        if (result == -1) {
            printf("Invalid null command\n");
            continue;
        }

        if (result == -2) {
            printf("Too many commands or arguments\n");
            continue;
        }

        // Handle built-in cd command
        if (command_count == 1 &&
            strcmp(commands[0].argv[0], "cd") == 0) {

            if (commands[0].argc != 2) {
                printf("Usage: cd dir\n");
                continue;
            }

            if (chdir(commands[0].argv[1]) == -1) {
                perror("cd");
            }

            continue;
        }

        // Execute the pipeline
        execute_pipeline(commands, command_count);
    }

    printf("\n");

    return 0;
}
