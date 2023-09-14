/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Gamespy stuff
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
#include "hooker.h"
#endif

#ifdef GAME_DLL
extern ARRAY_DEC(int, g_gameSpyColor, 27);
#else
extern int g_gameSpyColor[27];
#endif
