/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Entry point and associated low level init code.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "always.h"

#ifdef GAME_DLL
extern HWND &g_applicationHWnd;
extern unsigned &g_theMessageTime;
extern bool &g_gameIsWindowed;
extern bool &g_gameNotFullscreen;
extern bool &g_gameActive;
extern bool &g_creatingWindow;
extern HGDIOBJ &g_splashImage;
extern HINSTANCE &g_applicationHInstance;

// This will eventually be replaced by a standard int main(int arc, char *argv[]) function
int __stdcall Main_Func(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);

#else
#ifdef PLATFORM_WINDOWS
extern unsigned g_theMessageTime;
extern HWND g_applicationHWnd;
extern HGDIOBJ g_splashImage;
extern HINSTANCE g_applicationHInstance;
#endif
// #ifdef BUILD_WITH_SDL2
struct SDL_Window;
extern SDL_Window *g_applicationWindow;
//#endif
extern bool g_gameIsWindowed;
extern bool g_gameNotFullscreen;
extern bool g_gameActive;
extern bool g_creatingWindow;
#endif

int main(int argc, char *argv[]);
