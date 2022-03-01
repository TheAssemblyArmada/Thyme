#include "always.h"

#ifdef PLATFORM_WINDOWS
#include <windows.h>
HWND g_applicationHWnd;
unsigned g_theMessageTime = 0;
bool g_gameIsWindowed;
bool g_gameNotFullscreen;
bool g_creatingWindow;
HGDIOBJ g_splashImage;
HINSTANCE g_applicationHInstance;
#endif
