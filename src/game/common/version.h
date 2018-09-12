/**
 * @file
 *
 * @Author OmniBlade
 *
 * @brief Stores information about the current build.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "always.h"
#include "asciistring.h"
#include "unicodestring.h"

class Version
{
public:
    Version();

    void Set_Version(int32_t maj, int32_t min, int32_t build, int32_t local_build, AsciiString location, AsciiString user,
        AsciiString time, AsciiString date);

    int32_t Get_Version_Number() { return m_minor | (m_major << 16); }
    int32_t Get_Build_Number() { return m_buildNum; }
    int32_t Get_Local_Build_Number() { return m_localBuildNum; }

    AsciiString Get_Ascii_Version();
    AsciiString Get_Ascii_Branch() { return m_branch; }
    AsciiString Get_Ascii_Commit_Hash() { return m_commitHash; }
    AsciiString Get_Ascii_Build_Time();
    Utf16String Get_Unicode_Version();
    Utf16String Get_Full_Unicode_Version();
    Utf16String Get_Unicode_Branch();
    Utf16String Get_Unicode_Commit_Hash();
    Utf16String Get_Unicode_Build_Time();

    void Set_Use_Full_Version(bool cmd_line) { m_useFullVersion = cmd_line; }
    bool Using_Full_Version() { return m_useFullVersion; }

private:
    int32_t m_major;
    int32_t m_minor;
    int32_t m_buildNum;
    int32_t m_localBuildNum;
    AsciiString m_branch; // Was Location in orignal
    AsciiString m_commitHash; // Was User in original
    AsciiString m_buildTime;
    AsciiString m_buildDate;
    bool m_useFullVersion;
};

#ifndef THYME_STANDALONE
extern Version *&g_theVersion;
#else
extern Version *g_theVersion;
#endif