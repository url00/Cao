#pragma once

#include "resource.h"

LRESULT CALLBACK
WndProc(
    HWND,
    UINT,
    WPARAM,
    LPARAM);

void
_editWindow_UpdateSize(int);