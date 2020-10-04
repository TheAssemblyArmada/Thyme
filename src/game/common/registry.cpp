/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Recovers configuration information.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */

#include "registry.h"
#include <captainslog.h>

#ifdef PLATFORM_WINDOWS
#include <winreg.h>

static bool getStringFromReg(HKEY hkey, Utf8String subkey, Utf8String key, Utf8String &result)
{
    HKEY phk_result;
    if (RegOpenKeyExA(hkey, subkey, 0, KEY_READ, &phk_result) == ERROR_SUCCESS) {
        BYTE data[256];
        DWORD data_len = 256;
        DWORD type;
        LSTATUS query_result = RegQueryValueExA(phk_result, key, 0, &type, data, &data_len);
        RegCloseKey(phk_result);

        if (query_result == ERROR_SUCCESS) {
            result = reinterpret_cast<char *>(data);

            return true;
        }
    }

    return false;
}
#endif

Utf8String Get_Registry_Language()
{
    static Utf8String lang = "english";
    static bool retrieved = false;

    if (retrieved) {
        return lang;
    } else {
        Get_String_From_Registry("", "Language", lang);
        retrieved = true;

        return lang;
    }
}

bool Get_String_From_Registry(Utf8String subkey, Utf8String value, Utf8String &destination)
{
#ifdef PLATFORM_WINDOWS
    Utf8String key = "SOFTWARE\\Electronic Arts\\EA Games\\Command and Conquer Generals Zero Hour";
    key += subkey;
    captainslog_trace("Get_String_From_Registry - looking in %s for key %s\n", key.Str(), value.Str());
    bool success = getStringFromReg(HKEY_LOCAL_MACHINE, key, value, destination);

    if (!success) {
        return getStringFromReg(HKEY_CURRENT_USER, key, value, destination);
    }

    return false;
#else
    return false;
#endif
}

bool Get_String_From_Generals_Registry(Utf8String subkey, Utf8String value, Utf8String &destination)
{
#ifdef PLATFORM_WINDOWS
    Utf8String key = "SOFTWARE\\Electronic Arts\\EA Games\\Generals";
    key += subkey;
    captainslog_trace("Get_String_From_Generals_Registry - looking in %s for key %s\n", key.Str(), value.Str());
    bool success = getStringFromReg(HKEY_LOCAL_MACHINE, key, value, destination);

    if (!success) {
        return getStringFromReg(HKEY_CURRENT_USER, key, value, destination);
    }

    return false;
#else
    return false;
#endif
}