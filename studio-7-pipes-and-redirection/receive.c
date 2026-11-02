#include <stdio.h>

int main()
{
    char input[256];

    while (1)
    {
        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            break;
        }

        printf("Program 2 got: %s\n", input);
    }

    return 0;
}
