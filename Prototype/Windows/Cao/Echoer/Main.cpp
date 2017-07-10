#include <string>
#include <Windows.h>
#include <iostream>
#include <fstream>

int main(int argc, char *argv[])
{
    //__debugbreak();



    printf("\n\n\n----------- Echoer ----------\n\n\n");



    printf("via passed arguments:\n");
    printf("argc: %d\n", argc);
    printf("argv[1..n]:\n");

    for (int i = 1; i < argc; i++)
    {
        char buffer[4096];
        buffer[0] = '\0';
        strcat(buffer, argv[i]);
        printf("%s\n", buffer);
    }

        
    
    printf("\n\n\n====================\n\n\n");



    printf("via stdin:\n");
    {
        const int readBuffer_size = 5000;
        char *readBuffer[readBuffer_size];
        
        {
            HANDLE standardIn = GetStdHandle(STD_INPUT_HANDLE);
            DWORD bytesRead = 0;

            bool readSuccess =
                ReadFile(
                    standardIn,
                    readBuffer,
                    readBuffer_size,
                    &bytesRead,
                    NULL);
            if (!readSuccess)
            {
                printf("Could not read from standard in!\n");
            }

            CloseHandle(standardIn);
        }
        
        printf("%s", readBuffer);
    }
    

    
    printf("\n\n\n====================\n\n\n");



    printf("via temp file:\n");
    {        
        const int readBuffer_size = 5000;
        char *readBuffer[readBuffer_size];
        
        {
            HANDLE tempFile =
                CreateFile(
                    argv[1],
                    GENERIC_READ,
                    0,
                    NULL,
                    OPEN_EXISTING, 
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);
            DWORD bytesRead = 0;

            bool readSuccess =
                ReadFile(
                    tempFile,
                    readBuffer,
                    readBuffer_size,
                    &bytesRead,
                    NULL);
            if (!readSuccess)
            {
            printf("Could not read from temp file!\n");
            }

            CloseHandle(tempFile);
        }
        
        printf("%s", readBuffer);

    }
    


    printf("\n\n\nStarting sleep.\n");    
    Sleep(1000);
    printf("Ending sleep.\n");

    

    printf("\n\n\n---------- End of Echoer ----------\n\n\n");
}