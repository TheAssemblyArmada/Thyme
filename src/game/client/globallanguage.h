////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: GLOBALLANGUAGE.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Handles font configurations for current language.
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

#ifndef GLOBALLANGUAGE_H
#define GLOBALLANGUAGE_H

#ifndef THYME_STANDALONE
#include "hooker.h"
#endif

#include "asciistring.h"
#include "ini.h"
#include "subsysteminterface.h"
#include <list>

class FontDesc
{
    friend class GlobalLanguage;
public:
    FontDesc() : m_name("Arial Unicode MS"), m_pointSize(12), m_bold(false) {}

    AsciiString &Get_Font_Name() { return m_name; }
    int Get_Point_Size() { return m_pointSize; }
    bool Is_Bold() { return m_bold; }

private:
    AsciiString m_name;
    int m_pointSize;
    bool m_bold;
};

class GlobalLanguage : public SubsystemInterface
{
public:
    GlobalLanguage();
    virtual ~GlobalLanguage() {}

    virtual void Init() override;
    virtual void Reset() override {}
    virtual void Update() override {}

    static void Parse_Language_Defintions(INI *ini);
    static void Parse_Font_Filename(INI *ini, void *formal, void *store, void const *user_data);
    static void Parse_FontDesc(INI *ini, void *formal, void *store, void const *user_data);

private:
    AsciiString m_unicodeFontName;
    AsciiString m_unkAsciiString;
    bool m_useHardWordWrap;
    int m_militaryCaptionSpeed;
    int m_militaryCaptionDelayMs;
    FontDesc m_copyrightFont;
    FontDesc m_messageFont;
    FontDesc m_militaryCaptionTitleFont;
    FontDesc m_militaryCaptionFont;
    FontDesc m_superweaponCountdownNormalFont;
    FontDesc m_superweaponCountdownReadyFont;
    FontDesc m_namedTimerCountdownNormalFont;
    FontDesc m_namedTimerCountdownReadyFont;
    FontDesc m_drawableCaptionFont;
    FontDesc m_defaultWindowFont;
    FontDesc m_defaultDisplayStringFont;
    FontDesc m_tooltipFont;
    FontDesc m_nativeDebugDisplayFont;
    FontDesc m_drawGroupInfoFont;
    FontDesc m_creditsTitleFont;
    FontDesc m_creditsMinorTitleFont;
    FontDesc m_creditsNormalFont;
    float m_resolutionFontAdjustment;
    std::list<AsciiString> m_localFontFiles;

    static FieldParse s_languageParseTable[];
};

#ifdef THYME_STANDALONE
extern GlobalLanguage *g_theGlobalLanguage;
#else
#define g_theGlobalLanguage (Make_Global<GlobalLanguage*>(0x00A2A6CC))
#endif

#endif // GLOBALLANGUAGE_H
