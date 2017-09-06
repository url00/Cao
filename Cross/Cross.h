#pragma once

const char Config_CONTROL = 1 << 0;
const char Config_ALT     = 1 << 1;
const char Config_SHIFT   = 1 << 2;

const char Config_STANDARD = 1 << 0;
const char Config_TEMPFILE = 1 << 1;
const char Config_ARGS     = 1 << 2;



#ifdef Windows
const char *Log_Filename = "log.txt";
#elif Linux
const char *Log_Filename = "log";
#endif

#define Log_Write(message)\
{\
    __Log_Write(message, __FILE__, __LINE__);\
}\

void __Log_Write(char *message, char *file, int line);

void Log_Close();



typedef struct
{
    int screenWidth;
    int screenHeight;
    bool shouldDrawTest;
} MainWindow_CrossStateDef;

void MainWindow_Show();

void MainWindow_Hide();

void MainWindow_DisplayMessage(char *message);

void MainWindow_DrawTest(MainWindow_CrossStateDef state);

void MainWindow_DrawTestRect(int x, int y, int width, int height);



void MainWindow_DrawTest(MainWindow_CrossStateDef state)
{
    const int size = 100;
    MainWindow_DrawTestRect(0, 0, size, size);
    MainWindow_DrawTestRect(state.screenWidth - size, 0, size, size);
    MainWindow_DrawTestRect(0, state.screenHeight - size, size, size);
    MainWindow_DrawTestRect(state.screenWidth - size, state.screenHeight - size, size, size);
}
