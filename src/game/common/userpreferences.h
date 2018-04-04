////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: USERPREFERENCES.H
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
#pragma once

#include "asciistring.h"
#include <map>

class UserPreferences : public std::map<AsciiString, AsciiString>
{
public:
    UserPreferences();
    virtual ~UserPreferences() {}

    virtual bool Load(AsciiString filename);
    virtual bool Write();

    AsciiString Get_AsciiString(AsciiString key, AsciiString def_arg = AsciiString::s_emptyString);
    int Get_Int(AsciiString key, int def_arg);
    float Get_Real(AsciiString key, float def_arg);
    bool Get_Bool(AsciiString key, bool def_arg);
    void Set_AsciiString(AsciiString key, AsciiString value);
    void Set_Int(AsciiString key, int value);
    void Set_Real(AsciiString key, float value);
    void Set_Bool(AsciiString key, bool value);

private:
    AsciiString m_filename;
};