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
#include "hooker.h"
#include "captainslog.h"
#include "mapview.h"

DWORD s_oldProtect1 = 0;
DWORD s_oldProtect2 = 0;

void StartHooking()
{
    StartHooks();
}
void StopHooking()
{
    StopHooks();
}

bool StartHooks()
{
    bool success = false;
    ImageSectionInfo info;

    if (GetModuleSectionInfo(info)) {
        success = true;
        HANDLE process = GetCurrentProcess();
        if (VirtualProtectEx(process, info.BaseOfCode, info.SizeOfCode, PAGE_EXECUTE_READWRITE, &s_oldProtect1) == FALSE)
            success = false;
        if (VirtualProtectEx(process, info.BaseOfData, info.SizeOfData, PAGE_EXECUTE_READWRITE, &s_oldProtect2) == FALSE)
            success = false;
    }

    captainslog_dbgassert(success, "Unable to lift code page protection");
    return success;
}

bool StopHooks()
{
    bool success = false;
    DWORD old_protect;
    ImageSectionInfo info;

    if (GetModuleSectionInfo(info)) {
        success = true;
        HANDLE process = GetCurrentProcess();
        if (VirtualProtectEx(process, info.BaseOfCode, info.SizeOfCode, s_oldProtect1, &old_protect) == FALSE)
            success = false;
        if (VirtualProtectEx(process, info.BaseOfData, info.SizeOfData, s_oldProtect2, &old_protect) == FALSE)
            success = false;
    }

    captainslog_dbgassert(success, "Unable to restore code page protection");
    return success;
}
