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
#include "editmain.h"

#if !defined GAME_DLL && defined PLATFORM_WINDOWS
HWND g_applicationHWnd;
#endif

int main(int argc, char **argv)
{
    return 0; // TODO allows linking as none functional standalone.
}
