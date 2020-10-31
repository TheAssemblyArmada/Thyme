/**
 * @file
 *
 * @author CCHyper
 * @author OmniBlade
 *
 * @brief Classes providing critical section behaviour.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "critsection.h"
#include "captainslog.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

CriticalSectionClass::CriticalSectionClass() : m_handle(), m_locked(0)
{
#ifdef HAVE_PTHREAD_H
    pthread_mutexattr_t attr;

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&m_handle, &attr);
#elif defined PLATFORM_WINDOWS
    m_handle = new CRITICAL_SECTION;
    InitializeCriticalSection(m_handle);
#endif
}

CriticalSectionClass::~CriticalSectionClass()
{
    captainslog_assert(!m_locked);
#ifdef HAVE_PTHREAD_H
    pthread_mutex_destroy(&m_handle);
#elif defined PLATFORM_WINDOWS
    DeleteCriticalSection(m_handle);
    delete m_handle;
#endif
}

/**
 * Performs the lock when entering a critical section of code. Private and is only called from the Lock object.
 */
void CriticalSectionClass::Lock()
{
#ifdef HAVE_PTHREAD_H
    pthread_mutex_lock(&m_handle);
#elif defined PLATFORM_WINDOWS
    EnterCriticalSection(m_handle);
#endif
    ++m_locked;
}

/**
 * Removes the lock when leaving a critical section of code. Private and is only called from the Lock object.
 */
void CriticalSectionClass::Unlock()
{
    captainslog_assert(m_locked);
#ifdef HAVE_PTHREAD_H
    pthread_mutex_unlock(&m_handle);
#elif defined PLATFORM_WINDOWS
    LeaveCriticalSection(m_handle);
#endif
    --m_locked;
}

/**
 * Performs the lock when entering a critical section of code. Private and is only called from the Lock object.
 */
void FastCriticalSectionClass::Thread_Safe_Set_Flag()
{
#ifndef GAME_DLL
    while (m_flag.test_and_set(std::memory_order_seq_cst)) {
#else
    // Should work for both x86_32 and x86_64 plus no assembly.
    while (_interlockedbittestandset(&m_flag, 0)) {
#endif
        // Yield the thread if no lock aquired.
#ifdef HAVE_UNISTD_H
        usleep(1);
#elif defined PLATFORM_WINDOWS
        Sleep(1);
#else
#error Add appropriate sleep function to critsection.cpp
#endif
    }
}

/**
 * Removes the lock when leaving a critical section of code. Private and is only called from the Lock object.
 */
void FastCriticalSectionClass::Thread_Safe_Clear_Flag()
{
#ifndef GAME_DLL
    m_flag.clear();
#else
    m_flag = 0;
#endif
}
