//This program brute-forces a given password hash by trying all possible
//passwords of a given length.
//
//Usage:
//crack <threads> <keysize> <target>
//
//Where <threads> is the number of threads to use, <keysize> is the maximum
//password length to search, and <target> is the target password hash.
//
//For example:
//
//./crack 1 5 na3C5487Wz4zw
//
//Should return the password 'apple'


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <crypt.h>
#include <stdint.h>
#include <stdatomic.h>

#define MAX_KEYSIZE 8

// Shared variables
char *target;
char salt[3];
char cracked_password[MAX_KEYSIZE + 1] = "";

atomic_int found = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// Thread information
struct thread_data {
    int thread_id;
    int total_threads;
    int keysize;
};

// Generate a password from a number
void generate_password(uint64_t number, int length, char *password) {

    password[length] = '\0';

    for (int i = length - 1; i >= 0; i--) {
        password[i] = 'a' + (number % 26);
        number /= 26;
    }
}

// Calculate 26^length
uint64_t power(int length) {

    uint64_t result = 1;

    for (int i = 0; i < length; i++) {
        result *= 26;
    }

    return result;
}

// Function executed by each thread
void *crack_password(void *arg) {

    struct thread_data *data = (struct thread_data *)arg;

    struct crypt_data crypt_info = {0};

    char password[MAX_KEYSIZE + 1];

    for (int length = 1; length <= data->keysize; length++) {

        uint64_t total = power(length);

        for (uint64_t i = data->thread_id;
             i < total;
             i += data->total_threads) {

            if (atomic_load(&found)) {
                return NULL;
            }

            generate_password(i, length, password);

            char *result = crypt_r(password, salt, &crypt_info);

            if (result != NULL && strcmp(result, target) == 0) {

                pthread_mutex_lock(&lock);

                if (!atomic_load(&found)) {
                    strcpy(cracked_password, password);
                    atomic_store(&found, 1);
                }

                pthread_mutex_unlock(&lock);

                return NULL;
            }
        }
    }

    return NULL;
}

int main(int argc, char *argv[]) {

    if (argc != 4) {
        printf("Usage: crack <threads> <keysize> <target>\n");
        return 1;
    }

    int threads = atoi(argv[1]);
    int keysize = atoi(argv[2]);

    target = argv[3];

    if (threads <= 0 || keysize < 1 || keysize > MAX_KEYSIZE ||
        strlen(target) < 2) {
        return 1;
    }

    
    salt[0] = target[0];
    salt[1] = target[1];
    salt[2] = '\0';

    pthread_t *thread_ids = malloc(threads * sizeof(pthread_t));

    struct thread_data *thread_args =
        malloc(threads * sizeof(struct thread_data));

    if (thread_ids == NULL || thread_args == NULL) {
        free(thread_ids);
        free(thread_args);
        return 1;
    }

    
    for (int i = 0; i < threads; i++) {

        thread_args[i].thread_id = i;
        thread_args[i].total_threads = threads;
        thread_args[i].keysize = keysize;

        if (pthread_create(&thread_ids[i], NULL,
                           crack_password, &thread_args[i]) != 0) {
            for (int j = 0; j < i; j++) {
                pthread_join(thread_ids[j], NULL);
            }

            free(thread_ids);
            free(thread_args);
            return 1;
        }
    }

    
    for (int i = 0; i < threads; i++) {
        pthread_join(thread_ids[i], NULL);
    }

    
    if (atomic_load(&found)) {
        printf("%s\n", cracked_password);
    }

    free(thread_ids);
    free(thread_args);

    return 0;
}
