/**
 * @file
 *
 * @Author OmniBlade
 *
 * @brief Entry point and associated low level init code.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "always.h"

#ifdef GAME_DLL
extern HWND &g_applicationHWnd;
extern unsigned &g_theMessageTime;

// This will eventually be replaced by a standard int main(int arc, char *argv[]) function
int __stdcall Main_Func(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);

#else
#ifdef PLATFORM_WINDOWS
extern unsigned g_theMessageTime;
extern HWND g_applicationHWnd;
#endif
#endif

int main(int argc, char *argv[]);
