/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Stores information about the current build.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "version.h"
#include "gametext.h"

#ifndef GAME_DLL
Version *g_theVersion = nullptr;
#endif

Version::Version() :
    m_major(1),
    m_minor(0),
    m_buildNum(0),
    m_localBuildNum(0),
    m_branch("somewhere"),
    m_commitHash("someone"),
    m_buildTime(),
    m_buildDate(),
    m_useFullVersion(false)
{
}

void Version::Set_Version(int32_t maj,
    int32_t min,
    int32_t build,
    int32_t local_build,
    Utf8String location,
    Utf8String user,
    Utf8String time,
    Utf8String date)
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

Utf8String Version::Get_Ascii_Version()
{
    Utf8String version;

    if (m_localBuildNum != 0) {
        version.Format("%d.%d.%d.%d", m_major, m_minor, m_buildNum, m_localBuildNum);
    } else {
        version.Format("%d.%d.%d", m_major, m_minor, m_buildNum);
    }

    return version;
}

Utf8String Version::Get_Ascii_Build_Time()
{
    Utf8String version;

    version.Format("%s %s", m_buildDate.Str(), m_buildTime.Str());

    return version;
}

Utf16String Version::Get_Unicode_Version()
{
    Utf16String ret;

    ret.Format(g_theGameText->Fetch("Version:Format2").Str(), m_major, m_minor);

    return ret;
}

Utf16String Version::Get_Full_Unicode_Version()
{
    Utf16String ret;

    if (m_localBuildNum != 0) {
        ret.Format(g_theGameText->Fetch("Version:Format4").Str(), m_major, m_minor, m_buildNum, m_localBuildNum);
    } else {
        ret.Format(g_theGameText->Fetch("Version:Format3").Str(), m_major, m_minor, m_buildNum);
    }

    return ret;
}

Utf16String Version::Get_Unicode_Branch()
{
    Utf16String ret;
    Utf16String branch;

    branch.Translate(m_branch);
    ret.Format(g_theGameText->Fetch("Version:BuildLocation").Str(), branch.Str());

    return ret;
}

Utf16String Version::Get_Unicode_Commit_Hash()
{
    Utf16String ret;
    Utf16String hash;

    hash.Translate(m_commitHash);
    ret.Format(g_theGameText->Fetch("Version:BuildUser").Str(), hash.Str());

    return ret;
}

Utf16String Version::Get_Unicode_Build_Time()
{
    Utf16String ret;
    Utf16String date;
    Utf16String time;

    date.Translate(m_buildDate);
    time.Translate(m_buildTime);

    ret.Format(g_theGameText->Fetch("Version:BuildTime").Str(), date.Str(), time.Str());

    return ret;
}
