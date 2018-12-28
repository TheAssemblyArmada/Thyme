/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Classes providing mutex behaviour.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "mutex.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

MutexClass::MutexClass(const char *name) : m_handle(), m_locked(0)
{
#ifdef HAVE_PTHREAD_H
    // The mac build appears to just use critical sections for all MutexClass instances?
    // TODO make use of pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED) to properly support same functionality?
    pthread_mutexattr_t attr;

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&m_handle, &attr);
#elif defined PLATFORM_WINDOWS
    m_handle = CreateMutexA(nullptr, false, name);
#endif
}

MutexClass::~MutexClass()
{
#ifdef PLATFORM_WINDOWS
    CloseHandle(&m_handle);
#else
    pthread_mutex_destroy(&m_handle);
#endif // PLATFORM_WINDOWS
}

/**
 * Performs the lock when entering a critical section of code. Private and is only called from the Lock object.
 */
bool MutexClass::Lock(int time)
{
#ifdef PLATFORM_WINDOWS
    if (WaitForSingleObject(m_handle, time == WAIT_INFINITE ? INFINITE : time) != WAIT_OBJECT_0) {
        return false;
    }

    ++m_locked;

    return true;
#else
    int err;

    if (time == WAIT_INFINITE) {
        err = pthread_mutex_lock(&m_handle);
    } else {
        err = pthread_mutex_trylock(&m_handle);
    }

    if (err == 0) {
        ++m_locked;
    }

    return err == 0;
#endif // PLATFORM_WINDOWS
}

/**
 * Removes the lock when leaving a critical section of code. Private and is only called from the Lock object.
 */
void MutexClass::Unlock()
{
#ifdef PLATFORM_WINDOWS
    ReleaseMutex(m_handle);
#else
    pthread_mutex_unlock(&m_handle);
#endif // PLATFORM_WINDOWS

    --m_locked;
}
