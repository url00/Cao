#include <stdio.h>
#include <string>
#include <Windows.h>
#include <iostream>

int main(int argc, char *argv[])
{
    //__debugbreak();

    printf("\n\n\n----------- Echoer ----------\n\n\n");

    printf("via passed arguments:\n");
    printf("argc: %d\n", argc);
    printf("argv[1..n]: ");

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

    printf("\n\n\n====================\n\n\n");

    printf("via stdin:\n");


    printf("\n\n\n---------- End of Echoer ----------\n\n\n");
}