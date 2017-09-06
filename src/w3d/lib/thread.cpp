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
#include "always.h"
#include "thread.h"
#include "gamedebug.h"
#include "stringex.h"
#include "systimer.h"
#include <cstdio>

#ifdef PLATFORM_WINDOWS
#include <mmsystem.h>
// void *test_event = CreateEventA(nullptr, FALSE, FALSE, "");
#define test_event (Make_Global<void *>(0x00A65178))
#else
#include <sched.h>
#include <unistd.h>
#endif

#ifdef PLATFORM_LINUX
#include <sys/syscall.h>
#include <sys/types.h>
#endif

ThreadClass::ThreadClass(const char *thread_name, except_t exception_handler) :
    m_isRunning(false),
    m_handle(0),
    m_priority(0)
{
    if (thread_name != nullptr) {
        // Safer copy, prevents buffer overrun
        strlcpy(m_threadName, thread_name, sizeof(m_threadName));
    } else {
        // We know this is safe for string "No Name" as buffer is 67.
        strcpy(m_threadName, "No Name");
    }

    m_exceptionHandler = exception_handler;
}

ThreadClass::~ThreadClass()
{
    m_isRunning = false;

    Stop(3000);
}

/**
 * @brief Wrapper to call classes virtual thread function from platform thread API.
 *
 * This calls the virtual Thread_Function so you create different types of
 * thread by deriving from the ThreadClass base and implementing the
 * virtual Thread_Function to do the actual work.
 */
#ifdef PLATFORM_WINDOWS
void ThreadClass::Internal_Thread_Function(void *params)
#else
void *ThreadClass::Internal_Thread_Function(void *params)
#endif
{
    //
    // Set is running true and call the virtual thread function. Function should
    // check for m_isRunning in its loop and finish if set false;
    //
    static_cast<ThreadClass *>(params)->m_isRunning = true;
    static_cast<ThreadClass *>(params)->m_threadID = Get_Current_Thread_ID();
    static_cast<ThreadClass *>(params)->Thread_Function();
    static_cast<ThreadClass *>(params)->m_handle = 0;

// Returns void * on none windows
#ifndef PLATFORM_WINDOWS
    return nullptr;
#endif
}

/**
 * @brief Start the thread executing.
 */
void ThreadClass::Execute()
{
    DEBUG_LOG("Executing thread '%s'.\n", m_threadName);
#ifdef PLATFORM_WINDOWS
    m_handle = (threadid_t)_beginthread(ThreadClass::Internal_Thread_Function, 0, this);
    SetThreadPriority(m_handle, m_priority);
#else
    // These can be used to set none default params
    // pthread_attr_t attr;
    // pthread_attr_init(&attr);
    // Second param can be pthread_attr_t, NULL causes it to use defaults
    pthread_create(&m_handle, nullptr, Internal_Thread_Function, this);
    Set_Priority(m_priority);
#endif // PLATFORM_WINDOWS
}

/**
 * @brief Set thread priority where supported by the platform.
 */
void ThreadClass::Set_Priority(int priority)
{
    m_priority = priority;

    if (m_handle != 0) {
#ifdef PLATFORM_WINDOWS
        SetThreadPriority(m_handle, m_priority);
#else
// This emulates windows SetThreadPriority behaviour.
// Should consider if we really need this as linux at least this won't have an effect.
#if 0
        struct sched_param param;
        int method;
        
        if ( pthread_getschedparam(m_handle, &method, &param) == 0 ) {
            if ( m_priority + 15 < 30 ) {
                switch( m_priority + 15 ) {
                    case 0:		//Fallthrough
                    case 13:
                        param.sched_priority = sched_get_priority_min(method);
                        break;
                        
                    case 14:
                        param.sched_priority = (sched_get_priority_min(method) + 0x1F) / 2;
                        break;
                        
                    case 15:
                        param.sched_priority = 31;
                        break;
                        
                    case 17:
                        param.sched_priority = sched_get_priority_max(method);
                        break;
                        
                    case 16:	//Fallthrough
                    case 30:
                        param.sched_priority = (sched_get_priority_max(method) + 0x1F) / 2;
                        break;
                        
                    default:
                        break;
                }
            }
            
            pthread_setschedparam(m_handle, method, &param);
        }
#endif
#endif // PLATFORM_WINDOWS
    }
}

/**
 * @brief Stop the thread, giving it a specified time to complete.
 */
void ThreadClass::Stop(unsigned int ms)
{
    DEBUG_LOG("Stopping thread '%s'.\n", m_threadName);
#ifdef PLATFORM_WINDOWS
    m_isRunning = false;
    unsigned int time = g_theSysTimer.Get();

    while (m_handle != 0) {
        if (g_theSysTimer.Get() - time > ms) {
            if (!TerminateThread(m_handle, 0)) {
                /*
                if ( byte_8A9AD9 ) {

                    TheCurrentAllowCrashPtr = (int)&byte_8A9AD9;
                    DebugCrash(aSSD, aRes, aCProjectsRt_23, 64);
                    TheCurrentAllowCrashPtr = 0;
                }
                */
            }

            m_handle = 0;
        }

        Sleep(0);
    }
#else
    m_isRunning = false;

    if (m_handle != 0) {
        if (pthread_join(m_handle, nullptr) != 0) {
            // Handle error from thread not ending?
        }
    }

    m_handle = 0;
#endif
}

/**
 * @brief Wrapper around the platform sleep function.
 */
void ThreadClass::Sleep_Ms(unsigned int ms)
{
#ifdef PLATFORM_WINDOWS
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

/**
 * @brief Unconditionally yield the thread.
 */
void ThreadClass::Switch_Thread()
{
#ifdef PLATFORM_WINDOWS
    // Waits for 1 millisecond
    WaitForSingleObject(test_event, 1);
#else
    //
    // Force thread to sleep, not quite same as windows code as it can be
    // interrupted by test_event, but it only sleeps for 1ms anyhow so who cares
    // if it can be interrupted in theory.
    //
    usleep(1);
#endif // PLATFORM_WINDOWS
}

/**
 * @brief Get an integral value that identifies the thread.
 */
uintptr_t ThreadClass::Get_Current_Thread_ID()
{
#ifdef PLATFORM_WINDOWS
    return GetCurrentThreadId();
#elif defined PLATFORM_LINUX
    return syscall(SYS_gettid);
#elif defined PLATFORM_APPLE || defined PLATFORM_BSD
    return pthread_getthreadid_np();
#else
#error Check platform documentation for appropriate function for integral thread id.
#endif
}
