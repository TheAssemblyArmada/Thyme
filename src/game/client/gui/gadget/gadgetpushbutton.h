/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Push Button
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

struct _PushButtonData
{
    char m_drawClock;
    int m_percentClock;
    int m_colorClock;
    bool m_drawBorder;
    int m_colorBorder;
    void *m_userData;
    const Image *m_overlayImage;
    Utf8String m_altSound;
};

inline void Gadget_Button_Set_Enabled_Image(GameWindow *push_button, const Image *image)
{
    push_button->Win_Set_Enabled_Image(0, image);
    push_button->Win_Set_Enabled_Image(5, 0);
    push_button->Win_Set_Enabled_Image(6, 0);
}

inline void Gadget_Button_Set_Disabled_Image(GameWindow *push_button, const Image *image)
{
    push_button->Win_Set_Disabled_Image(0, image);
    push_button->Win_Set_Enabled_Image(5, 0);
    push_button->Win_Set_Enabled_Image(6, 0);
}

inline void Gadget_Button_Set_Hilite_Image(GameWindow *push_button, const Image *image)
{
    push_button->Win_Set_Hilite_Image(0, image);
    push_button->Win_Set_Enabled_Image(5, 0);
    push_button->Win_Set_Enabled_Image(6, 0);
}

inline void Gadget_Button_Set_Enabled_Selected_Image(GameWindow *push_button, const Image *image)
{
    push_button->Win_Set_Enabled_Image(1, image);
    push_button->Win_Set_Enabled_Image(2, 0);
    push_button->Win_Set_Enabled_Image(3, 0);
}

inline void Gadget_Button_Set_Disabled_Selected_Image(GameWindow *push_button, const Image *image)
{
    push_button->Win_Set_Disabled_Image(1, image);
    push_button->Win_Set_Enabled_Image(2, 0);
    push_button->Win_Set_Enabled_Image(3, 0);
}

inline void Gadget_Button_Set_Hilite_Selected_Image(GameWindow *push_button, const Image *image)
{
    push_button->Win_Set_Hilite_Image(1, image);
    push_button->Win_Set_Enabled_Image(2, 0);
    push_button->Win_Set_Enabled_Image(3, 0);
}

inline void Gadget_Button_Set_Enabled_Color(GameWindow *push_button, int color)
{
    push_button->Win_Set_Enabled_Color(0, color);
}

inline void Gadget_Button_Set_Enabled_Border_Color(GameWindow *push_button, int color)
{
    push_button->Win_Set_Enabled_Border_Color(0, color);
}

inline void Gadget_Button_Set_Enabled_Selected_Color(GameWindow *push_button, int color)
{
    push_button->Win_Set_Enabled_Color(1, color);
}

inline void Gadget_Button_Set_Enabled_Selected_Border_Color(GameWindow *push_button, int color)
{
    push_button->Win_Set_Enabled_Border_Color(1, color);
}

inline void Gadget_Button_Set_Disabled_Color(GameWindow *push_button, int color)
{
    push_button->Win_Set_Disabled_Color(0, color);
}

inline void Gadget_Button_Set_Disabled_Border_Color(GameWindow *push_button, int color)
{
    push_button->Win_Set_Disabled_Border_Color(0, color);
}

inline void Gadget_Button_Set_Disabled_Selected_Color(GameWindow *push_button, int color)
{
    push_button->Win_Set_Disabled_Color(1, color);
}

inline void Gadget_Button_Set_Disabled_Selected_Border_Color(GameWindow *push_button, int color)
{
    push_button->Win_Set_Disabled_Border_Color(1, color);
}

inline void Gadget_Button_Set_Hilite_Color(GameWindow *push_button, int color)
{
    push_button->Win_Set_Hilite_Color(0, color);
}

inline void Gadget_Button_Set_Hilite_Border_Color(GameWindow *push_button, int color)
{
    push_button->Win_Set_Hilite_Border_Color(0, color);
}

inline void Gadget_Button_Set_Hilite_Selected_Color(GameWindow *push_button, int color)
{
    push_button->Win_Set_Hilite_Color(1, color);
}

inline void Gadget_Button_Set_Hilite_Selected_Border_Color(GameWindow *push_button, int color)
{
    push_button->Win_Set_Hilite_Border_Color(1, color);
}

inline int Gadget_Button_Get_Enabled_Selected_Color(GameWindow *push_button)
{
    return push_button->Win_Get_Enabled_Color(1);
}

inline int Gadget_Button_Get_Enabled_Selected_Border_Color(GameWindow *push_button)
{
    return push_button->Win_Get_Enabled_Border_Color(1);
}

inline int Gadget_Button_Get_Disabled_Selected_Color(GameWindow *push_button)
{
    return push_button->Win_Get_Disabled_Color(1);
}

inline int Gadget_Button_Get_Disabled_Selected_Border_Color(GameWindow *push_button)
{
    return push_button->Win_Get_Disabled_Border_Color(1);
}

inline int Gadget_Button_Get_Hilite_Selected_Color(GameWindow *push_button)
{
    return push_button->Win_Get_Hilite_Color(1);
}

inline int Gadget_Button_Get_Hilite_Selected_Border_Color(GameWindow *push_button)
{
    return push_button->Win_Get_Hilite_Border_Color(1);
}

WindowMsgHandledType Gadget_Push_Button_Input(
    GameWindow *push_button, unsigned int message, unsigned int data_1, unsigned int data_2);

WindowMsgHandledType Gadget_Push_Button_System(
    GameWindow *push_button, unsigned int message, unsigned int data_1, unsigned int data_2);

void Gadget_Check_Like_Button_Set_Visual_Check(GameWindow *push_button, bool is_checked);
bool Gadget_Check_Like_Button_Is_Checked(GameWindow *push_button);
void Gadget_Button_Enable_Check_Like(GameWindow *push_button, bool is_enabled, bool is_checked);
void Gadget_Button_Set_Text(GameWindow *push_button, Utf16String text);
void Gadget_Button_Set_Border(GameWindow *push_button, int color, bool draw_border);
void Gadget_Button_Draw_Clock(GameWindow *push_button, int percent, int color);
void Gadget_Button_Draw_Inverse_Clock(GameWindow *push_button, int percent, int color);
void Gadget_Button_Draw_Overlay_Image(GameWindow *push_button, const Image *image);
void Gadget_Button_Set_Data(GameWindow *push_button, void *user_data);
void *Gadget_Button_Get_Data(GameWindow *push_button);
void Gadget_Button_Set_Alt_Sound(GameWindow *push_button, Utf8String alt_sound);
