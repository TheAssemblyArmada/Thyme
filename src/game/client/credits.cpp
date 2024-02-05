/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Credits
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "credits.h"
#include "displaystringmanager.h"
#include "gamefont.h"
#include "gametext.h"
#include "globallanguage.h"

#ifndef GAME_DLL
CreditsManager *g_theCredits;
#endif

static LookupListRec s_creditStyleNames[] = { { "TITLE", STYLE_TITLE },
    { "MINORTITLE", STYLE_MINORTITLE },
    { "NORMAL", STYLE_NORMAL },
    { "COLUMN", STYLE_COLUMN },
    { nullptr, 0 } };

// clang-format off
const FieldParse CreditsManager::s_creditsFieldParseTable[] = {
    {"ScrollRate", &INI::Parse_Int, nullptr, offsetof(CreditsManager, m_scrollRate)},
    {"ScrollRateEveryFrames", &INI::Parse_Int, nullptr, offsetof(CreditsManager, m_scrollRateEveryFrames)},
    {"ScrollDown", &INI::Parse_Bool, nullptr, offsetof(CreditsManager, m_scrollDown)},
    {"TitleColor", &INI::Parse_Color_Int, nullptr, offsetof(CreditsManager, m_titleColor)},
    {"MinorTitleColor", &INI::Parse_Color_Int, nullptr, offsetof(CreditsManager, m_minorTitleColor)},
    {"NormalColor", &INI::Parse_Color_Int, nullptr, offsetof(CreditsManager, m_normalColor)},
    {"Style", &INI::Parse_Lookup_List, s_creditStyleNames, offsetof(CreditsManager, m_currentStyle)},
    {"Blank", &CreditsManager::Parse_Blank, nullptr, 0},
    {"Text", &CreditsManager::Parse_Text, nullptr, 0},
    {nullptr, nullptr, nullptr, 0}
};
// clang-format on

CreditsLine::CreditsLine() :
    m_style(STYLE_BLANK),
    m_hasFirstColumn(false),
    m_hasSecondColumn(false),
    m_displayString(nullptr),
    m_secondColumnDisplayString(nullptr),
    m_xPos(0),
    m_yPos(0),
    m_scroll(0),
    m_color(0)
{
}

CreditsLine::~CreditsLine()
{
    if (m_displayString != nullptr) {
        g_theDisplayStringManager->Free_Display_String(m_displayString);
        m_displayString = nullptr;
    }

    if (m_secondColumnDisplayString != nullptr) {
        g_theDisplayStringManager->Free_Display_String(m_secondColumnDisplayString);
        m_secondColumnDisplayString = nullptr;
    }
}

void CreditsManager::Load()
{
    INI ini;
    ini.Load("Data\\INI\\Credits.ini", INI_LOAD_OVERWRITE, nullptr);

    if (m_scrollRateEveryFrames <= 0) {
        m_scrollRateEveryFrames = 1;
    }

    if (m_scrollRate <= 0) {
        m_scrollRate = 1;
    }

    GameFont *font = g_theFontLibrary->Get_Font(g_theGlobalLanguage->Credits_Normal_Font().Name(),
        g_theGlobalLanguage->Adjust_Font_Size(g_theGlobalLanguage->Credits_Normal_Font().Point_Size()),
        g_theGlobalLanguage->Credits_Normal_Font().Bold());
    m_fontHeight = font->m_height;
}

void CreditsManager::Parse_Blank(INI *ini, void *formal, void *store, const void *user_data)
{
    static_cast<CreditsManager *>(formal)->Add_Blank();
}

void CreditsManager::Parse_Text(INI *ini, void *formal, void *store, const void *user_data)
{
    static_cast<CreditsManager *>(formal)->Add_Text(ini->Get_Next_Quoted_Ascii_String());
}

void CreditsManager::Parse(INI *ini)
{
    captainslog_dbgassert(g_theCredits != nullptr, "CreditsManager::Parse: TheCredits has not been ininialized yet.");

    if (g_theCredits != nullptr) {
        ini->Init_From_INI(g_theCredits, g_theCredits->Get_Field_Parse());
    }
}

void CreditsManager::Add_Blank()
{
    CreditsLine *line = new CreditsLine();
    line->m_style = STYLE_BLANK;
    m_creditLines.push_back(line);
}

void CreditsManager::Add_Text(Utf8String text)
{
    CreditsLine *line = new CreditsLine();

    if (m_currentStyle < STYLE_TITLE) {
        captainslog_dbgassert(false,
            "CreditsManager::Add_Text we tried to add a credit text with the wrong style before it.  Style is %d",
            m_currentStyle);
        delete line;
        return;
    }

    if (m_currentStyle <= STYLE_NORMAL) {
        line->m_string = Get_Unicode_String(text);
        line->m_style = m_currentStyle;
        m_creditLines.push_back(line);
        return;
    }

    if (m_currentStyle == STYLE_COLUMN) {
        auto it = m_creditLines.rbegin();
        CreditsLine *last_line = *it;

        if (it == m_creditLines.rend() || last_line->m_style != STYLE_COLUMN || last_line->m_hasSecondColumn) {
            line->m_string = Get_Unicode_String(text);
            line->m_style = STYLE_COLUMN;
            line->m_hasFirstColumn = true;
            m_creditLines.push_back(line);
        } else {
            last_line->m_secondColumnString = Get_Unicode_String(text);
            last_line->m_hasSecondColumn = 1;
            delete line;
        }
    } else {
        captainslog_dbgassert(false,
            "CreditsManager::Add_Text we tried to add a credit text with the wrong style before it.  Style is %d",
            m_currentStyle);
        delete line;
    }
}

Utf16String CreditsManager::Get_Unicode_String(Utf8String string)
{
    Utf16String unicode_string;

    if (string.Compare("<BLANK>") != 0) {
        if (string.Find(':') != nullptr) {
            unicode_string = g_theGameText->Fetch(string);
        } else {
            unicode_string.Translate(string);
        }

        return unicode_string;
    } else {
        return Utf16String::s_emptyString;
    }
}
