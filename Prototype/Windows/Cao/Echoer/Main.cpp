#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
    printf("\n\n---------- Echoer\n");

    printf("argc: %d\n", argc);
    printf("text:\n");

    printf("some change\n");

    //__debugbreak();

    for (int i = 1; i < argc; i++)
    {
        char buffer[4096];
        buffer[0] = '\0';
        strcat(buffer, argv[i]);
        if (i + 1 < argc)
        {
            strcat(buffer, " ");
        }
        printf(buffer);
    }
}