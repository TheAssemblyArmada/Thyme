/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Radio Button
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

struct _RadioButtonData
{
    int m_screen;
    int m_group;
};

inline void Gadget_Radio_Button_Set_Enabled_Image(GameWindow *radio_button, const Image *image)
{
    radio_button->Win_Set_Enabled_Image(0, image);
}

inline void Gadget_Radio_Button_Set_Enabled_Box_Unselected_Image(GameWindow *radio_button, const Image *image)
{
    radio_button->Win_Set_Enabled_Image(1, image);
}

inline void Gadget_Radio_Button_Set_Enabled_Box_Selected_Image(GameWindow *radio_button, const Image *image)
{
    radio_button->Win_Set_Enabled_Image(2, image);
}

inline void Gadget_Radio_Button_Set_Disabled_Image(GameWindow *radio_button, const Image *image)
{
    radio_button->Win_Set_Disabled_Image(0, image);
}

inline void Gadget_Radio_Button_Set_Disabled_Box_Unselected_Image(GameWindow *radio_button, const Image *image)
{
    radio_button->Win_Set_Disabled_Image(1, image);
}

inline void Gadget_Radio_Button_Set_Disabled_Box_Selected_Image(GameWindow *radio_button, const Image *image)
{
    radio_button->Win_Set_Disabled_Image(2, image);
}

inline void Gadget_Radio_Button_Set_Hilite_Image(GameWindow *radio_button, const Image *image)
{
    radio_button->Win_Set_Hilite_Image(0, image);
}

inline void Gadget_Radio_Button_Set_Hilite_Box_Unselected_Image(GameWindow *radio_button, const Image *image)
{
    radio_button->Win_Set_Hilite_Image(1, image);
}

inline void Gadget_Radio_Button_Set_Hilite_Box_Selected_Image(GameWindow *radio_button, const Image *image)
{
    radio_button->Win_Set_Hilite_Image(2, image);
}

inline void Gadget_Radio_Button_Set_Enabled_Color(GameWindow *radio_button, int color)
{
    radio_button->Win_Set_Enabled_Color(0, color);
}

inline void Gadget_Radio_Button_Set_Enabled_Border_Color(GameWindow *radio_button, int color)
{
    radio_button->Win_Set_Enabled_Border_Color(0, color);
}

inline void Gadget_Radio_Button_Set_Enabled_Box_Unselected_Color(GameWindow *radio_button, int color)
{
    radio_button->Win_Set_Enabled_Color(1, color);
}

inline void Gadget_Radio_Button_Set_Enabled_Box_Unselected_Border_Color(GameWindow *radio_button, int color)
{
    radio_button->Win_Set_Enabled_Border_Color(1, color);
}

inline void Gadget_Radio_Button_Set_Enabled_Box_Selected_Color(GameWindow *radio_button, int color)
{
    radio_button->Win_Set_Enabled_Color(2, color);
}

inline void Gadget_Radio_Button_Set_Enabled_Box_Selected_Border_Color(GameWindow *radio_button, int color)
{
    radio_button->Win_Set_Enabled_Border_Color(2, color);
}

inline void Gadget_Radio_Button_Set_Disabled_Color(GameWindow *radio_button, int color)
{
    radio_button->Win_Set_Disabled_Color(0, color);
}

inline void Gadget_Radio_Button_Set_Disabled_Border_Color(GameWindow *radio_button, int color)
{
    radio_button->Win_Set_Disabled_Border_Color(0, color);
}

inline void Gadget_Radio_Button_Set_Disabled_Box_Unselected_Color(GameWindow *radio_button, int color)
{
    radio_button->Win_Set_Disabled_Color(1, color);
}

inline void Gadget_Radio_Button_Set_Disabled_Box_Unselected_Border_Color(GameWindow *radio_button, int color)
{
    radio_button->Win_Set_Disabled_Border_Color(1, color);
}

inline void Gadget_Radio_Button_Set_Disabled_Box_Selected_Color(GameWindow *radio_button, int color)
{
    radio_button->Win_Set_Disabled_Color(2, color);
}

inline void Gadget_Radio_Button_Set_Disabled_Box_Selected_Border_Color(GameWindow *radio_button, int color)
{
    radio_button->Win_Set_Disabled_Border_Color(2, color);
}

inline void Gadget_Radio_Button_Set_Hilite_Color(GameWindow *radio_button, int color)
{
    radio_button->Win_Set_Hilite_Color(0, color);
}

inline void Gadget_Radio_Button_Set_Hilite_Border_Color(GameWindow *radio_button, int color)
{
    radio_button->Win_Set_Hilite_Border_Color(0, color);
}

inline void Gadget_Radio_Button_Set_Hilite_Box_Unselected_Color(GameWindow *radio_button, int color)
{
    radio_button->Win_Set_Hilite_Color(1, color);
}

inline void Gadget_Radio_Button_Set_Hilite_Box_Unselected_Border_Color(GameWindow *radio_button, int color)
{
    radio_button->Win_Set_Hilite_Border_Color(1, color);
}

inline void Gadget_Radio_Button_Set_Hilite_Box_Selected_Color(GameWindow *radio_button, int color)
{
    radio_button->Win_Set_Hilite_Color(2, color);
}

inline void Gadget_Radio_Button_Set_Hilite_Box_Selected_Border_Color(GameWindow *radio_button, int color)
{
    radio_button->Win_Set_Hilite_Border_Color(2, color);
}

WindowMsgHandledType Gadget_Radio_Button_Input(
    GameWindow *radio_button, unsigned int message, unsigned int data_1, unsigned int data_2);

WindowMsgHandledType Gadget_Radio_Button_System(
    GameWindow *radio_button, unsigned int message, unsigned int data_1, unsigned int data_2);

void Gadget_Radio_Set_Text(GameWindow *radio_button, Utf16String text);
void Gadget_Radio_Set_Group(GameWindow *radio_button, int group, int screen);
void Gadget_Radio_Set_Selection(GameWindow *radio_button, bool send_msg);
