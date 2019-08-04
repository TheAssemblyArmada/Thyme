/**
 * @file
 *
 * @Author OmniBlade
 *
 * @brief Base class to wrap threading API.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#ifdef GAME_DLL
#include "hooker.h"
#endif

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
typedef void _EXCEPTION_POINTERS; // TODO set this to something appropriate, void for now so it compiles
#elif defined PLATFORM_WINDOWS
#include <process.h>
#else
#error Threading API not detected.
#endif

typedef int (*except_t)(int, _EXCEPTION_POINTERS *);

class ThreadClass
{
public:
    ThreadClass(const char *thread_name, except_t exception_handler = nullptr);
    virtual ~ThreadClass();

    virtual void Thread_Function() = 0;
    void Execute();
    void Stop(unsigned int ms);
    bool Is_Running() { return m_handle != 0; }
    void Set_Priority(int priority);

    static void Sleep_Ms(unsigned int ms);
    static int Get_Current_Thread_ID();
    static void Switch_Thread();

#ifdef GAME_DLL
    static void Hook_Me();
#endif
private:
// Thread prototypes are slightly different between winapi threads and pthread.
#ifdef HAVE_PTHREAD_H
    static void *Internal_Thread_Function(void *params);
#elif defined PLATFORM_WINDOWS
    static void Internal_Thread_Function(void *params);
#endif

protected:
    volatile bool m_isRunning;
    char m_threadName[67];
    uintptr_t m_threadID;
    except_t m_exceptionHandler;
#ifdef HAVE_PTHREAD_H
    pthread_t m_handle;
#elif defined PLATFORM_WINDOWS
    HANDLE m_handle;
#endif
    int m_priority;
};

#ifdef GAME_DLL
inline void ThreadClass::Hook_Me()
{
    Hook_Method(0x0089CDC0, &Execute);
    Hook_Method(0x0089CE10, &Stop);
    Hook_Method(0x0089CDF0, &Set_Priority);
    Hook_Function(0x0089CEF0, &Switch_Thread);
    Hook_Function(0x0089CD10, &Internal_Thread_Function);
    Hook_Function(0x0089CF00, &Get_Current_Thread_ID);
}
#endif