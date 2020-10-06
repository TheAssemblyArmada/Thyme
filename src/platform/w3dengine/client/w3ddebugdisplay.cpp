/**
 * @file
 *
 * @author tomsons26
 *
 * @brief
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3ddebugdisplay.h"
#include "colorspace.h"
#include "displaystring.h"
#include "displaystringmanager.h"

W3DDebugDisplay::~W3DDebugDisplay()
{
    if (m_displayString != nullptr) {
        g_theDisplayStringManager->Free_Display_String(m_displayString);
    }
}

void W3DDebugDisplay::Draw_Text(int x, int y, char *text)
{
    if (m_font != nullptr && m_displayString != nullptr) {
        Utf16String unicode;

        unicode.Translate(text);

        m_displayString->Set_Text(unicode);
        m_displayString->Draw(
            m_fontWidth * x, m_fontHeight * y + 13, Make_Color(255, 255, 255, 255), Make_Color(0, 0, 0, 255));
    }
}

void W3DDebugDisplay::Init()
{
    m_displayString = g_theDisplayStringManager->New_Display_String();
}

void W3DDebugDisplay::Set_Font(GameFont *font)
{
    m_font = font;

    if (m_displayString != nullptr) {
        m_displayString->Set_Font(font);
    }
}
