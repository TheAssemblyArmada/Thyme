////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: CRITSECTION.CPP
//
//        Author:: OmniBlade
//
//  Contributors:: CCHyper
//
//   Description:: 
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
#include	"critsection.h"

////////////////////////////////////////////////////////////////////////////////
///
/// <!-- CriticalSectionClass::CriticalSectionClass() -->
///
/// \brief
///     Class constructor, initialises the platform object used as a mutex.
///
////////////////////////////////////////////////////////////////////////////////
CriticalSectionClass::CriticalSectionClass() :
    Handle(),
    Locked(0)
{
#ifdef PLATFORM_WINDOWS
    InitializeCriticalSection(&Handle);
#else
    pthread_mutexattr_t attr;
    
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&Handle, &attr);
#endif // PLATFORM_WINDOWS
}

////////////////////////////////////////////////////////////////////////////////
///
/// <!-- CriticalSectionClass::~CriticalSectionClass() -->
///
/// \brief
///     Class destructor, frees the platform object used as a mutex.
///
////////////////////////////////////////////////////////////////////////////////
CriticalSectionClass::~CriticalSectionClass()
{  
#ifdef PLATFORM_WINDOWS
    DeleteCriticalSection(&Handle);
#else
    pthread_mutex_destroy(&Handle);
#endif // PLATFORM_WINDOWS

}

////////////////////////////////////////////////////////////////////////////////
///
/// <!-- CriticalSectionClass::Lock() -->
///
/// \brief
///     Performs the lock when entering a critical section of code. Private and
///     is only called from the Lock object.
///
////////////////////////////////////////////////////////////////////////////////
void CriticalSectionClass::Lock()
{    
#ifdef PLATFORM_WINDOWS	
    EnterCriticalSection(&Handle);
#else
    pthread_mutex_lock(&Handle);
#endif // PLATFORM_WINDOWS

    //
    // 
    //
    ++Locked;
}

////////////////////////////////////////////////////////////////////////////////
///
/// <!-- CriticalSectionClass::Unlock() -->
///
/// \brief
///     Removes the lock when leaving a critical section of code. Private and is
///     only called from the Lock object.
///
////////////////////////////////////////////////////////////////////////////////
void CriticalSectionClass::Unlock()
{
#ifdef PLATFORM_WINDOWS	
    LeaveCriticalSection(&Handle);
#else
    pthread_mutex_unlock(&Handle);
#endif // PLATFORM_WINDOWS

    //
    // 
    //
    --Locked;
}

////////////////////////////////////////////////////////////////////////////////
///
/// <!-- FastCriticalSectionClass::Thread_Safe_Set_Lock() -->
///
/// \brief
///     Performs the lock when entering a critical section of code. Private and
///     is only called from the Lock object.
///
////////////////////////////////////////////////////////////////////////////////
void FastCriticalSectionClass::Thread_Safe_Set_Flag()
{
#ifdef PLATFORM_WINDOWS
    //
    // Should work for both x86_32 and x86_64 plus no assembly.
    //
    while ( _interlockedbittestandset(&Flag, 0) ) {
        //
        // Yield the thread if no lock aquired.
        //
        Sleep(1);
    }
#elif defined COMPILER_GNUC || defined COMPILER_CLANG
    while ( __sync_lock_test_and_set(&Flag, 1) ) {
        //
        // Yield the thread if no lock aquired.
        //
        usleep(1);
    }
#else
    //TODO
#endif // COMPILER_MSVC
}

////////////////////////////////////////////////////////////////////////////////
///
/// <!-- FastCriticalSectionClass::Thread_Safe_Clear_Flag() -->
///
/// \brief
///     Removes the lock when leaving a critical section of code. Private and is
///     only called from the Lock object.
///
////////////////////////////////////////////////////////////////////////////////
void FastCriticalSectionClass::Thread_Safe_Clear_Flag()
{
    Flag = 0;
}
