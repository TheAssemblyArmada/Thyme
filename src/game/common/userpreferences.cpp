/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Class holding user customisable preferences.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "userpreferences.h"
#include "globaldata.h"
#include <cstdio>

UserPreferences::UserPreferences() {}

bool UserPreferences::Load(Utf8String filename)
{
    m_filename = g_theWriteableGlobalData->m_userDataDirectory;
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

bool UserPreferences::Write()
{
    if (m_filename.Is_Empty()) {
        return false;
    }

    FILE *fp = fopen(m_filename.Str(), "w");

    if (fp != nullptr) {
        for (auto it = begin(); it != end(); ++it) {
            fprintf(fp, "%s = %s\n", it->first.Str(), it->second.Str());
        }

        fclose(fp);

        return true;
    }

    return false;
}

Utf8String UserPreferences::Get_AsciiString(Utf8String key, Utf8String def_arg) const
{
    auto it = find(key);

    if (it == end()) {
        return def_arg;
    }

    return it->second;
}

int UserPreferences::Get_Int(Utf8String key, int def_arg) const
{
    Utf8String value = Get_AsciiString(key);

    if (value.Is_Empty()) {
        return def_arg;
    }

    return atoi(value.Str());
}

float UserPreferences::Get_Real(Utf8String key, float def_arg) const
{
    Utf8String value = Get_AsciiString(key);

    if (value.Is_Empty()) {
        return def_arg;
    }

    return (float)atof(value.Str());
}

bool UserPreferences::Get_Bool(Utf8String key, bool def_arg) const
{
    Utf8String value = Get_AsciiString(key);

    if (value.Is_Empty()) {
        return def_arg;
    }

    value.To_Lower();
    const char *val_c = value.Str();

    if (strcmp(val_c, "1") != 0 && strcmp(val_c, "t") != 0 && strcmp(val_c, "true") != 0 && strcmp(val_c, "y") != 0
        && strcmp(val_c, "yes") != 0 && strcmp(val_c, "ok") != 0) {
        return false;
    }

    return true;
}

void UserPreferences::Set_AsciiString(Utf8String key, Utf8String value)
{
    (*this)[key] = value;
}

void UserPreferences::Set_Int(Utf8String key, int value)
{
    Utf8String val;
    val.Format("%d", value);
    (*this)[key] = val;
}

void UserPreferences::Set_Real(Utf8String key, float value)
{
    Utf8String val;
    val.Format("%g", value);
    (*this)[key] = val;
}

void UserPreferences::Set_Bool(Utf8String key, bool value)
{
    Utf8String val;
    val.Format("%d", value);
    (*this)[key] = val;
}
