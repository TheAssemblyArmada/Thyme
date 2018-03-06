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
#include "systimer.h"

#ifndef THYME_STANDALONE
// CPUDetect uses this in static init, breaks if that is initialised before this, define instantiates where used.
// SysTimeClass &g_theSysTimer = Make_Global<SysTimeClass>(0x00A66B30);
#else
SysTimeClass g_theSysTimer;
#endif

void SysTimeClass::Reset()
{
    m_startTime = Time_Func();
    m_negTime = -m_startTime;
}

int SysTimeClass::Get()
{
#ifndef THYME_STANDALONE
#define _is_init (Make_Global<bool>(0x00A66B30))
#else
    static bool _is_init;
#endif

    if (!_is_init) {
        Reset();
        _is_init = true;
    }

    int time = Time_Func();

    if (time < m_startTime) {
        return m_negTime + time;
    }

    return time - m_startTime;
}

bool SysTimeClass::Is_Getting_Late()
{
    return Get() > 0x6FFFFFFF;
}
