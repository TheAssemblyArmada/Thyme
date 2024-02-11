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
#include "rtsutilsw3d.h"

#ifndef GAME_DLL
SysTimeClass g_theSysTimer;
#endif

void SysTimeClass::Reset()
{
    m_startTime = rts::Get_Time();
    m_negTime = -m_startTime;
}

int SysTimeClass::Get()
{
#ifdef GAME_DLL
#define _is_init (Make_Global<bool>(0x00A66B30))
#else
    static bool _is_init;
#endif

    if (!_is_init) {
        Reset();
        _is_init = true;
    }

    unsigned int time = rts::Get_Time();

    if (time > m_startTime) {
        return time - m_startTime;
    }

    return time + m_negTime;
}

bool SysTimeClass::Is_Getting_Late()
{
    return Get() > 0x6FFFFFFF;
}
