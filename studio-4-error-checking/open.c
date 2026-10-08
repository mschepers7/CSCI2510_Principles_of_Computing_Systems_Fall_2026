#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#define bufferSize 1

 int main(int argc, char *argv[]) {
    char buffer[bufferSize];
    ssize_t bytesRead;
    int fd;

    fd = open(argv[1], O_RDONLY);

	if  (fd == -1) {
	perror("error opening file");
	return -1;
	}

    while (1) {
        bytesRead = read(fd, buffer, bufferSize);

        if (bytesRead == 0) {
            break;
        }

        write(STDOUT_FILENO, buffer, bytesRead);
    }

    close(fd);

    return 0;
}  
