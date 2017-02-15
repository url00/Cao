#include "stdafx.h"
#include "Cao.h"
#include "..\..\..\Cross\Cross.h"

#define MAX_LOADSTRING 100



static HINSTANCE _mainInstance;
static WCHAR _title[MAX_LOADSTRING];
static WCHAR _mainClass[MAX_LOADSTRING];



static HWND _mainWindow;
static const int _mainWindow_HEIGHT = 100;
static const int _mainWindow_WIDTH  = 400;



static HWND _editWindow;



static int _font_height = 0;
static const int _font_PADDING = 6;



int APIENTRY
wWinMain(
    HINSTANCE instance,
    HINSTANCE prevInstance,
    LPWSTR    cmdLine,
    int       cmdShow
)
{
    UNREFERENCED_PARAMETER(prevInstance);
    UNREFERENCED_PARAMETER(cmdLine);
    
    _mainInstance = instance;



    LoadStringW(_mainInstance, IDS_APP_TITLE, _title,     MAX_LOADSTRING);
    LoadStringW(_mainInstance, IDC_CAO,       _mainClass, MAX_LOADSTRING);



    {
        WNDCLASSEXW params;
        params.cbSize        = sizeof(WNDCLASSEX);
        params.style         = CS_HREDRAW | CS_VREDRAW;
        params.lpfnWndProc   = WndProc;
        params.cbClsExtra    = 0;
        params.cbWndExtra    = 0;
        params.hInstance     = _mainInstance;
        params.hIcon         = LoadIcon(_mainInstance, MAKEINTRESOURCE(IDI_CAO));
        params.hCursor       = LoadCursor(NULL, IDC_ARROW);
        params.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
        params.lpszMenuName  = MAKEINTRESOURCEW(IDC_CAO);
        params.lpszClassName = _mainClass;
        params.hIconSm       = LoadIcon(params.hInstance, MAKEINTRESOURCE(IDI_SMALL));

        ATOM result = RegisterClassExW(&params);
        if (result == 0)
        {
            goto error_cleanup_logFile;
        }
    }


       
    _mainWindow =
        CreateWindowW(
            _mainClass,
            _title,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT,
            _mainWindow_WIDTH, _mainWindow_HEIGHT,
            NULL,
            NULL,
            _mainInstance,
            NULL);
    if (!_mainWindow)
    {
        goto error_cleanup_logFile;
    }

    ShowWindow(_mainWindow, cmdShow);
    UpdateWindow(_mainWindow);


    
    _editWindow =
        CreateWindowExW(
            WS_EX_CLIENTEDGE,
            L"EDIT",
            _title,
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT,
            0, 0,
            _mainWindow_WIDTH, _mainWindow_HEIGHT,
            _mainWindow,
            NULL,
            _mainInstance,
            NULL);    

    {
        HDC context = GetDC(_editWindow);
        if (context == NULL)
        {
            Log("Could not get DC for font height.");

            ReleaseDC(_editWindow, context);
            goto error_cleanup_logFile;
        }


        TEXTMETRIC metrics;
        {
            BOOL success = GetTextMetrics(context, &metrics);
            if (!success)
            {
                Log("Could not get metrics for font height.");

                ReleaseDC(_editWindow, context);
                goto error_cleanup_logFile;
            }
        }

        _font_height = metrics.tmHeight;


        
        RECT clientRect;
        GetClientRect(_mainWindow, &clientRect);
        _editWindow_UpdateSize(clientRect.right);
    }




    HACCEL accelTable = LoadAccelerators(_mainInstance, MAKEINTRESOURCE(IDC_CAO));



    MSG message;
    while (GetMessage(&message, NULL, 0, 0))
    {
        if (!TranslateAccelerator(message.hwnd, accelTable, &message))
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
    }


    CloseLog();

    return (int) message.wParam;



error_cleanup_logFile:
    CloseLog();



    return EXIT_FAILURE;
}



LRESULT CALLBACK
WndProc(
    HWND window,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (message)
    {
        case WM_CREATE:
        {
            break;
        }

        case WM_COMMAND:
        {
            break;
        }

        case WM_PAINT:
        {
            PAINTSTRUCT paint;
            HDC context = BeginPaint(window, &paint);
            EndPaint(window, &paint);
            break;
        }

        case WM_DESTROY:
        {
            PostQuitMessage(0);
            break;
        }

        case WM_SIZE:
        {
            
            int width = LOWORD(lParam);
            _editWindow_UpdateSize(width);

            break;
        }
    }

    return DefWindowProc(window, message, wParam, lParam);
}



void
_editWindow_UpdateSize(int width)
{
    BOOL success =
        MoveWindow(
            _editWindow,
            0, 0,
            width, _font_height + _font_PADDING,
            true);
}