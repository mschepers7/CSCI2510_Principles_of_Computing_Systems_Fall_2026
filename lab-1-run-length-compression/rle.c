//Runlength compression in C
// Usage: ./rle <input file> <output file> <runlength> <mode>
//        Where mode=0 is compress and mode=1 is decompress

#include <stdio.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

int write_all(int fd, const unsigned char *buffer, size_t length)
{
    size_t written = 0;

    while (written < length)
    {
        ssize_t n = write(fd, buffer + written, length - written);

        if (n == -1)
        {
            if (errno == EINTR)
                continue;

            perror("write");
            return -1;
        }

        if (n == 0)
        {
            fprintf(stderr, "write returned zero\n");
            return -1;
        }

        written += n;
    }

    return 0;
}

int read_file(int fd, unsigned char **data, size_t *length)
{
    size_t capacity = 4096;
    size_t used = 0;

    *data = malloc(capacity);

    if (*data == NULL)
    {
        perror("malloc");
        return -1;
    }

    while (1)
    {
        if (used == capacity)
        {
            if (capacity > (size_t)-1 / 2)
            {
                fprintf(stderr, "File is too large\n");
                free(*data);
                *data = NULL;
                return -1;
            }

            size_t new_capacity = capacity * 2;
            unsigned char *temp = realloc(*data, new_capacity);

            if (temp == NULL)
            {
                perror("realloc");
                free(*data);
                *data = NULL;
                return -1;
            }

            *data = temp;
            capacity = new_capacity;
        }

        ssize_t n = read(fd, *data + used, capacity - used);

        if (n == -1)
        {
            if (errno == EINTR)
                continue;

            perror("read");
            free(*data);
            *data = NULL;
            return -1;
        }

        if (n == 0)
            break;

        used += (size_t)n;
    }

    *length = used;
    return 0;
}

int compress_file(int fd, const unsigned char *data,
                  size_t length, size_t k)
{
    size_t pos = 0;

    while (pos < length)
    {
        size_t pattern_length = length - pos;

        if (pattern_length > k)
            pattern_length = k;

        const unsigned char *pattern = data + pos;
        unsigned int count = 1;

        pos += pattern_length;

        while (pos < length && count < 255)
        {
            size_t next_length = length - pos;

            if (next_length > k)
                next_length = k;

            if (next_length != pattern_length)
                break;

            if (memcmp(data + pos, pattern, pattern_length) != 0)
                break;

            count++;
            pos += next_length;
        }

        unsigned char counter = (unsigned char)count;

        if (write_all(fd, &counter, 1) == -1)
            return -1;

        if (write_all(fd, pattern, pattern_length) == -1)
            return -1;
    }

    return 0;
}

int decompress_file(int fd, const unsigned char *data,
                    size_t length, size_t k)
{
    size_t pos = 0;

    while (pos < length)
    {
        unsigned int count = data[pos++];

        if (count == 0)
        {
            fprintf(stderr, "Invalid compressed file: zero counter\n");
            return -1;
        }

        if (length - pos < k)
        {
            fprintf(stderr, "Invalid compressed file: incomplete pattern\n");
            return -1;
        }

        for (unsigned int i = 0; i < count; i++)
        {
            if (write_all(fd, data + pos, k) == -1)
                return -1;
        }

        pos += k;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        fprintf(stderr,
                "Usage: %s <input file> <output file> "
                "<compression length> <mode>\n",
                argv[0]);
        exit(-1);
    }

    char *end;
    errno = 0;

    long k_value = strtol(argv[3], &end, 10);

    if (errno == ERANGE || end == argv[3] ||
        *end != '\0' || k_value < 1)
    {
        fprintf(stderr,
                "Compression length must be a positive integer\n");
        exit(-1);
    }

    errno = 0;
    long mode = strtol(argv[4], &end, 10);

    if (errno == ERANGE || end == argv[4] ||
        *end != '\0' || (mode != 0 && mode != 1))
    {
        fprintf(stderr, "Mode must be 0 or 1\n");
        exit(-1);
    }

    int input_fd = open(argv[1], O_RDONLY);

    if (input_fd == -1)
    {
        perror("open input");
        exit(-1);
    }

    unsigned char *data = NULL;
    size_t length = 0;

    if (read_file(input_fd, &data, &length) == -1)
    {
        if (close(input_fd) == -1)
            perror("close input");

        exit(-1);
    }

    if (close(input_fd) == -1)
    {
        perror("close input");
        free(data);
        exit(-1);
    }

    int output_fd = open(argv[2],
                         O_WRONLY | O_CREAT | O_TRUNC,
                         S_IRUSR | S_IWUSR);

    if (output_fd == -1)
    {
        perror("open output");
        free(data);
        exit(-1);
    }

    int status;

    if (mode == 0)
        status = compress_file(output_fd, data, length,
                               (size_t)k_value);
    else
        status = decompress_file(output_fd, data, length,
                                 (size_t)k_value);

    free(data);

    if (close(output_fd) == -1)
    {
        perror("close output");
        exit(-1);
    }

    if (status == -1)
        exit(-1);

    return 0;
}
