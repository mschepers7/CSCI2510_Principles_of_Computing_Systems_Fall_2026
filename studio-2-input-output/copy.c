//Replace this file with your copy.c

#include <stdio.h>
#include <unistd.h>

#define bufferSize 1

int main() {
    char buffer[bufferSize];
    ssize_t bytesRead;

    while (1) {
        bytesRead = read(STDIN_FILENO, buffer, bufferSize);

        if (bytesRead == 0) {
            break;
        }

        write(STDOUT_FILENO, buffer, bytesRead);
    }

    return 0;
}

