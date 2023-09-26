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
#include "gamewindow.h"

struct _TextData
{
    DisplayString *m_text;
    bool m_centered;
    bool m_vertCentered;
    int m_xOffset;
    int m_yOffset;
};

inline void Gadget_Static_Text_Set_Enabled_Image(GameWindow *static_text, const Image *image)
{
    static_text->Win_Set_Enabled_Image(0, image);
}

inline void Gadget_Static_Text_Set_Disabled_Image(GameWindow *static_text, const Image *image)
{
    static_text->Win_Set_Disabled_Image(0, image);
}

inline void Gadget_Static_Text_Set_Hilite_Image(GameWindow *static_text, const Image *image)
{
    static_text->Win_Set_Hilite_Image(0, image);
}

inline void Gadget_Static_Text_Set_Enabled_Color(GameWindow *static_text, int color)
{
    static_text->Win_Set_Enabled_Color(0, color);
}

inline void Gadget_Static_Text_Set_Enabled_Border_Color(GameWindow *static_text, int color)
{
    static_text->Win_Set_Enabled_Border_Color(0, color);
}

inline void Gadget_Static_Text_Set_Disabled_Color(GameWindow *static_text, int color)
{
    static_text->Win_Set_Disabled_Color(0, color);
}

inline void Gadget_Static_Text_Set_Disabled_Border_Color(GameWindow *static_text, int color)
{
    static_text->Win_Set_Disabled_Border_Color(0, color);
}

inline void Gadget_Static_Text_Set_Hilite_Color(GameWindow *static_text, int color)
{
    static_text->Win_Set_Hilite_Color(0, color);
}

inline void Gadget_Static_Text_Set_Hilite_Border_Color(GameWindow *static_text, int color)
{
    static_text->Win_Set_Hilite_Border_Color(0, color);
}

WindowMsgHandledType Gadget_Static_Text_Input(
    GameWindow *static_text, unsigned int message, unsigned int data_1, unsigned int data_2);

WindowMsgHandledType Gadget_Static_Text_System(
    GameWindow *static_text, unsigned int message, unsigned int data_1, unsigned int data_2);

void Gadget_Static_Text_Set_Text(GameWindow *static_text, Utf16String text);
Utf16String Gadget_Static_Text_Get_Text(GameWindow *static_text);
void Gadget_Static_Text_Set_Font(GameWindow *static_text, GameFont *font);
