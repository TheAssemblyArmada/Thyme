/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Class holding preferences for crash handling and uploading.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "crashpreferences.h"
#include <captainslog.h>
#include <cstdio>
#include <cstdlib>

using std::fclose;
using std::fgets;
using std::fopen;
using std::getenv;

CrashPreferences::CrashPreferences()
{
    captainslog_debug("Creating crash preference object.");
    Load("CrashLog.ini");
}

bool CrashPreferences::Load(Utf8String filename)
{
    // Overriding normal loading function to allow getting path earlier in program startup.
    // TODO replace the globaldata user path entirely with a new system for getting various paths.
#ifdef PLATFORM_WINDOWS
    m_filename = getenv("USERPROFILE");
#else
    m_filename = getenv("HOME");
#endif
    m_filename += "/Documents/Command and Conquer Generals Zero Hour Data/";
    m_filename += filename;
    FILE *fptr = fopen(m_filename.Str(), "r");

    if (fptr != nullptr) {
        char buffer[2048];

        while (fgets(buffer, sizeof(buffer), fptr) != 0) {
            // Get key/value pair per line.
            Utf8String key;
            Utf8String line = buffer;
            line.Trim();
            line.Next_Token(&key, "=");
            Utf8String value = line.Str() + 1;

            // Trim whitespace.
            key.Trim();
            value.Trim();

            // Insert into map.
            (*this)[key] = value;
        }

        fclose(fptr);

        return true;
    }

    return false;
}

Utf8String CrashPreferences::Get_Upload_URL()
{
    return Get_AsciiString("UploadURL", "");
}

bool CrashPreferences::Upload_Allowed()
{
    return Get_Bool("UploadCrashDumps", false);
}

void CrashPreferences::Set_Upload_URL(const Utf8String &url)
{
    Set_AsciiString("UploadURL", url);
}

void CrashPreferences::Set_Allow_Upload(bool allowed)
{
    Set_AsciiString("UploadCrashDumps", allowed ? "yes" : "no");
}
