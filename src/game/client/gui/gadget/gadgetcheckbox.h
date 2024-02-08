/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Check Box
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

inline void Gadget_Check_Box_Set_Enabled_Image(GameWindow *check_box, const Image *image)
{
    check_box->Win_Set_Enabled_Image(0, image);
}

inline void Gadget_Check_Box_Set_Enabled_Box_Unselected_Image(GameWindow *check_box, const Image *image)
{
    check_box->Win_Set_Enabled_Image(1, image);
}

inline void Gadget_Check_Box_Set_Enabled_Box_Selected_Image(GameWindow *check_box, const Image *image)
{
    check_box->Win_Set_Enabled_Image(2, image);
}

inline void Gadget_Check_Box_Set_Disabled_Image(GameWindow *check_box, const Image *image)
{
    check_box->Win_Set_Disabled_Image(0, image);
}

inline void Gadget_Check_Box_Set_Disabled_Box_Unselected_Image(GameWindow *check_box, const Image *image)
{
    check_box->Win_Set_Disabled_Image(1, image);
}

inline void Gadget_Check_Box_Set_Disabled_Box_Selected_Image(GameWindow *check_box, const Image *image)
{
    check_box->Win_Set_Disabled_Image(2, image);
}

inline void Gadget_Check_Box_Set_Hilite_Image(GameWindow *check_box, const Image *image)
{
    check_box->Win_Set_Hilite_Image(0, image);
}

inline void Gadget_Check_Box_Set_Hilite_Box_Unselected_Image(GameWindow *check_box, const Image *image)
{
    check_box->Win_Set_Hilite_Image(1, image);
}

inline void Gadget_Check_Box_Set_Hilite_Box_Selected_Image(GameWindow *check_box, const Image *image)
{
    check_box->Win_Set_Hilite_Image(2, image);
}

inline void Gadget_Check_Box_Set_Enabled_Color(GameWindow *check_box, int color)
{
    check_box->Win_Set_Enabled_Color(0, color);
}

inline void Gadget_Check_Box_Set_Enabled_Border_Color(GameWindow *check_box, int color)
{
    check_box->Win_Set_Enabled_Border_Color(0, color);
}

inline void Gadget_Check_Box_Set_Enabled_Box_Unselected_Color(GameWindow *check_box, int color)
{
    check_box->Win_Set_Enabled_Color(1, color);
}

inline void Gadget_Check_Box_Set_Enabled_Box_Unselected_Border_Color(GameWindow *check_box, int color)
{
    check_box->Win_Set_Enabled_Border_Color(1, color);
}

inline void Gadget_Check_Box_Set_Enabled_Box_Selected_Color(GameWindow *check_box, int color)
{
    check_box->Win_Set_Enabled_Color(2, color);
}

inline void Gadget_Check_Box_Set_Enabled_Box_Selected_Border_Color(GameWindow *check_box, int color)
{
    check_box->Win_Set_Enabled_Border_Color(2, color);
}

inline void Gadget_Check_Box_Set_Disabled_Color(GameWindow *check_box, int color)
{
    check_box->Win_Set_Disabled_Color(0, color);
}

inline void Gadget_Check_Box_Set_Disabled_Border_Color(GameWindow *check_box, int color)
{
    check_box->Win_Set_Disabled_Border_Color(0, color);
}

inline void Gadget_Check_Box_Set_Disabled_Box_Unselected_Color(GameWindow *check_box, int color)
{
    check_box->Win_Set_Disabled_Color(1, color);
}

inline void Gadget_Check_Box_Set_Disabled_Box_Unselected_Border_Color(GameWindow *check_box, int color)
{
    check_box->Win_Set_Disabled_Border_Color(1, color);
}

inline void Gadget_Check_Box_Set_Disabled_Box_Selected_Color(GameWindow *check_box, int color)
{
    check_box->Win_Set_Disabled_Color(2, color);
}

inline void Gadget_Check_Box_Set_Disabled_Box_Selected_Border_Color(GameWindow *check_box, int color)
{
    check_box->Win_Set_Disabled_Border_Color(2, color);
}

inline void Gadget_Check_Box_Set_Hilite_Color(GameWindow *check_box, int color)
{
    check_box->Win_Set_Hilite_Color(0, color);
}

inline void Gadget_Check_Box_Set_Hilite_Border_Color(GameWindow *check_box, int color)
{
    check_box->Win_Set_Hilite_Border_Color(0, color);
}

inline void Gadget_Check_Box_Set_Hilite_Box_Unselected_Color(GameWindow *check_box, int color)
{
    check_box->Win_Set_Hilite_Color(1, color);
}

inline void Gadget_Check_Box_Set_Hilite_Box_Unselected_Border_Color(GameWindow *check_box, int color)
{
    check_box->Win_Set_Hilite_Border_Color(1, color);
}

inline void Gadget_Check_Box_Set_Hilite_Box_Selected_Color(GameWindow *check_box, int color)
{
    check_box->Win_Set_Hilite_Color(2, color);
}

inline void Gadget_Check_Box_Set_Hilite_Box_Selected_Border_Color(GameWindow *check_box, int color)
{
    check_box->Win_Set_Hilite_Border_Color(2, color);
}

WindowMsgHandledType Gadget_Check_Box_Input(
    GameWindow *check_box, unsigned int message, unsigned int data_1, unsigned int data_2);

WindowMsgHandledType Gadget_Check_Box_System(
    GameWindow *check_box, unsigned int message, unsigned int data_1, unsigned int data_2);

void Gadget_Check_Box_Set_Text(GameWindow *check_box, Utf16String text);
void Gadget_Check_Box_Set_Checked(GameWindow *check_box, bool is_checked);
void Gadget_Check_Box_Toggle(GameWindow *check_box);
bool Gadget_Check_Box_Is_Checked(GameWindow *check_box);
