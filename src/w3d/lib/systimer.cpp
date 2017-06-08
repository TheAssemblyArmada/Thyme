////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: SYSTIMER.CPP
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Tracks the system time.
//
//       License:: Thyme is free software: you can redistribute it and/or 
//                 modify it under the terms of the GNU General Public License 
//                 as published by the Free Software Foundation, either version 
//                 2 of the License, or (at your option) any later version.
//
//                 A full copy of the GNU General Public License can be found in
//                 LICENSE
//
////////////////////////////////////////////////////////////////////////////////
#include "always.h"
#include "systimer.h"

//SysTimeClass g_theSysTimer;

void SysTimeClass::Reset()
{
    m_startTime = Time_Func();
    m_negTime = -m_startTime;
}

int SysTimeClass::Get()
{
    //static bool _is_init;
#define _is_init (Make_Global<bool>(0x00A66B30))

    if ( !_is_init ) {
        Reset();
        _is_init = true;
    }

    int time = Time_Func();

    if ( time < m_startTime ) {
        return m_negTime + time;
    }

    return time - m_startTime;
}

bool SysTimeClass::Is_Getting_Late()
{
    return Get() > 0x6FFFFFFF;
}
