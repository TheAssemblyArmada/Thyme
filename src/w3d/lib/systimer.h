////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: SYSTIMER.H
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
#pragma once

#ifndef SYSTIMER_H
#define SYSTIMER_H

#include "hooker.h"

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
    static int Time_Func();

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

inline int SysTimeClass::Time_Func()
{
#ifdef PLATFORM_WINDOWS
    return timeGetTime();
#else
    struct timeval now;
    gettimeofday(&now, NULL);
    return now.tv_usec / 1000;
#endif
}

#define g_theSysTimer (MakeGlobal<SysTimeClass>(0x00A66B30))
//extern SysTimeClass g_theSysTimer;

#endif // SYSTIMER_H