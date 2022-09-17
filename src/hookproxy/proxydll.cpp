/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Proxy dll to maquerade as bandtest.dll and inject our hooking reimplementation at the same time.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "hooker.h"
#include "macros.h"
#include "sha1.h"
#include <cstdio>
#include <cstdlib>
#include <io.h>
#include <fstream>
#include <windows.h>

/**
 * Pointer for the hooking function from the payload.
 */
void (*Setup_Hooks)();

/**
 * Functions we will export that will jump to the original dll functions that we are proxying for injection.
 * avifil32.dll in this case.
 */
FARPROC OriginalAVIStreamWrite;
FARPROC OriginalAVIFileInit;
FARPROC OriginalAVIFileOpen;
FARPROC OriginalAVIFileOpenA;
FARPROC OriginalAVIFileReadData;
FARPROC OriginalAVIFileRelease;
FARPROC OriginalAVIFileExit;
FARPROC OriginalAVIFileCreateStreamA;
FARPROC OriginalAVIStreamSetFormat;
FARPROC OriginalAVIStreamRelease;
__declspec(naked) void FakeAVIStreamWrite()
{
    _asm { jmp[OriginalAVIStreamWrite] }
}
__declspec(naked) void FakeAVIFileInit()
{
    _asm { jmp[OriginalAVIFileInit] }
}
__declspec(naked) void FakeAVIFileOpen()
{
    _asm { jmp[OriginalAVIFileOpen] }
}
__declspec(naked) void FakeAVIFileOpenA()
{
    _asm { jmp[OriginalAVIFileOpenA] }
}
__declspec(naked) void FakeAVIFileReadData()
{
    _asm { jmp[OriginalAVIFileReadData] }
}
__declspec(naked) void FakeAVIFileRelease()
{
    _asm { jmp[OriginalAVIFileRelease] }
}
__declspec(naked) void FakeAVIFileExit()
{
    _asm { jmp[OriginalAVIFileExit] }
}
__declspec(naked) void FakeAVIFileCreateStreamA()
{
    _asm { jmp[OriginalAVIFileCreateStreamA] }
}
__declspec(naked) void FakeAVIStreamSetFormat()
{
    _asm { jmp[OriginalAVIStreamSetFormat] }
}
__declspec(naked) void FakeAVIStreamRelease()
{
    _asm { jmp[OriginalAVIStreamRelease] }
}

/**
 * Get the path for the exe we were loaded by.
 */
const wchar_t *Get_Path()
{
    static wchar_t buffer[MAX_PATH];
    DWORD size = GetModuleFileNameW(nullptr, buffer, ARRAY_SIZE(buffer));

    if (size == 0 || size >= ARRAY_SIZE(buffer)) {
        MessageBoxA(nullptr, "Failed to get path to target executable for code injection", "Proxy", MB_ICONERROR);
        ExitProcess(0);
    }

    return buffer;
}

/**
 * Get the hash for the exe we were loaded by.
 */
const char* Get_Hash()
{
    SHAEngine sha;
    char buffer[1024];

    std::ifstream in(Get_Path(), std::ios_base::in | std::ios_base::binary);

    // Get the hash of the binary.
    do {
        in.read(buffer, sizeof(buffer));
        sha.Hash(buffer, int(in.gcount()));
    } while (in.gcount() == sizeof(buffer));

    return sha.Print_Result();
}

/**
 * Get the payload dll name we expect for the binary based on its hash.
 */
const char* Get_Payload_Name()
{
    struct ExePayloads
    {
        const char *exe_hash;
        const char *payload_name;
    };
    
    static const ExePayloads Payloads[] = {
        { "5b805b4ecba79581fe8ec454b5acf7b7e13d4dd5", "thyme.dll" },
        { "3c1a1d431eae665978c9406f05c427252bc4e768", "thymeedit.dll" },
    };

    const char *target_hash = Get_Hash();

    for (unsigned i = 0; i < ARRAY_SIZE(Payloads); ++i) {
        if (strcmp(Payloads[i].exe_hash, target_hash) == 0) {
            return Payloads[i].payload_name;
        }
    }

    return nullptr;
}

/**
 * Loads the dll we are proxying for and the functions the target application imports.
 */
void Load_Forwarded_Functions()
{
    static const char dll_name[] = "\\avifil32.dll";
    char path[MAX_PATH];
    std::memcpy(path + GetSystemDirectoryA(path, MAX_PATH - sizeof(dll_name)), dll_name, sizeof(dll_name));
    HMODULE dll = LoadLibraryA(path);
    if (dll == nullptr) {
        MessageBoxA(nullptr, "Cannot load original avifil32.dll library", "Proxy", MB_ICONERROR);
        ExitProcess(0);
    } else {
        OriginalAVIStreamWrite = GetProcAddress(dll, "AVIStreamWrite");
        OriginalAVIFileInit = GetProcAddress(dll, "AVIFileInit");
        OriginalAVIFileOpen = GetProcAddress(dll, "AVIFileOpen");
        OriginalAVIFileOpenA = GetProcAddress(dll, "AVIFileOpenA");
        OriginalAVIFileReadData = GetProcAddress(dll, "AVIFileReadData");
        OriginalAVIFileRelease = GetProcAddress(dll, "AVIFileRelease");
        OriginalAVIFileExit = GetProcAddress(dll, "AVIFileExit");
        OriginalAVIFileCreateStreamA = GetProcAddress(dll, "AVIFileCreateStreamA");
        OriginalAVIStreamSetFormat = GetProcAddress(dll, "AVIStreamSetFormat");
        OriginalAVIStreamRelease = GetProcAddress(dll, "AVIStreamRelease");
    }
}

/**
 * Loads the dll payload and retrieves the Setup_Hooks function from it.
 */
bool Load_Setup_Hooks()
{
    const char *payload = Get_Payload_Name();

    if (payload == nullptr) {
#ifdef GAME_DEBUG
        MessageBoxA(nullptr, "Cannot find the patch dll for this binary, thyme will not be active.", "Proxy", MB_ICONERROR);
#endif
        return false;
    }

    HMODULE dll = LoadLibraryA(payload);

    if (dll == nullptr) {
#ifdef GAME_DEBUG
        MessageBoxA(nullptr, "Cannot load patch library", "Proxy", MB_ICONERROR);
#endif
        return false;
    }
    
    Setup_Hooks = (void (*)())GetProcAddress(dll, "Setup_Hooks");

    if (Setup_Hooks == nullptr) {
#ifdef GAME_DEBUG
        MessageBoxA(nullptr, "Cannot load Setup_Hooks() function from the patch library", "Proxy", MB_ICONERROR);
#endif
        return false;
    }

    return true;
}

/**
 * Our proxy dll entry point, we load the dll we are proxying as well as set up our injection payload on process attach.
 */
BOOL WINAPI DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            // So we can handle calls to the functions we pretend to have.
            Load_Forwarded_Functions();

            // Now we work out which binary we have and load the payload dll if we have it.
            if (Load_Setup_Hooks()) {
                if (StartHooks()) {
                    Setup_Hooks();
                    StopHooks();
                }
            }
            break;

        case DLL_PROCESS_DETACH:
            // This DLL is supposed to be loaded via imports in an exe and stay loaded for the life of the process.
            // As a result we don't clean up after ourselves regarding calling FreeLibray which isn't best practice.
            // However the documentation for LoadLibrary states that the libraries loaded with it will unload on
            // process termination which is the behaviour we want anyhow.
            break;

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        default:
            break;
    }

    return TRUE;
}
