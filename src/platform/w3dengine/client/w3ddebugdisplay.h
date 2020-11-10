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
#pragma once

#include "always.h"
#include "debugdisplay.h"
#include <new>

class DisplayString;
class GameFont;

class W3DDebugDisplay : public DebugDisplay
{
public:
    W3DDebugDisplay() : m_font(nullptr), m_fontWidth(0), m_fontHeight(0), m_displayString(nullptr) {}
    virtual ~W3DDebugDisplay();

    virtual void Draw_Text(int x, int y, char *text) override;

    void Init();
    void Set_Font(GameFont *font);
    void Set_Font_Width(int width) { m_fontWidth = width; }
    void Set_Font_Height(int height) { m_fontHeight = height; }

#ifdef GAME_DLL
    W3DDebugDisplay *Hook_Ctor() { return new (this) W3DDebugDisplay(); }
#endif

private:
    GameFont *m_font;
    int m_fontWidth;
    int m_fontHeight;
    DisplayString *m_displayString;
};
