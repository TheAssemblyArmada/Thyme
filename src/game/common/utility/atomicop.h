/**
 * @file
 *
 * @author xezon
 *
 * @brief Simple atomic integer operations. (Thyme Feature)
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

// Usefulness of atomic operations is limited cross platform. Be aware that integer types can be either signed or unsigned,
// so conversion must take place manually. Prefer using 32 bit type for best compatibility or std::atomic<> when available.

// clang-format off

#if defined(__linux__)
using AtomicType16 = int16_t;
using AtomicType32 = int32_t;
using AtomicType64 = int64_t;

inline AtomicType16 Atomic_Increment_16(volatile AtomicType16 *addend) { return __sync_fetch_and_add(addend, AtomicType16{ 1 }); }
inline AtomicType32 Atomic_Increment_32(volatile AtomicType32 *addend) { return __sync_fetch_and_add(addend, AtomicType32{ 1 }); }
inline AtomicType64 Atomic_Increment_64(volatile AtomicType64 *addend) { return __sync_fetch_and_add(addend, AtomicType64{ 1 }); }

inline AtomicType16 Atomic_Decrement_16(volatile AtomicType16 *addend) { return __sync_fetch_and_sub(addend, AtomicType16{ 1 }); }
inline AtomicType32 Atomic_Decrement_32(volatile AtomicType32 *addend) { return __sync_fetch_and_sub(addend, AtomicType32{ 1 }); }
inline AtomicType64 Atomic_Decrement_64(volatile AtomicType64 *addend) { return __sync_fetch_and_sub(addend, AtomicType64{ 1 }); }

#elif defined(__APPLE__) && defined(__MACH__)
#include <OSAtomic.h>
using AtomicType16 = SInt16;
using AtomicType32 = SInt32;
using AtomicType64 = SInt64;

inline AtomicType16 Atomic_Increment_16(volatile AtomicType16 *addend) { return OSIncrementAtomic16(addend); }
inline AtomicType32 Atomic_Increment_32(volatile AtomicType32 *addend) { return OSIncrementAtomic(addend); }
inline AtomicType64 Atomic_Increment_64(volatile AtomicType64 *addend) { return OSIncrementAtomic64(addend); }

inline AtomicType16 Atomic_Increment_16(volatile AtomicType16 *addend) { return OSDecrementAtomic16(addend); }
inline AtomicType32 Atomic_Increment_32(volatile AtomicType32 *addend) { return OSDecrementAtomic(addend); }
inline AtomicType64 Atomic_Increment_64(volatile AtomicType64 *addend) { return OSDecrementAtomic64(addend); }

#elif defined(__OpenBSD__)
#include <sys/atomic.h>
// Appears to have no native atomic operations for 16 bit integers.
using AtomicType16 = unsigned short;
using AtomicType32 = unsigned int;
using AtomicType64 = unsigned long; // is 64 bit on 64 bit architecture

inline AtomicType32 Atomic_Increment_32(volatile AtomicType32 *addend) { return atomic_inc_int_nv(addend); }
inline AtomicType64 Atomic_Increment_64(volatile AtomicType64 *addend) { return atomic_inc_long_nv(addend); }

inline AtomicType32 Atomic_Increment_32(volatile AtomicType32 *addend) { return atomic_dec_int_nv(addend); }
inline AtomicType64 Atomic_Increment_64(volatile AtomicType64 *addend) { return atomic_dec_long_nv(addend); }

#elif defined(__FreeBSD__) || defined(__DragonFly__)
#include <machine/atomic.h>
// Appears to have no native atomic operations for 16, 64 bit integers.
using AtomicType16 = uint16_t;
using AtomicType32 = uint32_t;
using AtomicType64 = uint64_t;

inline AtomicType32 Atomic_Increment_32(volatile AtomicType32 *addend) { return atomic_fetchadd_32(addend, AtomicType32{ 1 }); }

inline AtomicType32 Atomic_Decrement_32(volatile AtomicType32 *addend) { return atomic_fetchadd_32(addend, ~AtomicType32{ 0 }); }

#elif defined(__NetBSD__)
#include <sys/atomic.h>
// Appears to have no native atomic operations for 16 bit integers.
using AtomicType16 = uint16_t;
using AtomicType32 = uint32_t;
using AtomicType64 = uint64_t;

inline AtomicType32 Atomic_Increment_32(volatile AtomicType32 *addend) { return atomic_inc_32_nv(addend); }
inline AtomicType64 Atomic_Increment_64(volatile AtomicType64 *addend) { return atomic_inc_64_nv(addend); }

inline AtomicType32 Atomic_Decrement_32(volatile AtomicType32 *addend) { return atomic_dec_32_nv(addend); }
inline AtomicType32 Atomic_Decrement_64(volatile AtomicType64 *addend) { return atomic_dec_64_nv(addend); }

#elif defined(_WIN32)
#include <windows.h>
using AtomicType16 = short;
using AtomicType32 = long;
using AtomicType64 = __int64;

inline AtomicType16 Atomic_Increment_16(volatile AtomicType16 *addend) { return InterlockedIncrement16(addend); }
inline AtomicType32 Atomic_Increment_32(volatile AtomicType32 *addend) { return InterlockedIncrement(addend); }
inline AtomicType64 Atomic_Increment_64(volatile AtomicType64 *addend) { return InterlockedIncrement64(addend); }

inline AtomicType16 Atomic_Decrement_16(volatile AtomicType16 *addend) { return InterlockedDecrement16(addend); }
inline AtomicType32 Atomic_Decrement_32(volatile AtomicType32 *addend) { return InterlockedDecrement(addend); }
inline AtomicType64 Atomic_Decrement_64(volatile AtomicType64 *addend) { return InterlockedDecrement64(addend); }

#else
#error platform not supported
#endif

template<typename Type> inline Type Atomic_Increment(volatile Type *addend);
template<> inline AtomicType16 Atomic_Increment<AtomicType16>(volatile AtomicType16 *addend) { return Atomic_Increment_16(addend); }
template<> inline AtomicType32 Atomic_Increment<AtomicType32>(volatile AtomicType32 *addend) { return Atomic_Increment_32(addend); }
template<> inline AtomicType64 Atomic_Increment<AtomicType64>(volatile AtomicType64 *addend) { return Atomic_Increment_64(addend); }

template<typename Type> inline Type Atomic_Decrement(volatile Type *addend);
template<> inline AtomicType16 Atomic_Decrement<AtomicType16>(volatile AtomicType16 *addend) { return Atomic_Decrement_16(addend); }
template<> inline AtomicType32 Atomic_Decrement<AtomicType32>(volatile AtomicType32 *addend) { return Atomic_Decrement_32(addend); }
template<> inline AtomicType64 Atomic_Decrement<AtomicType64>(volatile AtomicType64 *addend) { return Atomic_Decrement_64(addend); }

// clang-format on
