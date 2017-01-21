////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: GAMETEXT.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: String file handler.
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
#ifdef _MSC_VER
#pragma once
#endif // _MSC_VER

#ifndef _GAMETEXT_H_
#define _GAMETEXT_H_

#include "subsysteminterface.h"
#include "unicodestring.h"
#include "asciistring.h"

struct CSFHeader
{
    int32_t ID;
    int32_t Version;
    int32_t NumLabels;
    int32_t NumStrings;
    int32_t Skip;
    int32_t LangID;
};

class GameTextInterface : public SubsystemInterface
{
    public:
        GameTextInterface();
        virtual ~GameTextInterface() {}

        virtual UnicodeString Fetch(const char *args, bool *success);
        virtual UnicodeString Fetch(AsciiString args, bool *success);
        virtual std::vector<AsciiString> Get_Strings_With_Prefix(AsciiString label);
        virtual void Init_Map_String_File(AsciiString const &filename);
};

class GameTextManager : public GameTextInterface
{

};

#endif // _GAMETEXT_H_
