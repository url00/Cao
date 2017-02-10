// Cao.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Cao.h"

#define MAX_LOADSTRING 100

ATOM
MyRegisterClass(HINSTANCE hInstance);

BOOL
InitInstance(HINSTANCE, int);

LRESULT CALLBACK
WndProc(HWND, UINT, WPARAM, LPARAM);

INT_PTR CALLBACK
About(HWND, UINT, WPARAM, LPARAM);



static HINSTANCE MainInstance;
static WCHAR Title[MAX_LOADSTRING];
static WCHAR MainClass[MAX_LOADSTRING];



static const char *logFileName = "log.txt";
static std::ofstream f(logFileName);



int APIENTRY
wWinMain(
    _In_     HINSTANCE Instance,
    _In_opt_ HINSTANCE PrevInstance,
    _In_     LPWSTR    cmdLine,
    _In_     int       cmdShow
)
{
    UNREFERENCED_PARAMETER(PrevInstance);
    UNREFERENCED_PARAMETER(cmdLine);
    
    MainInstance = Instance;



    // Initialize global strings.
    LoadStringW(MainInstance, IDS_APP_TITLE, Title, MAX_LOADSTRING);
    LoadStringW(MainInstance, IDC_CAO, MainClass, MAX_LOADSTRING);



    {
        WNDCLASSEXW params;
        params.cbSize = sizeof(WNDCLASSEX);
        params.style          = CS_HREDRAW | CS_VREDRAW;
        params.lpfnWndProc    = WndProc;
        params.cbClsExtra     = 0;
        params.cbWndExtra     = 0;
        params.hInstance      = MainInstance;
        params.hIcon          = LoadIcon(MainInstance, MAKEINTRESOURCE(IDI_CAO));
        params.hCursor        = LoadCursor(nullptr, IDC_ARROW);
        params.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
        params.lpszMenuName   = MAKEINTRESOURCEW(IDC_CAO);
        params.lpszClassName  = MainClass;
        params.hIconSm        = LoadIcon(params.hInstance, MAKEINTRESOURCE(IDI_SMALL));

        ATOM result = RegisterClassExW(&params);
        if (result == 0)
        {
        }
    }




    // Perform application initialization:
    if (!InitInstance (Instance, cmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(Instance, MAKEINTRESOURCE(IDC_CAO));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{

   HWND hWnd = CreateWindowW(MainClass, Title, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(MainInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
