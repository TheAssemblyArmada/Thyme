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
#pragma once

#include "always.h"
#include "asciistring.h"
#include <map>

class UserPreferences : public std::map<Utf8String, Utf8String>
{
public:
    UserPreferences();
    virtual ~UserPreferences() {}

    virtual bool Load(Utf8String filename);
    virtual bool Write();

    Utf8String Get_AsciiString(Utf8String key, Utf8String def_arg = Utf8String::s_emptyString) const;
    int Get_Int(Utf8String key, int def_arg) const;
    float Get_Real(Utf8String key, float def_arg) const;
    bool Get_Bool(Utf8String key, bool def_arg) const;
    void Set_AsciiString(Utf8String key, Utf8String value);
    void Set_Int(Utf8String key, int value);
    void Set_Real(Utf8String key, float value);
    void Set_Bool(Utf8String key, bool value);

protected:
    Utf8String m_filename;
};