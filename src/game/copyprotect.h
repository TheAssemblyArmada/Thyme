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
#pragma once

class CopyProtect
{
public:
    static bool isLauncherRunning(); // 0x004122F0
    static bool notifyLauncher(); // 0x00412330
    static void checkForMessage(unsigned int msg, int handle); // 0x00412420
    static bool validate(); // 0x00412450
    static void *shutdown(); // 0x004124B0
private:
    //
    // Original used a mutex and void* to communicate with the launcher.
    //
};