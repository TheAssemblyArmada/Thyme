/**
 * @file
 *
 * @author tomsons26
 *
 * @brief Base class for the display handling.
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

class DebugDisplayInterface
{
public:
    enum Color
    {
        WHITE,
        BLACK,
        YELLOW,
        RED,
        GREEN,
        BLUE,
        NUM_COLORS,
    };

public:
    DebugDisplayInterface() {}
    virtual ~DebugDisplayInterface() {}

    virtual void Printf(const char *format, ...) = 0;
    virtual void Set_Cursor_Pos(int x, int y) = 0;
    virtual int Get_Cursor_X_Pos() = 0;
    virtual int Get_Cursor_Y_Pos() = 0;
    virtual int Get_Width() = 0;
    virtual int Get_Height() = 0;
    virtual void Set_Text_Color(Color color) = 0;
    virtual void Set_Right_Margin(int right_pos) = 0;
    virtual void Set_Left_Margin(int left_pos) = 0;
    virtual void Reset() = 0;
    virtual void Draw_Text(int x, int y, char *text) = 0;
};

class DebugDisplay : public DebugDisplayInterface
{
public:
    DebugDisplay() : m_width(0), m_height(0) { Reset(); }
    virtual ~DebugDisplay() {}

    virtual void Printf(const char *format, ...) override;
    virtual void Set_Cursor_Pos(int x, int y) override
    {
        m_xPos = x;
        m_yPos = y;
    }
    virtual int Get_Cursor_X_Pos() override { return m_xPos; }
    virtual int Get_Cursor_Y_Pos() override { return m_yPos; }
    virtual int Get_Width() override { return m_width; }
    virtual int Get_Height() override { return m_height; }
    virtual void Set_Text_Color(DebugDisplayInterface::Color color) override { m_textColor = color; }
    virtual void Set_Right_Margin(int right_pos) override { m_rightMargin = right_pos; }
    virtual void Set_Left_Margin(int left_pos) override { m_leftMargin = left_pos; }
    virtual void Reset() override
    {
        Set_Cursor_Pos(0, 0);
        Set_Text_Color(DebugDisplayInterface::WHITE);
        Set_Right_Margin(0);
        Set_Left_Margin(Get_Width());
    }

private:
    void Print(char *string);

private:
    DebugDisplayInterface::Color m_textColor;
    int m_xPos;
    int m_yPos;
    int m_width;
    int m_height;
    int m_rightMargin;
    int m_leftMargin;
};
