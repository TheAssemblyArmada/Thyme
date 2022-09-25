/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Tracks system time.
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

#ifdef PLATFORM_WINDOWS
#include <mmsystem.h>
#else
#include <sys/time.h>
#endif

class SysTimeClass
{
public:
    SysTimeClass();
    ~SysTimeClass();

    int Get();
    bool Is_Getting_Late();

private:
    void Reset();
    static unsigned Time_Func();

    int m_startTime;
    int m_negTime;
};

inline SysTimeClass::SysTimeClass()
{
#ifdef PLATFORM_WINDOWS
    timeBeginPeriod(1);
#endif
}

inline SysTimeClass::~SysTimeClass()
{
#ifdef PLATFORM_WINDOWS
    timeEndPeriod(1);
#endif
}

#ifdef GAME_DLL
extern SysTimeClass &g_theSysTimer;
#else
extern SysTimeClass g_theSysTimer;
#endif