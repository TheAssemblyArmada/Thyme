////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: VERSION.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Stores information about the current build.
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

#ifndef _VERSION_H_
#define _VERSION_H_

#include "asciistring.h"
#include "unicodestring.h"

#define TheVersion (Make_Global<Version *>(0x00A29BA0))

class Version
{
    public:
        Version();

        void Set_Version(
            int maj, int min, int build, int local_build, 
            AsciiString location, AsciiString user, AsciiString time, AsciiString date);
        
        int Get_Version_Number() { return Minor | (Major << 16); }
        int Get_Build_Number() { return BuildNum; }
        int Get_Local_Build_Number() { return LocalBuildNum; }

        AsciiString Get_Ascii_Version();
        AsciiString Get_Ascii_Build_Location() { return BuildLocation; }
        AsciiString Get_Ascii_Build_User() { return BuildUser; }
        AsciiString Get_Ascii_Build_Time();

        // TODO unicode implementation rely on GameTextManager
        UnicodeString Get_Unicode_Version();
        UnicodeString Get_Full_Unicode_Version();
        UnicodeString Get_Unicode_Build_Location();
        UnicodeString Get_Unicode_Build_User();
        UnicodeString Get_Unicode_Build_Time();

        // Some functions check this bool to decide what format the
        // printed version string should take.
        void Set_Use_Full_Version(bool cmd_line) { UseFullVersion = cmd_line; }
        bool Using_Full_Version() { return UseFullVersion; }

    private:
        int Major;
        int Minor;
        int BuildNum;
        int LocalBuildNum;
        AsciiString BuildLocation;  // Using this as Git Branch
        AsciiString BuildUser;      // Using this as Short Git SHA1
        AsciiString BuildTime;
        AsciiString BuildDate;
        bool UseFullVersion;
};

#endif // _VERSION_H_