/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Static Text
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

struct _TextData
{
    DisplayString *m_text;
    bool m_centered;
    bool m_vertCentered;
    int m_xOffset;
    int m_yOffset;
};

void Gadget_Static_Text_Set_Font(GameWindow *static_text, GameFont *font);
