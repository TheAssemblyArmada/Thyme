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
#pragma once

#include "always.h"

#ifndef GAME_DLL
#include <atomic>
#endif

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#elif defined PLATFORM_WINDOWS
#include <synchapi.h>
#else
#error Threading API not detected.
#endif

/**
 * @brief Wrapper around WinAPI critical secitons and pthread mutexes.
 *
 * Simple critical sections are a thin cross platform wrapper around WinAPI
 * critical sections or pthreads recursive mutexes which perform the same
 * function and behave similarly. Create a SimpleCriticalSectionClass object
 * that can be access by the threads that need shared access to some resource
 * and have them call Enter to get a lock before manipulating the shared
 * resource and then call Leave after it is done. Try_Enter can be used to
 * test for a lock and selectively manipulate shared data based on if the lock
 * is available.
 */
class SimpleCriticalSectionClass // Called CriticalSection in ZH 1.04 Mac,
{
public:
    SimpleCriticalSectionClass();
    virtual ~SimpleCriticalSectionClass(); // Virtual in ZH windows 1.04, not Mac.

    void Enter();
    bool Try_Enter();
    void Leave();

protected:
    SimpleCriticalSectionClass &operator=(SimpleCriticalSectionClass const &that) { return *this; }

#ifdef HAVE_PTHREAD_H
    pthread_mutex_t m_handle;
#elif defined PLATFORM_WINDOWS
    CRITICAL_SECTION m_handle;
#endif
};

inline SimpleCriticalSectionClass::SimpleCriticalSectionClass() : m_handle()
{
#ifdef HAVE_PTHREAD_H
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&m_handle, &attr);
#elif defined PLATFORM_WINDOWS
    InitializeCriticalSection(&m_handle);
#endif
}

inline SimpleCriticalSectionClass::~SimpleCriticalSectionClass()
{
#ifdef HAVE_PTHREAD_H
    pthread_mutex_destroy(&m_handle);
#elif defined PLATFORM_WINDOWS
    DeleteCriticalSection(&m_handle);
#endif
}

inline void SimpleCriticalSectionClass::Enter()
{
#ifdef HAVE_PTHREAD_H
    pthread_mutex_lock(&m_handle);
#elif defined PLATFORM_WINDOWS
    EnterCriticalSection(&m_handle);
#endif
}

inline bool SimpleCriticalSectionClass::Try_Enter()
{
#ifdef HAVE_PTHREAD_H
    return pthread_mutex_trylock(&m_handle) == 0;
#elif defined PLATFORM_WINDOWS
    return TryEnterCriticalSection(&m_handle) != FALSE;
#endif
}

inline void SimpleCriticalSectionClass::Leave()
{
#ifdef HAVE_PTHREAD_H
    pthread_mutex_unlock(&m_handle);
#elif defined PLATFORM_WINDOWS
    LeaveCriticalSection(&m_handle);
#endif
}

class ScopedCriticalSectionClass
{
public:
    ScopedCriticalSectionClass(SimpleCriticalSectionClass *cs) : m_critSection(cs)
    {
        if (cs != nullptr) {
            // DEBUG_LOG("Entering CriticalSection from scoped lock\n");
            m_critSection->Enter();
        }
    }

    virtual ~ScopedCriticalSectionClass()
    {
        if (m_critSection != nullptr) {
            // DEBUG_LOG("Leaving CriticalSection from scoped lock\n");
            m_critSection->Leave();
        }
    }

private:
    SimpleCriticalSectionClass *m_critSection;
};

/**
 * @brief Wrapper around WinAPI critical secitons and pthread mutexes.
 *
 * Critical section wraps WinAPI critical sections or pthreads recursive
 * mutexes to create an automatically unlocking lock that can allow safe
 * sharing of data between threads. To obtain a lock, a lock object is created
 * with the a shared critical section object being passed to it. The lock
 * object constructor will perform the lock and then the destructor will unlock
 * it automatically when the lock object goes out of scope or the destructor
 * is called manually as per RAII programming methodology.
 */
class CriticalSectionClass
{
public:
    CriticalSectionClass();
    ~CriticalSectionClass();

    class LockClass
    {
    public:
        // In order to enter a critical section create a local instance of LockClass with critical section as a parameter.
        LockClass(CriticalSectionClass &critical_section) : CriticalSection(critical_section) { CriticalSection.Lock(); }

        ~LockClass() { CriticalSection.Unlock(); }

    private:
        LockClass &operator=(LockClass const &that) { return *this; }
        CriticalSectionClass &CriticalSection;
    };

    friend class LockClass;

private:
    // Lock and unlock are private, you have to create a
    // CriticalSectionClass::LockClass object to call them instead.
    void Lock();
    void Unlock();
    bool Is_Locked() { return m_locked > 0; }

#ifdef HAVE_PTHREAD_H
    pthread_mutex_t m_handle;
#elif defined PLATFORM_WINDOWS
    CRITICAL_SECTION *m_handle;
#endif

    unsigned int m_locked;
};

/**
 * @brief Critical section behaviour implemented with compiler intrinsics.
 *
 * Fast critical section uses compiler intrinsics to create an automatically
 * unlocking lock that can allow safe sharing of data between threads. It is
 * used the same as the normal CriticalSectionClass but can only lock once
 * and is an acquisition barrier only.
 */
class FastCriticalSectionClass
{
public:
#ifndef GAME_DLL
    FastCriticalSectionClass() { m_flag.clear(); }
#else
    FastCriticalSectionClass() : m_flag(0) {}
#endif
    ~FastCriticalSectionClass() {}

    class LockClass
    {
    public:
        LockClass(FastCriticalSectionClass &critical_section) : CriticalSection(critical_section)
        {
            CriticalSection.Thread_Safe_Set_Flag();
        }

        ~LockClass() { CriticalSection.Thread_Safe_Clear_Flag(); }

    private:
        LockClass &operator=(LockClass const &that) { return *this; }
        FastCriticalSectionClass &CriticalSection;
    };

    friend class LockClass;

private:
    void Thread_Safe_Set_Flag();
    void Thread_Safe_Clear_Flag();

private:
#ifndef GAME_DLL
    std::atomic_flag m_flag;
#else
    long m_flag;
#endif
};
