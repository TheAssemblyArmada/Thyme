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
#include "registry.h"
#include <algorithm>
#include <cstddef>

#ifndef GAME_DLL
GlobalLanguage *g_theGlobalLanguage;
#endif

// clang-format off
const FieldParse GlobalLanguage::s_languageParseTable[] = {
    FIELD_PARSE_ASCIISTRING("UnicodeFontName", GlobalLanguage, m_unicodeFontName),
    FIELD_PARSE_FONT_FILENAME("LocalFontFile"),
    FIELD_PARSE_INT("MilitaryCaptionSpeed", GlobalLanguage, m_militaryCaptionSpeed),
    FIELD_PARSE_BOOL("UseHardWordWrap", GlobalLanguage, m_useHardWordWrap),
    FIELD_PARSE_REAL("ResolutionFontAdjustment", GlobalLanguage, m_resolutionFontAdjustment),
    FIELD_PARSE_FONT_DESC("CopyrightFont", GlobalLanguage, m_copyrightFont),
    FIELD_PARSE_FONT_DESC("MessageFont", GlobalLanguage, m_messageFont),
    FIELD_PARSE_FONT_DESC("MilitaryCaptionTitleFont", GlobalLanguage, m_militaryCaptionTitleFont),
    FIELD_PARSE_INT("MilitaryCaptionDelayMS", GlobalLanguage, m_militaryCaptionDelayMs),
    FIELD_PARSE_FONT_DESC("MilitaryCaptionFont", GlobalLanguage, m_militaryCaptionFont),
    FIELD_PARSE_FONT_DESC("SuperweaponCountdownNormalFont", GlobalLanguage, m_superweaponCountdownNormalFont),
    FIELD_PARSE_FONT_DESC("SuperweaponCountdownReadyFont", GlobalLanguage, m_superweaponCountdownReadyFont),
    FIELD_PARSE_FONT_DESC("NamedTimerCountdownNormalFont", GlobalLanguage, m_namedTimerCountdownNormalFont),
    FIELD_PARSE_FONT_DESC("NamedTimerCountdownReadyFont", GlobalLanguage, m_namedTimerCountdownReadyFont),
    FIELD_PARSE_FONT_DESC("DrawableCaptionFont", GlobalLanguage, m_drawableCaptionFont),
    FIELD_PARSE_FONT_DESC("DefaultWindowFont", GlobalLanguage, m_defaultWindowFont),
    FIELD_PARSE_FONT_DESC("DefaultDisplayStringFont", GlobalLanguage, m_defaultDisplayStringFont),
    FIELD_PARSE_FONT_DESC("TooltipFontName", GlobalLanguage, m_tooltipFont),
    FIELD_PARSE_FONT_DESC("NativeDebugDisplay", GlobalLanguage, m_nativeDebugDisplayFont),
    FIELD_PARSE_FONT_DESC("DrawGroupInfoFont", GlobalLanguage, m_drawGroupInfoFont),
    FIELD_PARSE_FONT_DESC("CreditsTitleFont", GlobalLanguage, m_creditsTitleFont),
    FIELD_PARSE_FONT_DESC("CreditsMinorTitleFont", GlobalLanguage, m_creditsMinorTitleFont),
    FIELD_PARSE_FONT_DESC("CreditsNormalFont", GlobalLanguage, m_creditsNormalFont),
    FIELD_PARSE_LAST
};
// clang-format on

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
}

int GlobalLanguage::Adjust_Font_Size(int size)
{
    float adjustment = std::clamp(
        (((g_theWriteableGlobalData->m_xResolution / 800) - 1.0f) * m_resolutionFontAdjustment) + 1.0f, 1.0f, 2.0f);

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
