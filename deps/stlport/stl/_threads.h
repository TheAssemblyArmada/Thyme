/*
 * Copyright (c) 1997-1999
 * Silicon Graphics Computer Systems, Inc.
 *
 * Copyright (c) 1999 
 * Boris Fomitchev
 *
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use or copy this software for any purpose is hereby granted 
 * without fee, provided the above notices are retained on all copies.
 * Permission to modify the code and to distribute modified code is granted,
 * provided the above notices are retained, and a notice that the code was
 * modified is included with the above copyright notice.
 *
 */

// WARNING: This is an internal header file, included by other C++
// standard library headers.  You should not attempt to use this header
// file directly.
// Stl_config.h should be included before this file.


#ifndef _STLP_INTERNAL_THREADS_H
#define _STLP_INTERNAL_THREADS_H

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4275)
#endif

// Supported threading models are native SGI, pthreads, uithreads
// (similar to pthreads, but based on an earlier draft of the Posix
// threads standard), and Win32 threads.  Uithread support by Jochen
// Schlick, 1999, and Solaris threads generalized to them.

#ifndef _STLP_CONFIG_H
#include <stl/_config.h>
#endif

# if ! defined (_STLP_CSTDDEF)
#  include <cstddef>
# endif

# if ! defined (_STLP_CSTDLIB)
#  include <cstdlib>
# endif

// On SUN and Mac OS X gcc, zero-initialization works just fine...
# if defined (__sun) || ( defined(__GNUC__) && defined(__APPLE__) )
# define _STLP_MUTEX_INITIALIZER
# endif

# if defined (_STLP_WIN32) || defined (__sgi) || defined (_STLP_SPARC_SOLARIS_THREADS)
  typedef long __stl_atomic_t;
# else 
# if defined (_STLP_USE_NAMESPACES) && ! defined (_STLP_VENDOR_GLOBAL_CSTD)
using _STLP_VENDOR_CSTD::size_t;
# endif
  typedef size_t __stl_atomic_t;
#endif  

# if defined(_STLP_SGI_THREADS)
#  include <mutex.h>
// Hack for SGI o32 compilers.
#if !defined(__add_and_fetch) && \
    (__mips < 3 || !(defined (_ABIN32) || defined(_ABI64)))
#  define __add_and_fetch(__l,__v) add_then_test((unsigned long*)__l,__v)  
#  define __test_and_set(__l,__v)  test_and_set(__l,__v)
#endif /* o32 */

# if __mips < 3 || !(defined (_ABIN32) || defined(_ABI64))
#  define _STLP_ATOMIC_EXCHANGE(__p, __q) test_and_set(__p, __q)
# else
#  define _STLP_ATOMIC_EXCHANGE(__p, __q) __test_and_set((unsigned long*)__p, (unsigned long)__q)
# endif

#  define _STLP_ATOMIC_INCREMENT(__x) __add_and_fetch(__x, 1)
#  define _STLP_ATOMIC_DECREMENT(__x) __add_and_fetch(__x, (size_t) -1)
# elif defined(_STLP_PTHREADS)
#  include <pthread.h>
#  if defined (PTHREAD_MUTEX_INITIALIZER) && ! defined (_STLP_MUTEX_INITIALIZER) && defined (_REENTRANT)
#   define _STLP_MUTEX_INITIALIZER = { PTHREAD_MUTEX_INITIALIZER }
#  endif

//HPUX variants have (on some platforms optional) non-standard "DCE" pthreads impl
#  if defined(_DECTHREADS_) && ( defined (_PTHREAD_USE_D4) || defined (__hpux)) \
   && ! defined (_CMA_SUPPRESS_EXTERNALS_)
#   define _STLP_PTHREAD_ATTR_DEFAULT pthread_mutexattr_default
#  else
#   define _STLP_PTHREAD_ATTR_DEFAULT 0
#  endif

# elif defined(_STLP_WIN32)
#  if !defined (_STLP_WINDOWS_H_INCLUDED) && ! defined (_WINDOWS_H)
#   if ! (defined ( _STLP_MSVC ) || defined (__BORLANDC__) || defined (__ICL) || defined (__WATCOMC__) || defined (__MINGW32__)) 
#    ifdef _STLP_USE_MFC
#     include <afx.h>
#    else
#     include <windows.h>
#    endif
#   else 
// This section serves as a replacement for windows.h header for Visual C++
extern "C" {
#   if (defined(_M_MRX000) || defined(_M_ALPHA) \
       || (defined(_M_PPC) && (_MSC_VER >= 1000))) && !defined(RC_INVOKED)
#    define InterlockedIncrement       _InterlockedIncrement
#    define InterlockedDecrement       _InterlockedDecrement
#    define InterlockedExchange        _InterlockedExchange
#    define _STLP_STDCALL
#   else
#    ifdef _MAC
#     define _STLP_STDCALL _cdecl
#    else
#     define _STLP_STDCALL __stdcall
#    endif
#   endif

#if (_MSC_VER >= 1300) || defined (_STLP_NEW_PLATFORM_SDK)
_STLP_IMPORT_DECLSPEC long InterlockedIncrement(long volatile *);
_STLP_IMPORT_DECLSPEC long InterlockedDecrement(long volatile *);
_STLP_IMPORT_DECLSPEC long InterlockedExchange(long volatile *, long);
#else
  // boris : for the latest SDK, you may actually need the other version of the declaration (above)
  // even for earlier VC++ versions. There is no way to tell SDK versions apart, sorry ...
_STLP_IMPORT_DECLSPEC long _STLP_STDCALL InterlockedIncrement(long*);
_STLP_IMPORT_DECLSPEC long _STLP_STDCALL InterlockedDecrement(long*);
_STLP_IMPORT_DECLSPEC long _STLP_STDCALL InterlockedExchange(long*, long);
#endif

_STLP_IMPORT_DECLSPEC void _STLP_STDCALL Sleep(unsigned long);
_STLP_IMPORT_DECLSPEC void _STLP_STDCALL OutputDebugStringA( const char* lpOutputString );

#    if defined (InterlockedIncrement)
#     pragma intrinsic(_InterlockedIncrement)
#     pragma intrinsic(_InterlockedDecrement)
#     pragma intrinsic(_InterlockedExchange)
#    endif
} /* extern "C" */

#   endif /* STL_MSVC */

#   define _STLP_WINDOWS_H_INCLUDED

#  endif /* _STLP_WIN32 */

#  ifndef _STLP_ATOMIC_INCREMENT
#   define _STLP_ATOMIC_INCREMENT(__x)           InterlockedIncrement((long*)__x)
#   define _STLP_ATOMIC_DECREMENT(__x)           InterlockedDecrement((long*)__x)
#   define _STLP_ATOMIC_EXCHANGE(__x, __y)       InterlockedExchange((long*)__x, (long)__y)
#  endif
# elif defined(__DECC) || defined(__DECCXX)
#  include <machine/builtins.h>
#  define _STLP_ATOMIC_EXCHANGE __ATOMIC_EXCH_LONG
#  define _STLP_ATOMIC_INCREMENT(__x) __ATOMIC_ADD_LONG(__x, 1)
#  define _STLP_ATOMIC_DECREMENT(__x) __ATOMIC_ADD_LONG(__x, -1)
# elif defined(_STLP_SPARC_SOLARIS_THREADS)
#  include <stl/_sparc_atomic.h>
# elif defined (_STLP_UITHREADS)
// this inclusion is potential hazard to bring up all sorts
// of old-style headers. Let's assume vendor already know how
// to deal with that.
#  include <ctime>
# if defined (_STLP_USE_NAMESPACES) && ! defined (_STLP_VENDOR_GLOBAL_CSTD)
using _STLP_VENDOR_CSTD::time_t;
# endif
#  include <synch.h>
#  include <cstdio>
#  include <cwchar>
# elif defined (_STLP_BETHREADS)
#  include <OS.h>
#include <cassert>
#include <stdio.h>
#  define _STLP_MUTEX_INITIALIZER = { 0 }
#elif defined(_STLP_OS2THREADS)
  // This section serves to replace os2.h for VisualAge C++
  typedef unsigned long ULONG;
  #ifndef __HEV__  /* INCL_SEMAPHORE may also define HEV */
    #define __HEV__
    typedef ULONG HEV;
    typedef HEV*  PHEV;
  #endif
  typedef ULONG APIRET;
  typedef ULONG HMTX;
  typedef HMTX*  PHMTX;
  typedef const char*  PCSZ;
  typedef ULONG BOOL32;
  APIRET _System DosCreateMutexSem(PCSZ pszName, PHEV phev, ULONG
flAttr, BOOL32 fState);
  APIRET _System DosRequestMutexSem(HMTX hmtx, ULONG ulTimeout);
  APIRET _System DosReleaseMutexSem(HMTX hmtx);
  APIRET _System DosCloseMutexSem(HMTX hmtx);
# define _STLP_MUTEX_INITIALIZER = { 0 };

# endif

# ifndef _STLP_MUTEX_INITIALIZER
#   if defined(_STLP_ATOMIC_EXCHANGE)
#     define _STLP_MUTEX_INITIALIZER = { 0 }
#   elif defined(_STLP_UITHREADS)
#     define _STLP_MUTEX_INITIALIZER = { DEFAULTMUTEX }
#   else
#     define _STLP_MUTEX_INITIALIZER
#   endif
# endif

_STLP_BEGIN_NAMESPACE
// Helper struct.  This is a workaround for various compilers that don't
// handle static variables in inline functions properly.
template <int __inst>
struct _STLP_mutex_spin {
  enum { __low_max = 30, __high_max = 1000 };
  // Low if we suspect uniprocessor, high for multiprocessor.
  static unsigned __max;
  static unsigned __last;
  static void _STLP_CALL _M_do_lock(volatile __stl_atomic_t* __lock);
  static void _STLP_CALL _S_nsec_sleep(int __log_nsec);
};


// Locking class.  Note that this class *does not have a constructor*.
// It must be initialized either statically, with _STLP_MUTEX_INITIALIZER,
// or dynamically, by explicitly calling the _M_initialize member function.
// (This is similar to the ways that a pthreads mutex can be initialized.)
// There are explicit member functions for acquiring and releasing the lock.

// There is no constructor because static initialization is essential for
// some uses, and only a class aggregate (see section 8.5.1 of the C++
// standard) can be initialized that way.  That means we must have no
// constructors, no base classes, no virtual functions, and no private or
// protected members.

// For non-static cases, clients should use  _STLP_mutex.

struct _STLP_CLASS_DECLSPEC _STLP_mutex_base
{
#if defined(_STLP_ATOMIC_EXCHANGE) || defined(_STLP_SGI_THREADS)
  // It should be relatively easy to get this to work on any modern Unix.
  volatile __stl_atomic_t _M_lock;
#endif

#ifdef _STLP_THREADS

#if defined (_STLP_ATOMIC_EXCHANGE)
  inline void _M_initialize() { _M_lock=0; }
  inline void _M_destroy() {}

  void _M_acquire_lock() {
    _STLP_mutex_spin<0>::_M_do_lock(&_M_lock);
  }

  inline void _M_release_lock() {
    volatile __stl_atomic_t* __lock = &_M_lock;
#   if defined(_STLP_SGI_THREADS) && defined(__GNUC__) && __mips >= 3
        asm("sync");
        *__lock = 0;
#   elif defined(_STLP_SGI_THREADS) && __mips >= 3 \
	 && (defined (_ABIN32) || defined(_ABI64))
        __lock_release(__lock);
#   elif defined (_STLP_SPARC_SOLARIS_THREADS)
#    if defined (__WORD64) || defined (__arch64__) \
     || defined (__sparcv9) || defined (__sparcv8plus)
	asm("membar #StoreStore ; membar #LoadStore");
#    else
	asm(" stbar ");
#    endif
        *__lock = 0;	
#   else
        *__lock = 0;
        // This is not sufficient on many multiprocessors, since
        // writes to protected variables and the lock may be reordered.
#   endif
  }
#elif defined(_STLP_PTHREADS)
  pthread_mutex_t _M_lock;
  inline void _M_initialize() {
    pthread_mutex_init(&_M_lock,_STLP_PTHREAD_ATTR_DEFAULT);
  }
  inline void _M_destroy() {
    pthread_mutex_destroy(&_M_lock);
  }
  inline void _M_acquire_lock() { 

#if defined ( __hpux ) && ! defined (PTHREAD_MUTEX_INITIALIZER)
    if (!_M_lock.field1)  _M_initialize();
#endif
    pthread_mutex_lock(&_M_lock); 
  }
  inline void _M_release_lock() { pthread_mutex_unlock(&_M_lock); }
  
#elif defined (_STLP_UITHREADS)
  mutex_t _M_lock;
  inline void _M_initialize() {
    mutex_init(&_M_lock,0,NULL);	
  }
  inline void _M_destroy() {
    mutex_destroy(&_M_lock);
  }
  inline void _M_acquire_lock() { mutex_lock(&_M_lock); }
  inline void _M_release_lock() { mutex_unlock(&_M_lock); }

#elif defined(_STLP_OS2THREADS)
  HMTX _M_lock;
  inline void _M_initialize() { DosCreateMutexSem(NULL, &_M_lock, 0,
false); }
  inline void _M_destroy() { DosCloseMutexSem(_M_lock); }
  inline void _M_acquire_lock() {
    if(!_M_lock) _M_initialize();
    DosRequestMutexSem(_M_lock, -1);
  }
  inline void _M_release_lock() { DosReleaseMutexSem(_M_lock); }
#elif defined(_STLP_BETHREADS)
  sem_id sem;
  inline void _M_initialize() 
  {
     sem = create_sem(1, "STLPort");
     assert(sem > 0);
  }
  inline void _M_destroy() 
  {
     int t = delete_sem(sem);
     assert(t == B_NO_ERROR);
  }
  inline void _M_acquire_lock();
  inline void _M_release_lock() 
  {
     status_t t = release_sem(sem);
     assert(t == B_NO_ERROR);
  }
#else		//*ty 11/24/2001 - added configuration check
#error "Unknown thread facility configuration"
#endif
#else /* No threads */
  inline void _M_initialize() {}
  inline void _M_destroy() {}
  inline void _M_acquire_lock() {}
  inline void _M_release_lock() {}
#endif
};

// This class could be just a smart pointer, but we do want to keep 
// WIN32 optimized at a maximum
#if  defined(_STLP_ATOMIC_EXCHANGE) || defined(_STLP_BETHREADS)
struct _STLP_CLASS_DECLSPEC _STLP_mutex_indirect : _STLP_mutex_base {};
#else
struct _STLP_CLASS_DECLSPEC _STLP_mutex_indirect
{
  void*    _M_lock;

# if defined(_STLP_PTHREADS)
  inline void _M_initialize() {
    _M_lock = (void*)_STLP_VENDOR_CSTD::calloc(1,sizeof(pthread_mutex_t));
    pthread_mutex_init((pthread_mutex_t*)_M_lock,_STLP_PTHREAD_ATTR_DEFAULT);
  }
  inline void _M_destroy() {
    pthread_mutex_destroy((pthread_mutex_t*)_M_lock);
    free((char*)_M_lock);
  }
  inline void _M_acquire_lock() { 
    pthread_mutex_lock((pthread_mutex_t*)_M_lock); 
  }
  inline void _M_release_lock() { pthread_mutex_unlock((pthread_mutex_t*)_M_lock); }
  
# elif defined (_STLP_UITHREADS)
  inline void _M_initialize() {
    _M_lock = (void*)calloc(1,sizeof(mutex_t));
    mutex_init((mutex_t*)_M_lock,0,NULL);	
  }
  inline void _M_destroy() {
    mutex_destroy((mutex_t*)_M_lock);
    free((char*)_M_lock);
  }
  inline void _M_acquire_lock() { mutex_lock((mutex_t*)_M_lock); }
  inline void _M_release_lock() { mutex_unlock((mutex_t*)_M_lock); }

#elif defined(_STLP_OS2THREADS)
  inline void _M_initialize() {
    _M_lock = (void*)calloc(1,sizeof(HMTX));
    DosCreateMutexSem(NULL, (HMTX*)_M_lock, 0, false);
  }
  inline void _M_destroy() {
    DosCloseMutexSem(*(HMTX*)_M_lock);
    free((char*)_M_lock);
  }
  inline void _M_acquire_lock() {
    if(!_M_lock) _M_initialize();
    DosRequestMutexSem(*(HMTX*)_M_lock, -1);
  }
  inline void _M_release_lock() { DosReleaseMutexSem(*(HMTX*)_M_lock); }
# else /* No threads */
  inline void _M_initialize() {}
  inline void _M_destroy() {}
  inline void _M_acquire_lock() {}
  inline void _M_release_lock() {}
# endif
};
#endif

// Locking class.  The constructor initializes the lock, the destructor destroys it.
// Well - behaving class, does not need static initializer

struct _STLP_CLASS_DECLSPEC _STLP_mutex : public _STLP_mutex_indirect {
  inline _STLP_mutex () {
    _M_initialize();
  }
  inline ~_STLP_mutex () {
    _M_destroy();
  }
private:
  _STLP_mutex(const _STLP_mutex&);
  void operator=(const _STLP_mutex&);
};

// Class _Refcount_Base provides a type, __stl_atomic_t, a data member,
// _M_ref_count, and member functions _M_incr and _M_decr, which perform
// atomic preincrement/predecrement.  The constructor initializes 
// _M_ref_count.
struct _STLP_CLASS_DECLSPEC _Refcount_Base
{
  // The data member _M_ref_count
  volatile __stl_atomic_t _M_ref_count;

# if !defined (_STLP_ATOMIC_EXCHANGE)
  _STLP_mutex _M_mutex;
# endif

  // Constructor
  _Refcount_Base(__stl_atomic_t __n) : _M_ref_count(__n) {}

  // _M_incr and _M_decr
# if defined (_STLP_THREADS) && defined ( _STLP_ATOMIC_EXCHANGE )
   void _M_incr() { _STLP_ATOMIC_INCREMENT((__stl_atomic_t*)&_M_ref_count); }
   void _M_decr() { _STLP_ATOMIC_DECREMENT((__stl_atomic_t*)&_M_ref_count); }
# elif defined(_STLP_THREADS)
  void _M_incr() {
    _M_mutex._M_acquire_lock();
    ++_M_ref_count;
    _M_mutex._M_release_lock();
  }
  void _M_decr() {
    _M_mutex._M_acquire_lock();
    --_M_ref_count;
    _M_mutex._M_release_lock();
  }
# else  /* No threads */
  void _M_incr() { ++_M_ref_count; }
  void _M_decr() { --_M_ref_count; }
# endif
};

// Atomic swap on unsigned long
// This is guaranteed to behave as though it were atomic only if all
// possibly concurrent updates use _Atomic_swap.
// In some cases the operation is emulated with a lock.
# if defined (_STLP_THREADS) && defined ( _STLP_ATOMIC_EXCHANGE )
inline __stl_atomic_t _Atomic_swap(volatile __stl_atomic_t * __p, __stl_atomic_t __q) {
  return (__stl_atomic_t) _STLP_ATOMIC_EXCHANGE(__p,__q);
}
# elif defined(_STLP_PTHREADS) || defined (_STLP_UITHREADS)  || defined (_STLP_OS2THREADS)
// We use a template here only to get a unique initialized instance.
template<int __dummy>
struct _Swap_lock_struct {
  static _STLP_STATIC_MUTEX _S_swap_lock;
};


// This should be portable, but performance is expected
// to be quite awful.  This really needs platform specific
// code.
inline __stl_atomic_t _Atomic_swap(volatile __stl_atomic_t * __p, __stl_atomic_t __q) {
  _Swap_lock_struct<0>::_S_swap_lock._M_acquire_lock();
  __stl_atomic_t __result = *__p;
  *__p = __q;
  _Swap_lock_struct<0>::_S_swap_lock._M_release_lock();
  return __result;
}
# else
/* no threads */
static inline __stl_atomic_t  _STLP_CALL
_Atomic_swap(volatile __stl_atomic_t * __p, __stl_atomic_t __q) {
  __stl_atomic_t __result = *__p;
  *__p = __q;
  return __result;
}
# endif

// A locking class that uses _STLP_STATIC_MUTEX.  The constructor takes
// a reference to an _STLP_STATIC_MUTEX, and acquires a lock.  The destructor
// releases the lock.
// It's not clear that this is exactly the right functionality.
// It will probably change in the future.

struct _STLP_CLASS_DECLSPEC _STLP_auto_lock
{
  _STLP_STATIC_MUTEX& _M_lock;
  
  _STLP_auto_lock(_STLP_STATIC_MUTEX& __lock) : _M_lock(__lock)
    { _M_lock._M_acquire_lock(); }
  ~_STLP_auto_lock() { _M_lock._M_release_lock(); }

private:
  void operator=(const _STLP_auto_lock&);
  _STLP_auto_lock(const _STLP_auto_lock&);
};

struct _STLP_CLASS_DECLSPEC _STLP_mutex_lock
{
  _STLP_mutex_indirect& _M_lock;
  
  _STLP_mutex_lock(_STLP_mutex_indirect& __lock) : _M_lock(__lock)
    { _M_lock._M_acquire_lock(); }
  ~_STLP_mutex_lock() { _M_lock._M_release_lock(); }

private:
  void operator=(const _STLP_mutex_lock&);
  _STLP_mutex_lock(const _STLP_mutex_lock&);
};

#ifdef _STLP_BETHREADS

template <int __inst>
struct _STLP_beos_static_lock_data
{
	static bool is_init;
	struct mutex_t : public _STLP_mutex
	{
		mutex_t()
		{
			_STLP_beos_static_lock_data<0>::is_init = true;
		}
		~mutex_t()
		{
			_STLP_beos_static_lock_data<0>::is_init = false;
		}
	};
	static mutex_t mut;
};

template <int __inst>
bool _STLP_beos_static_lock_data<__inst>::is_init = false;
template <int __inst>
typename _STLP_beos_static_lock_data<__inst>::mutex_t _STLP_beos_static_lock_data<__inst>::mut;


inline void _STLP_mutex_base::_M_acquire_lock() 
{
	if(sem == 0)
	{
		// we need to initialise on demand here
		// to prevent race conditions use our global
		// mutex if it's available:
		if(_STLP_beos_static_lock_data<0>::is_init)
		{
			_STLP_auto_lock al(_STLP_beos_static_lock_data<0>::mut);
			if(sem == 0) _M_initialize();
		}
		else
		{
			// no lock available, we must still be
			// in startup code, THERE MUST BE ONE THREAD
			// ONLY active at this point.
			_M_initialize();
		}
    }
	status_t t;
    t = acquire_sem(sem);
    assert(t == B_NO_ERROR);
}

#endif

_STLP_END_NAMESPACE

# if !defined (_STLP_LINK_TIME_INSTANTIATION)
#  include <stl/_threads.c>
# endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif /* _STLP_INTERNAL_THREADS_H */

// Local Variables:
// mode:C++
// End:
