/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Text Entry
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
class DisplayString;
class GameWindow;
class GameFont;

struct _EntryData
{
    DisplayString *m_text;
    DisplayString *m_sText;
    DisplayString *m_constructText;
    bool m_secretText;
    bool m_numericalOnly;
    bool m_alphaNumericalOnly;
    bool m_asciiOnly;
    short m_maxTextLen;
    bool m_receivedUnichar;
    GameWindow *m_constructList;
    short m_charPos;
    short m_conCharPos;
};

void Gadget_Text_Entry_Set_Font(GameWindow *text_entry, GameFont *font);
