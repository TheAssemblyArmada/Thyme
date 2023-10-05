/**
 * @file
 *
 * @author CCHyper
 * @author jonwil
 * @author LRFLEW
 * @author OmniBlade
 * @author Saberhawk
 *
 * @brief Hooking system for interacting with original binary.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 *
 * Originally based on work by the Tiberian Technologies team to hook Renegade, rewritten based on work by LRFLEW for
 * OpenMC2. Provides methods for accessing data and functions in an existing binary and replacing functions with new
 * implementations from an injected DLL.
 */
#pragma once

#ifndef GAME_DLL
#error This file must not be compiled into standalone binary
#endif

#include "always.h"
#include <memoryapi.h>
#include <processthreadsapi.h>

// For the few locations where a hook needs to be in the code, this macro can list all addresses and will select
// the correct one at compile time.
#ifdef BUILD_EDITOR
#define PICK_ADDRESS(a, b) (b)
#else
#define PICK_ADDRESS(a, b) (a)
#endif

// Helper Functions based on those from OpenMC2

// Use Make_Global to access global variables in the original exe once you know
// the correct type and address. This should not change from run to run if the
// exe loads at a standard base address.

// Typical use will be to use define to create a friendly name, e.g:
// #define SomeGlobalVar (Make_Global<bool>(0x00FF00FF))
// allows you to use SomeGlobalVar as though it was a bool you declared, though
// it will reflect the value the original exe sees at address 0x00FF00FF
template<typename T> __forceinline T &Make_Global(const uintptr_t address)
{
    return *reinterpret_cast<T *>(address);
}

template<typename T> __forceinline T *Make_Pointer(const uintptr_t address)
{
    return reinterpret_cast<T *>(address);
}

// Call_Function and Call_Method can be used to call functions from the original
// binary where they are required and a replacement hasn't been written yet.
template<typename ReturnType, typename... Arguments>
__forceinline ReturnType Call_Function(const uintptr_t address, Arguments... args)
{
    return reinterpret_cast<ReturnType(__cdecl *)(Arguments...)>(address)(args...);
}

template<typename ReturnType, typename... Arguments>
__forceinline ReturnType Call__StdCall_Function(const uintptr_t address, Arguments... args)
{
    return reinterpret_cast<ReturnType(__stdcall *)(Arguments...)>(address)(args...);
}

template<typename ReturnType, typename ThisType, typename... Arguments>
__forceinline ReturnType Call_Method(const uintptr_t address, ThisType *const self, Arguments... args)
{
    return reinterpret_cast<ReturnType(__thiscall *)(ThisType *, Arguments...)>(address)(self, args...);
}

// These create pointers to various types of function where the return,
// parameters and calling convention are known.
template<typename ReturnType, typename... Arguments>
__forceinline ReturnType(__cdecl *Make_Function_Ptr(const uintptr_t address))(Arguments...)
{
    return reinterpret_cast<ReturnType(__cdecl *)(Arguments...)>(address);
}

template<typename ReturnType, typename... Arguments>
__forceinline ReturnType(__stdcall *Make_StdCall_Ptr(const uintptr_t address))(Arguments...)
{
    return reinterpret_cast<ReturnType(__stdcall *)(Arguments...)>(address);
}

template<typename ReturnType, typename ThisType, typename... Arguments>
__forceinline ReturnType(__thiscall *Make_Method_Ptr(const uintptr_t address))(ThisType *, Arguments...)
{
    return reinterpret_cast<ReturnType(__thiscall *)(ThisType *, Arguments...)>(address);
}

template<typename ReturnType, typename... Arguments>
__forceinline ReturnType(__cdecl *Make_VA_Function_Ptr(const uintptr_t address))(Arguments..., ...)
{
    return reinterpret_cast<ReturnType(__cdecl *)(Arguments..., ...)>(address);
}

// A nice struct to pack the assembly in for jumping into replacement code.
// So long as the calling conventions and arguments for the replaced and
// replacement functions are the same, everything should just work.
#pragma pack(push, 1)
struct x86_jump
{
    const uint8_t cmd = 0xE9;
    uintptr_t addr;
};
#pragma pack(pop)

// Use these to hook existing functions and replace them with newly written ones
// to either replace them permanently or just to test correctness of a newly
// written one.

// Base hooking function to apply the actual hook.
inline void Hook_Func(uintptr_t in, uintptr_t out)
{
#ifdef GAME_DLL
    static_assert(sizeof(x86_jump) == 5, "Jump struct not expected size.");

    x86_jump cmd;
    cmd.addr = out - in - 5;
    WriteProcessMemory(GetCurrentProcess(), (LPVOID)in, &cmd, 5, nullptr);
#endif
}

template<typename T> void Hook_Memory(uintptr_t in, T out)
{
    WriteProcessMemory(GetCurrentProcess(), (LPVOID)in, &out, sizeof(T), nullptr);
}

// Hook regular functions and static methods.
template<typename T> void Hook_Function(uintptr_t in, T out)
{
    Hook_Func(in, reinterpret_cast<uintptr_t>(out));
}

// Method pointers need funky syntax to get the underlying function address
// hence the odd cast to void for the out pointer.
template<typename T> void Hook_Method(uintptr_t in, T out)
{
    Hook_Func(in, reinterpret_cast<uintptr_t>((void *&)out));
}

// Virtuals are even trickier so resort to inline assembler for those courtesy of the TTScripts guys.
#define Hook_Any(in, out) __asm { \
      __asm push out                  \
      __asm push in                         \
      __asm call Hook_Func     \
      __asm add esp, 8 }

#define ARRAY_DEC(type, var, size) type(&var)[size]
#define ARRAY_DEF(address, type, var, size) type(&var)[size] = Make_Global<type[size]>(address);
#define ARRAY2D_DEC(type, var, x, y) type(&var)[x][y]
#define ARRAY2D_DEF(address, type, var, x, y) type(&var)[x][y] = Make_Global<type[x][y]>(address);
