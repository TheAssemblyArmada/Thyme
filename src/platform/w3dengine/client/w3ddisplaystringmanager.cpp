/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Display String Manager
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3ddisplaystringmanager.h"
#include "drawgroupinfo.h"
#include "gameclient.h"
#include "gametext.h"
#include "globallanguage.h"
#include "w3ddisplaystring.h"

W3DDisplayStringManager::W3DDisplayStringManager()
{
    for (int i = 0; i < NUM_GROUP_STRINGS; i++) {
        m_groupNumeralStrings[i] = nullptr;
    }

    m_formationLetterString = nullptr;
}

W3DDisplayStringManager::~W3DDisplayStringManager()
{
    for (int i = 0; i < NUM_GROUP_STRINGS; i++) {
        DisplayString *str = m_groupNumeralStrings[i];

        if (str != nullptr) {
            Unlink(str);

            if (str == m_currentCheckpoint) {
                m_currentCheckpoint = nullptr;
            }

            str->Delete_Instance();
            m_groupNumeralStrings[i] = 0;
        }
    }

    if (m_formationLetterString != nullptr) {
        Unlink(m_formationLetterString);

        if (m_formationLetterString == m_currentCheckpoint) {
            m_currentCheckpoint = nullptr;
        }

        m_formationLetterString->Delete_Instance();
        m_formationLetterString = nullptr;
    }
}

void W3DDisplayStringManager::PostProcessLoad()
{
    GameFont *font = g_theFontLibrary->Get_Font(
        g_theDrawGroupInfo->m_fontName, g_theDrawGroupInfo->m_fontSize, g_theDrawGroupInfo->m_fontIsBold);

    for (int i = 0; i < NUM_GROUP_STRINGS; i++) {
        m_groupNumeralStrings[i] = New_Display_String();
        m_groupNumeralStrings[i]->Set_Font(font);
        Utf8String name;
        name.Format("NUMBER:%d", i);
        m_groupNumeralStrings[i]->Set_Text(g_theGameText->Fetch(name));
    }

    m_formationLetterString = New_Display_String();
    m_formationLetterString->Set_Font(font);
    Utf8String name;
    name.Format("LABEL:FORMATION");
    m_formationLetterString->Set_Text(g_theGameText->Fetch(name));
}

void W3DDisplayStringManager::Update()
{
    W3DDisplayString *str = static_cast<W3DDisplayString *>(m_stringList);

    if (m_currentCheckpoint != nullptr) {
        str = static_cast<W3DDisplayString *>(m_currentCheckpoint);
    }

    uint32_t cur_frame = g_theGameClient->Get_Frame();
    int i = NUM_GROUP_STRINGS - 1;

    do {
        if (str == nullptr) {
            break;
        }

        unsigned int draw_frame = str->m_frame;

        if (draw_frame && cur_frame - draw_frame > 60) {
            str->m_sentence.Reset();
            str->m_hotKeySentence.Reset();
            str->m_textChanged = true;
            str->m_frame = 0;
        }

        str = static_cast<W3DDisplayString *>(str->m_next);
    } while (i--);
}

DisplayString *W3DDisplayStringManager::New_Display_String()
{
    W3DDisplayString *str = new W3DDisplayString();

    Utf8String name;
    int size;
    bool bold;

    if (g_theGlobalLanguage != nullptr && !g_theGlobalLanguage->Default_Display_String_Font().Name().Is_Empty()) {
        name = g_theGlobalLanguage->Default_Display_String_Font().Name();
        size = g_theGlobalLanguage->Default_Display_String_Font().Point_Size();
        bold = g_theGlobalLanguage->Default_Display_String_Font().Bold();
    } else {
        name = "Times New Roman";
        size = 12;
        bold = false;
    }

    str->Set_Font(g_theFontLibrary->Get_Font(name, size, bold));
    Link(str);
    return str;
}

void W3DDisplayStringManager::Free_Display_String(DisplayString *string)
{
    if (string != nullptr) {
        Unlink(string);

        if (m_currentCheckpoint == string) {
            m_currentCheckpoint = nullptr;
        }

        string->Delete_Instance();
    }
}

DisplayString *W3DDisplayStringManager::Get_Group_Numeral_String(int num)
{
    if (num >= NUM_GROUP_STRINGS) {
        return m_groupNumeralStrings[0];
    } else {
        return m_groupNumeralStrings[num];
    }
}

DisplayString *W3DDisplayStringManager::Get_Formation_Letter_String()
{
    return m_formationLetterString;
}
