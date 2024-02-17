/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Wrapper to mask STLPort ABI differences for config loader and crashpad init code.
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

class CrashPreferences;

class CrashPrefWrapper
{
public:
    CrashPrefWrapper();
    ~CrashPrefWrapper();

    const char *Get_Upload_URL();
    bool Upload_Allowed();

private:
    CrashPreferences *m_prefs;
};
