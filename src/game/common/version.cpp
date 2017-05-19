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
#include "gametext.h"

Version::Version() :
    m_major(1),
    m_minor(0),
    m_buildNum(0),
    m_localBuildNum(0),
    m_branch("somewhere"),
    m_commitHash("someone"),
    m_buildDate(),
    m_buildTime(),
    m_useFullVersion(false)
{

}

void Version::Set_Version(
    int32_t maj, int32_t min, int32_t build, int32_t local_build, 
    AsciiString location, AsciiString user, AsciiString time, AsciiString date ) 
{
    m_major = maj;
    m_minor = min;
    m_buildNum = build;
    m_localBuildNum = local_build;
    m_branch = location;
    m_commitHash = user;
    m_buildTime = time;
    m_buildDate = date;
}

AsciiString Version::Get_Ascii_Version()
{
    AsciiString version;

    if ( m_localBuildNum != 0 ) {
        version.Format("%d.%d.%d.%d", m_major, m_minor, m_buildNum, m_localBuildNum);
    } else {
        version.Format("%d.%d.%d", m_major, m_minor, m_buildNum);
    }

    return version;
}

AsciiString Version::Get_Ascii_Build_Time()
{
    AsciiString version;

    version.Format("%s %s", m_buildDate.Str(), m_buildTime.Str());

    return version;
}

UnicodeString Version::Get_Unicode_Version()
{
    UnicodeString ret;

    ret.Format(TheGameText->Fetch("Version:Format2").Str(), m_major, m_minor);

    return ret;
}

UnicodeString Version::Get_Full_Unicode_Version()
{
    UnicodeString ret;

    if ( m_localBuildNum != 0 ) {
        ret.Format(TheGameText->Fetch("Version:Format4").Str(), m_major, m_minor, m_buildNum, m_localBuildNum);
    } else {
        ret.Format(TheGameText->Fetch("Version:Format3").Str(), m_major, m_minor, m_buildNum);
    }

    return ret;
}

UnicodeString Version::Get_Unicode_Branch()
{
    UnicodeString ret;
    UnicodeString branch;

    branch.Translate(m_branch);
    ret.Format(TheGameText->Fetch("Version:BuildLocation").Str(), branch.Str());

    return ret;
}

UnicodeString Version::Get_Unicode_Commit_Hash()
{
    UnicodeString ret;
    UnicodeString hash;

    hash.Translate(m_commitHash);
    ret.Format(TheGameText->Fetch("Version:BuildUser").Str(), hash.Str());

    return ret;
}

UnicodeString Version::Get_Unicode_Build_Time()
{
    UnicodeString ret;
    UnicodeString date;
    UnicodeString time;

    date.Translate(m_buildDate);
    time.Translate(m_buildTime);

    ret.Format(TheGameText->Fetch("Version:BuildTime").Str(), date.Str(), time.Str());

    return ret;
}
