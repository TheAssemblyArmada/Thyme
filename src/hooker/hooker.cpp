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

DWORD OldProtect = 0;

bool StartHooking()
{
    bool success = false;
    ImageSectionInfo info;

    if (GetModuleSectionInfo(info)) {
        HANDLE process = GetCurrentProcess();
        success =
            VirtualProtectEx(process, (LPVOID)info.BaseOfCode, (SIZE_T)info.SizeOfCode, PAGE_EXECUTE_READWRITE, &OldProtect)
            != FALSE;
    }

    captainslog_dbgassert(success, "Unable to lift code page protection");
    return success;
}

bool StopHooking()
{
    bool success = false;
    DWORD OldProtect2;
    ImageSectionInfo info;

    if (GetModuleSectionInfo(info)) {
        HANDLE process = GetCurrentProcess();
        success =
            VirtualProtectEx(process, (LPVOID)info.BaseOfCode, (SIZE_T)info.SizeOfCode, OldProtect, &OldProtect2) != FALSE;
    }

    captainslog_dbgassert(success, "Unable to restore code page protection");
    return success;
}
