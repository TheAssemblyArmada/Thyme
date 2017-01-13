////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: VERSION.CPP
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

#include "version.h"

Version::Version() :
    Major(1),
    Minor(0),
    BuildNum(0),
    LocalBuildNum(0),
    BuildLocation("somewhere"),
    BuildUser("someone"),
    BuildDate(),
    BuildTime(),
    UseFullVersion(false)
{

}

void Version::Set_Version(
    int maj, int min, int build, int local_build, 
    AsciiString location, AsciiString user, AsciiString time, AsciiString date ) 
{
    Major = maj;
    Minor = min;
    BuildNum = build;
    LocalBuildNum = local_build;
    BuildLocation = location;
    BuildUser = user;
    BuildTime = time;
    BuildDate = date;
}

AsciiString Version::Get_Ascii_Version()
{
    AsciiString version;

    if ( LocalBuildNum != 0 ) {
        version.Format("%d.%d.%d.%d", Major, Minor, BuildNum, LocalBuildNum);
    } else {
        version.Format("%d.%d.%d", Major, Minor, BuildNum);
    }

    return version;
}

AsciiString Version::Get_Ascii_Build_Time()
{
    AsciiString version;

    version.Format("%s %s", BuildDate.Str(), BuildTime.Str());

    return version;
}
