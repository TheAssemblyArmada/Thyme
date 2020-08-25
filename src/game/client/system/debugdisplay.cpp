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
#include "debugdisplay.h"
#include <captainslog.h>
#include <stdarg.h>
#include <stdio.h>

void DebugDisplay::Printf(const char *format, ...)
{
    static char _text[5120];

    va_list va;
    va_start(va, format);
    int length = vsnprintf(_text, sizeof(_text), format, va);
    va_end(va);

    if (length >= 0) {
        captainslog_dbgassert(length < sizeof(_text), "text overflow in %s - string too long", __CURRENT_FUNCTION__);
        Print(_text);
    }
}

void DebugDisplay::Print(char *string)
{
    char *line = string;
    int line_length = 0;

    while (true) {
        // Get the next character to evaluate.
        char c = *string++;

        // End of string, we are done scanning it.
        if (c == '\0') {
            break;
        }

        // New line, print what we currently have (if anything).
        if (c == '\n') {
            if (line_length > 0) {
                // Null terminate the string ready to draw.
                *(string - 1) = '\0';
                Draw_Text(m_xPos + m_rightMargin, m_yPos, line);
                line_length = 0;
            }

            // Set start of next line and move down a line.
            line = string;
            ++m_yPos;
            m_xPos = 0;
        } else {
            ++line_length;
        }
    }

    // Handle if we reached the end of the string with characters to print still.
    if (line_length > 0) {
        Draw_Text(m_xPos + m_rightMargin, m_yPos, line);
        m_xPos += line_length;
    }
}
