/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Shell
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "shell.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

#ifndef GAME_DLL
Shell *g_theShell;
#endif

void Shell::Push(Utf8String filename, bool b)
{
#ifdef GAME_DLL
    Call_Method<void, Shell, Utf8String, bool>(PICK_ADDRESS(0x005C50C0, 0x009E01DB), this, filename, b);
#endif
}

WindowLayout *Shell::Top()
{
#ifdef GAME_DLL
    return Call_Method<WindowLayout *, Shell>(PICK_ADDRESS(0x005C5530, 0x009E09B3), this);
#else
    return nullptr;
#endif
}

void Shell::Show_Shell(int i)
{
#ifdef GAME_DLL
    Call_Method<void, Shell, int>(PICK_ADDRESS(0x005C5300, 0x009E0558), this, i);
#endif
}

void Shell::Hide_Shell()
{
#ifdef GAME_DLL
    Call_Method<void, Shell>(PICK_ADDRESS(0x005C54E0, 0x009E08B7), this);
#endif
}
