/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Wrapper to mask ABI differences for config loader and crashpad init code.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "crashwrapper.h"
#include "crashpreferences.h"
#include <captainslog.h>

CrashPrefWrapper::CrashPrefWrapper() : m_prefs(new CrashPreferences) {}

CrashPrefWrapper::~CrashPrefWrapper()
{
    delete m_prefs;
}

const char *CrashPrefWrapper::Get_Upload_URL()
{
    static char path[512];
    strlcpy_tpl(path, m_prefs->Get_Upload_URL().Str());
    return path;
}

bool CrashPrefWrapper::Upload_Allowed()
{
    return m_prefs->Upload_Allowed();
}
