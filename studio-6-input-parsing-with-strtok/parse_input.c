
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main() {
    int max_args = 15;
    int max_argv_size = max_args + 2;

    char input_string[256];
    char *cmd;
    char *my_argv[max_argv_size];

    printf("Enter your input: ");
     
	if (fgets(input_string, sizeof(input_string), stdin) == NULL) {
        return 1;
    }

     
    input_string[strcspn(input_string, "\n")] = '\0';

    
    cmd = strtok(input_string, " ");

    if (cmd == NULL) {
        return 1;
    }

    
    int i = 0;
    my_argv[i] = cmd;
    i++;

    
    char *res = strtok(NULL, " ");

    while (res != NULL && i < max_argv_size - 1) {
        my_argv[i] = res;
        i++;

        res = strtok(NULL, " ");
    }

    
    my_argv[i] = NULL;

    execvp(cmd, my_argv);

    perror("execvp failed");

    return 1;
}

