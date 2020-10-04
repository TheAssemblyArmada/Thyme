/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Base class for strings to be displayed.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "displaystring.h"

DisplayString::DisplayString() : m_textString(), m_font(nullptr), m_next(nullptr), m_prev(nullptr) {}

DisplayString::~DisplayString()
{
    Reset();
}

/**
 * @brief Set text to be displayed.
 *
 * 0x007F8260
 */
void DisplayString::Set_Text(Utf16String text)
{
    if (text != m_textString) {
        m_textString = text;
        Notify_Text_Changed();
    }
}

/**
 * @brief Reset the instance to a blank string and null font.
 *
 * 0x007F82F0
 */
void DisplayString::Reset()
{
    m_textString.Clear();
    m_font = nullptr;
}

/**
 * @brief Removes the last character from the string.
 *
 * 0x007F8310
 */
void DisplayString::Remove_Last_Char()
{
    m_textString.Remove_Last_Char();
    Notify_Text_Changed();
}

/**
 * @brief Concatenates wchar_t onto end of string.
 *
 * 0x007F8330
 */
void DisplayString::Add_Char(wchar_t ch)
{
    m_textString += ch;
    Notify_Text_Changed();
}
