/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Handles font configurations for current language.
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
#include "ini.h"
#include "subsysteminterface.h"
#include <list>

class W3DFontLibrary;

class FontDesc
{
    friend class GlobalLanguage;

public:
    FontDesc() : m_name("Arial Unicode MS"), m_pointSize(12), m_bold(false) {}
    FontDesc(const char *name, int size = 12, bool bold = false) : m_name(name), m_pointSize(size), m_bold(bold) {}

    const Utf8String &Name() const { return m_name; }
    int Point_Size() const { return m_pointSize; }
    bool Bold() const { return m_bold; }
    void Set_Name(Utf8String name) { m_name = name; }
    void Set_Point_Size(int size) { m_pointSize = size; }
    void Set_Bold(bool bold) { m_bold = bold; }

private:
    Utf8String m_name;
    int m_pointSize;
    bool m_bold;
};

class GlobalLanguage : public SubsystemInterface
{
public:
    friend W3DFontLibrary;
    friend class W3DDisplayString;

    GlobalLanguage();
    virtual ~GlobalLanguage() {}

    virtual void Init() override;
    virtual void Reset() override {}
    virtual void Update() override {}

    int Adjust_Font_Size(int size);

    const FontDesc &Copyright_Font() const { return m_copyrightFont; }
    const FontDesc &Message_Font() const { return m_messageFont; }
    const FontDesc &Military_Caption_Title_Font() const { return m_militaryCaptionTitleFont; }
    const FontDesc &Military_Caption_Font() const { return m_militaryCaptionFont; }
    const FontDesc &Superweapon_Countdown_Normal_Font() const { return m_superweaponCountdownNormalFont; }
    const FontDesc &Superweapon_Countdown_Ready_Font() const { return m_superweaponCountdownReadyFont; }
    const FontDesc &Named_Timer_Countdown_Normal_Font() const { return m_namedTimerCountdownNormalFont; }
    const FontDesc &Named_Timer_Countdown_Ready_Font() const { return m_namedTimerCountdownReadyFont; }
    const FontDesc &Drawable_Caption_Font() const { return m_drawableCaptionFont; }
    const FontDesc &Default_Window_Font() const { return m_defaultWindowFont; }
    const FontDesc &Default_Display_String_Font() const { return m_defaultDisplayStringFont; }
    const FontDesc &Tooltip() const { return m_tooltipFont; }
    const FontDesc &Debug_Display_Font() const { return m_nativeDebugDisplayFont; }
    const FontDesc &Draw_Group_Info_Font() const { return m_drawGroupInfoFont; }
    const FontDesc &Credits_Title_Font() const { return m_drawGroupInfoFont; }
    const FontDesc &Credits_Minor_Title_Font() const { return m_creditsMinorTitleFont; }
    const FontDesc &Credits_Normal_Font() const { return m_creditsNormalFont; }
    int Get_Military_Caption_Delay_MS() const { return m_militaryCaptionDelayMs; }
    int Get_Military_Caption_Speed() const { return m_militaryCaptionSpeed; }

    static void Parse_Language_Definition(INI *ini);
    static void Parse_Font_Filename(INI *ini, void *formal, void *store, void const *user_data);
    static void Parse_FontDesc(INI *ini, void *formal, void *store, void const *user_data);

private:
    Utf8String m_unicodeFontName;
    Utf8String m_unkAsciiString;
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
    std::list<Utf8String> m_localFontFiles;

    static const FieldParse s_languageParseTable[];
};

#ifdef GAME_DLL
extern GlobalLanguage *&g_theGlobalLanguage;
#else
extern GlobalLanguage *g_theGlobalLanguage;
#endif
