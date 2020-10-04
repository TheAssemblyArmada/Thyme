/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Stub functions to remove launcher and cd check requirement.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "copyprotect.h"

bool CopyProtect::isLauncherRunning()
{
    return true;
}

bool CopyProtect::notifyLauncher()
{
    return true;
}

void CopyProtect::checkForMessage(unsigned int msg, int handle) {}

bool CopyProtect::validate()
{
    return true;
}

void *CopyProtect::shutdown()
{
    return nullptr;
}
