/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Entrypoint for the GUI toolkit.
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
#else
#ifdef PLATFORM_WINDOWS
extern HWND g_applicationHWnd;
#endif
#endif
