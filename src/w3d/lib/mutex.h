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
#pragma once

#include "always.h"

#ifndef GAME_DLL
#include <atomic>
#endif

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#elif defined PLATFORM_WINDOWS
#include <handleapi.h>
#include <synchapi.h>
#include <winbase.h>
#else
#error Threading implementation not detected in mutex.h
#endif

class SimpleMutexClass // Called CriticalSection in ZH 1.04 Mac,
{
    enum
    {
        LOCK_WAIT = 500,
    };

public:
    SimpleMutexClass(const char *name);
    ~SimpleMutexClass(); // Virtual in ZH windows 1.04, not Mac.

    void Lock();
    void Unlock();

protected:
    SimpleMutexClass &operator=(SimpleMutexClass const &that) { return *this; }

#ifdef HAVE_PTHREAD_H
    pthread_mutex_t m_handle;
#elif defined PLATFORM_WINDOWS
    HANDLE m_handle;
#endif
    const char *m_name;
};

inline SimpleMutexClass::SimpleMutexClass(const char *name) : m_name(name)
{
#ifdef HAVE_PTHREAD_H
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&m_handle, &attr);
#elif defined PLATFORM_WINDOWS
    m_handle = CreateMutexA(nullptr, 0, name);
#endif
}

inline SimpleMutexClass::~SimpleMutexClass()
{
#ifdef HAVE_PTHREAD_H
    pthread_mutex_destroy(&m_handle);
#elif defined PLATFORM_WINDOWS
    CloseHandle(m_handle);
#endif
}

inline void SimpleMutexClass::Lock()
{
#ifdef HAVE_PTHREAD_H
    // Uses of this inline class proceed without checking if the lock succeeded in windows
    pthread_mutex_trylock(&m_handle);
#elif defined PLATFORM_WINDOWS
    WaitForSingleObject(m_handle, LOCK_WAIT);
#endif
}

inline void SimpleMutexClass::Unlock()
{
#ifdef HAVE_PTHREAD_H
    pthread_mutex_unlock(&m_handle);
#elif defined PLATFORM_WINDOWS
    ReleaseMutex(m_handle);
#endif
}

class ScopedMutexClass
{
public:
    ScopedMutexClass(SimpleMutexClass *mutex) : m_mutex(mutex)
    {
        if (mutex != nullptr) {
            m_mutex->Lock();
        }
    }

    virtual ~ScopedMutexClass()
    {
        if (m_mutex != nullptr) {
            m_mutex->Unlock();
        }
    }

private:
    SimpleMutexClass *m_mutex;
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
class MutexClass
{
public:
    enum
    {
        WAIT_INFINITE = -1
    };

public:
    MutexClass(const char *name = nullptr);
    ~MutexClass();

    class LockClass
    {
    public:
        // In order to enter a critical section create a local instance of LockClass with critical section as a parameter.
        LockClass(MutexClass &critical_section, int time = MutexClass::WAIT_INFINITE) : m_muxtex(critical_section)
        {
            m_failed = !m_muxtex.Lock(time);
        }
        ~LockClass()
        {
            if (!m_failed)
                m_muxtex.Unlock();
        }
        bool Failed() const { return m_failed; }

    private:
        LockClass &operator=(LockClass const &that) { return *this; }
        MutexClass &m_muxtex;
        bool m_failed;
    };

    friend class LockClass;

private:
    // Lock and unlock are private, you have to create a
    // MutexClass::LockClass object to call them instead.
    bool Lock(int time);
    void Unlock();
    bool Is_Locked() { return m_locked > 0; }

#ifdef HAVE_PTHREAD_H
    pthread_mutex_t m_handle;
#elif defined PLATFORM_WINDOWS
    HANDLE m_handle;
#endif
    unsigned int m_locked;
};
