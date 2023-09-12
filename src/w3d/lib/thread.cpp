/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Base class to wrap threading API.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "thread.h"
#include "rtsutilsw3d.h"
#include "systimer.h"
#include "threadtrack.h"
#include <captainslog.h>
#include <cstdio>
#include <cstring>

using std::strcpy;

#ifdef PLATFORM_WINDOWS
#include <mmsystem.h>
#include <processthreadsapi.h>
#include <synchapi.h>
#ifdef GAME_DLL
#include "hooker.h"
#define test_event (Make_Global<void *>(0x00A65178))
#else
void *test_event = CreateEventA(nullptr, FALSE, FALSE, "");
#endif
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SCHED_H
#include <sched.h>
#endif

#ifdef PLATFORM_LINUX
#include <sys/syscall.h>
#endif

#if defined PLATFORM_FREEBSD || defined PLATFORM_OSX
#ifdef PLATFORM_FREEBSD
#include <pthread_np.h>
#endif
#include <sys/sysctl.h>
#endif

ThreadClass::ThreadClass(const char *thread_name, except_t exception_handler) :
    m_isRunning(false), m_handle(0), m_priority(0)
{
    if (thread_name != nullptr) {
        // Safer copy, prevents buffer overrun
        strlcpy_tpl(m_threadName, thread_name);
    } else {
        // We know this is safe for string "No Name" as buffer is 67.
        strcpy(m_threadName, "No Name");
    }

    m_exceptionHandler = exception_handler;
    // #BUGFIX Initialize all members
    m_threadID = 0;
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
#ifdef HAVE_PTHREAD_H
void *ThreadClass::Internal_Thread_Function(void *params)
#elif defined PLATFORM_WINDOWS
void ThreadClass::Internal_Thread_Function(void *params)
#endif
{
    // Set is running true and call the virtual thread function. Function should
    // check for m_isRunning in its loop and finish if set false;
    static_cast<ThreadClass *>(params)->m_isRunning = true;
    static_cast<ThreadClass *>(params)->m_threadID = Get_Current_Thread_ID();
    Register_Thread_ID(
        static_cast<ThreadClass *>(params)->m_threadID, static_cast<ThreadClass *>(params)->m_threadName, false);
    static_cast<ThreadClass *>(params)->Thread_Function();
    Unregister_Thread_ID(static_cast<ThreadClass *>(params)->m_threadID, static_cast<ThreadClass *>(params)->m_threadName);
    static_cast<ThreadClass *>(params)->m_handle = 0;

// Returns void * for pthread
#ifdef HAVE_PTHREAD_H
    return nullptr;
#endif
}

/**
 * @brief Start the thread executing.
 */
void ThreadClass::Execute()
{
    captainslog_trace("Executing thread '%s'.", m_threadName);
#ifdef HAVE_PTHREAD_H
    // These can be used to set none default params
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    // Second param can be pthread_attr_t, NULL causes it to use defaults
    pthread_create(&m_handle, &attr, Internal_Thread_Function, this);
    Set_Priority(m_priority);
#elif defined PLATFORM_WINDOWS
    m_handle = (HANDLE)_beginthread(ThreadClass::Internal_Thread_Function, 0, this);
    SetThreadPriority(m_handle, m_priority);
#endif
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
#endif // PLATFORM_WINDOWS
    }
}

/**
 * @brief Stop the thread, giving it a specified time to complete.
 */
void ThreadClass::Stop(unsigned int ms)
{
    captainslog_trace("Stopping thread '%s'.", m_threadName);
    m_isRunning = false;
    unsigned int time = g_theSysTimer.Get();

    while (m_handle != 0) {
        if (g_theSysTimer.Get() - time > ms) {
#ifdef HAVE_PTHREAD_H
            if (pthread_cancel(m_handle) != 0 || pthread_join(m_handle, nullptr) != 0) {
#elif defined PLATFORM_WINDOWS
            if (!TerminateThread(m_handle, 0)) {
#endif
                captainslog_assert(false);
            }

            m_handle = 0;
        }

#ifdef HAVE_SCHED_H
        sched_yield();
#elif defined PLATFORM_WINDOWS
        Sleep(0);
#else
#error Add appropriate thread yield function to thread.cpp
#endif
    }
}

/**
 * @brief Wrapper around the platform sleep function.
 */
void ThreadClass::Sleep_Ms(unsigned int ms)
{
    rts::Sleep_Ms(ms);
}

/**
 * @brief Unconditionally yield the thread.
 */
void ThreadClass::Switch_Thread()
{
#ifdef HAVE_UNISTD_H
    usleep(1);
#elif defined PLATFORM_WINDOWS
    WaitForSingleObject(test_event, 1);
#else
#error Add appropriate unconditionally yield function to thread.cpp
#endif
}

/**
 * @brief Get an integral value that identifies the thread.
 */
int ThreadClass::Get_Current_Thread_ID()
{
#ifdef PLATFORM_WINDOWS
    return GetCurrentThreadId();
#elif defined PLATFORM_LINUX
    pid_t tid = syscall(SYS_gettid);
    return tid;
#elif defined PLATFORM_APPLE
    uint64_t ktid;
    pthread_threadid_np(NULL, &ktid);
    return ktid;
#elif defined PLATFORM_FREEBSD
    return pthread_getthreadid_np();
#elif defined PLATFORM_OPENBSD
    return getthrid();
#else
#error Check platform documentation for appropriate function for integral thread id.
#endif
}
