////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: USERPREFERENCES.CPP
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Class holding user customisable preferences.
//
//       License:: Thyme is free software: you can redistribute it and/or 
//                 modify it under the terms of the GNU General Public License 
//                 as published by the Free Software Foundation, either version 
//                 2 of the License, or (at your option) any later version.
//
//                 A full copy of the GNU General Public License can be found in
//                 LICENSE
//
////////////////////////////////////////////////////////////////////////////////
#include "userpreferences.h"
#include "globaldata.h"
#include <cstdio>

UserPreferences::UserPreferences()
{
}

bool UserPreferences::Load(AsciiString filename)
{
    m_filename = g_theWriteableGlobalData->m_userDataDirectory;
    m_filename += filename;

    FILE *fptr = fopen(m_filename.Str(), "r");

    if ( fptr != nullptr ) {
        char buffer[2048];

        while ( fgets(buffer, sizeof(buffer), fptr) != 0 ) {
            // Get key/value pair per line.
            AsciiString key;
            AsciiString line = buffer;
            line.Trim();
            line.Next_Token(&key, "=");
            AsciiString value = line.Str() + 1;

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
    if ( m_filename.Is_Empty() ) {
        return false;
    }

    FILE *fp = fopen(m_filename.Str(), "w");

    if ( fp != nullptr ) {
        for ( auto it = begin(); it != end(); ++it ) {
            fprintf(fp, "%s = %s\n", it->first.Str(), it->second.Str());
        }

        fclose(fp);

        return true;
    }

    return false;
}

AsciiString UserPreferences::Get_AsciiString(AsciiString key, AsciiString def_arg)
{
    auto it = find(key);

    if ( it == end() ) {
        return def_arg;
    }

    return it->second;
}

int UserPreferences::Get_Int(AsciiString key, int def_arg)
{
    AsciiString value = Get_AsciiString(key);

    if ( value.Is_Empty() ) {
        return def_arg;
    }

    return atoi(value.Str());
}

float UserPreferences::Get_Real(AsciiString key, float def_arg)
{
    AsciiString value = Get_AsciiString(key);

    if ( value.Is_Empty() ) {
        return def_arg;
    }

    return (float)atof(value.Str());
}

bool UserPreferences::Get_Bool(AsciiString key, bool def_arg)
{
    AsciiString value = Get_AsciiString(key);

    if ( value.Is_Empty() ) {
        return def_arg;
    }

    value.To_Lower();
    const char *val_c = value.Str();

    if ( strcmp(val_c, "1") != 0
        && strcmp(val_c, "t") != 0
        && strcmp(val_c, "true") != 0
        && strcmp(val_c, "y") != 0
        && strcmp(val_c, "yes") != 0
        && strcmp(val_c, "ok") != 0
        ) {
        return false;
    }

    return true;
}

void UserPreferences::Set_AsciiString(AsciiString key, AsciiString value)
{
    (*this)[key] = value;
}

void UserPreferences::Set_Int(AsciiString key, int value)
{
    AsciiString val;
    val.Format("%d", value);
    (*this)[key] = val;
}

void UserPreferences::Set_Real(AsciiString key, float value)
{
    AsciiString val;
    val.Format("%g", value);
    (*this)[key] = val;
}

void UserPreferences::Set_Bool(AsciiString key, bool value)
{
    AsciiString val;
    val.Format("%d", value);
    (*this)[key] = val;
}
