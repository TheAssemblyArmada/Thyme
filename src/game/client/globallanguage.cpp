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
#include "globallanguage.h"
#include "gamemath.h"
#include "globaldata.h"
#include "registryget.h"
#include <algorithm>
#include <cstddef>

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#endif

#ifndef GAME_DLL
GlobalLanguage *g_theGlobalLanguage;
#endif

const FieldParse GlobalLanguage::s_languageParseTable[] = {
    { "UnicodeFontName", &INI::Parse_AsciiString, nullptr, offsetof(GlobalLanguage, m_unicodeFontName) },
    { "LocalFontFile", &GlobalLanguage::Parse_Font_Filename, nullptr, 0 },
    { "MilitaryCaptionSpeed", &INI::Parse_Int, nullptr, offsetof(GlobalLanguage, m_militaryCaptionSpeed) },
    { "UseHardWordWrap", &INI::Parse_Bool, nullptr, offsetof(GlobalLanguage, m_useHardWordWrap) },
    { "ResolutionFontAdjustment", &INI::Parse_Real, nullptr, offsetof(GlobalLanguage, m_resolutionFontAdjustment) },
    { "CopyrightFont", &GlobalLanguage::Parse_FontDesc, nullptr, offsetof(GlobalLanguage, m_copyrightFont) },
    { "MessageFont", &GlobalLanguage::Parse_FontDesc, nullptr, offsetof(GlobalLanguage, m_messageFont) },
    { "MilitaryCaptionTitleFont",
        &GlobalLanguage::Parse_FontDesc,
        nullptr,
        offsetof(GlobalLanguage, m_militaryCaptionTitleFont) },
    { "MilitaryCaptionDelayMS", &INI::Parse_Int, nullptr, offsetof(GlobalLanguage, m_militaryCaptionDelayMs) },
    { "MilitaryCaptionFont", &GlobalLanguage::Parse_FontDesc, nullptr, offsetof(GlobalLanguage, m_militaryCaptionFont) },
    { "SuperweaponCountdownNormalFont",
        &GlobalLanguage::Parse_FontDesc,
        nullptr,
        offsetof(GlobalLanguage, m_superweaponCountdownNormalFont) },
    { "SuperweaponCountdownReadyFont",
        &GlobalLanguage::Parse_FontDesc,
        nullptr,
        offsetof(GlobalLanguage, m_superweaponCountdownReadyFont) },
    { "NamedTimerCountdownNormalFont",
        &GlobalLanguage::Parse_FontDesc,
        nullptr,
        offsetof(GlobalLanguage, m_namedTimerCountdownNormalFont) },
    { "NamedTimerCountdownReadyFont",
        &GlobalLanguage::Parse_FontDesc,
        nullptr,
        offsetof(GlobalLanguage, m_namedTimerCountdownReadyFont) },
    { "DrawableCaptionFont", &GlobalLanguage::Parse_FontDesc, nullptr, offsetof(GlobalLanguage, m_drawableCaptionFont) },
    { "DefaultWindowFont", &GlobalLanguage::Parse_FontDesc, nullptr, offsetof(GlobalLanguage, m_defaultWindowFont) },
    { "DefaultDisplayStringFont",
        &GlobalLanguage::Parse_FontDesc,
        nullptr,
        offsetof(GlobalLanguage, m_defaultDisplayStringFont) },
    { "TooltipFontName", &GlobalLanguage::Parse_FontDesc, nullptr, offsetof(GlobalLanguage, m_tooltipFont) },
    { "NativeDebugDisplay", &GlobalLanguage::Parse_FontDesc, nullptr, offsetof(GlobalLanguage, m_nativeDebugDisplayFont) },
    { "DrawGroupInfoFont", &GlobalLanguage::Parse_FontDesc, nullptr, offsetof(GlobalLanguage, m_drawGroupInfoFont) },
    { "CreditsTitleFont", &GlobalLanguage::Parse_FontDesc, nullptr, offsetof(GlobalLanguage, m_creditsTitleFont) },
    { "CreditsMinorTitleFont", &GlobalLanguage::Parse_FontDesc, nullptr, offsetof(GlobalLanguage, m_creditsMinorTitleFont) },
    { "CreditsNormalFont", &GlobalLanguage::Parse_FontDesc, nullptr, offsetof(GlobalLanguage, m_creditsNormalFont) },
    { nullptr, nullptr, nullptr, 0 }
};

GlobalLanguage::GlobalLanguage() :
    m_unicodeFontName(),
    m_unkAsciiString(),
    m_useHardWordWrap(false),
    m_militaryCaptionSpeed(0),
    m_militaryCaptionDelayMs(750),
    m_copyrightFont(),
    m_messageFont(),
    m_militaryCaptionTitleFont(),
    m_militaryCaptionFont(),
    m_superweaponCountdownNormalFont(),
    m_superweaponCountdownReadyFont(),
    m_namedTimerCountdownNormalFont(),
    m_namedTimerCountdownReadyFont(),
    m_drawableCaptionFont(),
    m_defaultWindowFont(),
    m_defaultDisplayStringFont(),
    m_tooltipFont(),
    m_nativeDebugDisplayFont(),
    m_drawGroupInfoFont(),
    m_creditsTitleFont(),
    m_creditsMinorTitleFont(),
    m_creditsNormalFont(),
    m_resolutionFontAdjustment(0.7f),
    m_localFontFiles()
{
}

void GlobalLanguage::Init()
{
    INI ini;
    Utf8String file;

    file.Format("Data/%s/Language.ini", Get_Registry_Language().Str());

    // Original checked if it was running on Windows9x and loaded Language9x.ini
    // instead if the file existed. Thyme doesn't support running on Win9x so
    // we don't implement it. Only chinese language known to use it.
    ini.Load(file, INI_LOAD_OVERWRITE, nullptr);

#ifdef PLATFORM_WINDOWS
    // Original checked if it was running on a windows version, that supports this
    // Also use "AddFontResourceEx", to not permanently install the font
    for (auto &font : m_localFontFiles) {
        AddFontResourceEx(font.Str(), FR_PRIVATE, 0);
    }
#endif
}

int GlobalLanguage::Adjust_Font_Size(int size)
{
    float adjustment = std::clamp(
        (((g_theWriteableGlobalData->m_xResolution / 800.0f) - 1.0f) * m_resolutionFontAdjustment) + 1.0f, 1.0f, 2.0f);

    return GameMath::Fast_To_Int_Floor(adjustment * size);
}

// Was originally INI::parseLanguageDefinition
void GlobalLanguage::Parse_Language_Definition(INI *ini)
{
    if (g_theGlobalLanguage != nullptr) {
        ini->Init_From_INI(g_theGlobalLanguage, s_languageParseTable);
    }
}

void GlobalLanguage::Parse_Font_Filename(INI *ini, void *formal, void *store, void const *user_data)
{
    Utf8String font_name = ini->Get_Next_Ascii_String();
    static_cast<GlobalLanguage *>(formal)->m_localFontFiles.push_front(font_name);
}

void GlobalLanguage::Parse_FontDesc(INI *ini, void *formal, void *store, void const *user_data)
{
    FontDesc *font = static_cast<FontDesc *>(store);

    font->m_name = ini->Get_Next_Quoted_Ascii_String();
    font->m_pointSize = INI::Scan_Int(ini->Get_Next_Token());
    font->m_bold = INI::Scan_Bool(ini->Get_Next_Token());
}
