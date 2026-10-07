#include <stdio.h>
#include <stdlib.h>

char* reverseString(char* input) {
    int length = 0;

    // Count the characters
    while (input[length] != '\0') {
        length++;
    }

    // Allocate space for the reversed string
    char* output = (char*)malloc(length + 1);

    // Copy characters in reverse order
    for (int i = 0; i < length; i++) {
        output[i] = input[length - 1 - i];
    }

    // Add the null terminator
    output[length] = '\0';

    return output;
}

/*
void printReverse(char* string) {
    int length = 0;

    while (string[length] != '\0') {
        length++;
    }

    for (int i = length - 1; i >= 0; i--) {
        printf("%c\n", string[i]);
    }
} */


int main() {

	char *messagePtr = "HELLOWORLD!";

	char* reversedMessage = reverseString(messagePtr);
	printf("Reversed string: %s\n", reversedMessage);
//	printReverse(messagePtr);


/*	printf("%s\n", messagePtr);

	for (int i = 0; i < 11; i++) {
    		printf("%c\n", messagePtr[i]);
	} */

 /*  for (int i = 0; i < 11; i++) {
    printf("%c\n", *(messagePtr + i));
}*/

/* int i = 0;

while (messagePtr[i] != '\0') {
    printf("%c\n", messagePtr[i]);
    i++;
}*/





    return 0;
}

