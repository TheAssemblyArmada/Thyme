/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Progress Bar
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

inline void Gadget_Progress_Bar_Set_Enabled_Left_End_Image(GameWindow *progress_bar, const Image *image)
{
    progress_bar->Win_Set_Enabled_Image(0, image);
}

inline void Gadget_Progress_Bar_Set_Enabled_Right_End_Image(GameWindow *progress_bar, const Image *image)
{
    progress_bar->Win_Set_Enabled_Image(1, image);
}

inline void Gadget_Progress_Bar_Set_Enabled_Repeating_Center_Image(GameWindow *progress_bar, const Image *image)
{
    progress_bar->Win_Set_Enabled_Image(2, image);
}

inline void Gadget_Progress_Bar_Set_Enabled_Small_Repeating_Center_Image(GameWindow *progress_bar, const Image *image)
{
    progress_bar->Win_Set_Enabled_Image(3, image);
}

inline void Gadget_Progress_Bar_Set_Enabled_Bar_Left_End_Image(GameWindow *progress_bar, const Image *image)
{
    progress_bar->Win_Set_Enabled_Image(4, image);
}

inline void Gadget_Progress_Bar_Set_Enabled_Bar_Right_End_Image(GameWindow *progress_bar, const Image *image)
{
    progress_bar->Win_Set_Enabled_Image(5, image);
}

inline void Gadget_Progress_Bar_Set_Enabled_Bar_Repeating_Center_Image(GameWindow *progress_bar, const Image *image)
{
    progress_bar->Win_Set_Enabled_Image(6, image);
}

inline void Gadget_Progress_Bar_Set_Enabled_Bar_Small_Repeating_Center_Image(GameWindow *progress_bar, const Image *image)
{
    progress_bar->Win_Set_Enabled_Image(7, image);
}

inline void Gadget_Progress_Bar_Set_Disabled_Left_End_Image(GameWindow *progress_bar, const Image *image)
{
    progress_bar->Win_Set_Disabled_Image(0, image);
}

inline void Gadget_Progress_Bar_Set_Disabled_Right_End_Image(GameWindow *progress_bar, const Image *image)
{
    progress_bar->Win_Set_Disabled_Image(1, image);
}

inline void Gadget_Progress_Bar_Set_Disabled_Repeating_Center_Image(GameWindow *progress_bar, const Image *image)
{
    progress_bar->Win_Set_Disabled_Image(2, image);
}

inline void Gadget_Progress_Bar_Set_Disabled_Small_Repeating_Center_Image(GameWindow *progress_bar, const Image *image)
{
    progress_bar->Win_Set_Disabled_Image(3, image);
}

inline void Gadget_Progress_Bar_Set_Disabled_Bar_Left_End_Image(GameWindow *progress_bar, const Image *image)
{
    progress_bar->Win_Set_Disabled_Image(4, image);
}

inline void Gadget_Progress_Bar_Set_Disabled_Bar_Right_End_Image(GameWindow *progress_bar, const Image *image)
{
    progress_bar->Win_Set_Disabled_Image(5, image);
}

inline void Gadget_Progress_Bar_Set_Disabled_Bar_Repeating_Center_Image(GameWindow *progress_bar, const Image *image)
{
    progress_bar->Win_Set_Disabled_Image(6, image);
}

inline void Gadget_Progress_Bar_Set_Disabled_Bar_Small_Repeating_Center_Image(GameWindow *progress_bar, const Image *image)
{
    progress_bar->Win_Set_Disabled_Image(7, image);
}

inline void Gadget_Progress_Bar_Set_Hilite_Left_End_Image(GameWindow *progress_bar, const Image *image)
{
    progress_bar->Win_Set_Hilite_Image(0, image);
}

inline void Gadget_Progress_Bar_Set_Hilite_Right_End_Image(GameWindow *progress_bar, const Image *image)
{
    progress_bar->Win_Set_Hilite_Image(1, image);
}

inline void Gadget_Progress_Bar_Set_Hilite_Repeating_Center_Image(GameWindow *progress_bar, const Image *image)
{
    progress_bar->Win_Set_Hilite_Image(2, image);
}

inline void Gadget_Progress_Bar_Set_Hilite_Small_Repeating_Center_Image(GameWindow *progress_bar, const Image *image)
{
    progress_bar->Win_Set_Hilite_Image(3, image);
}

inline void Gadget_Progress_Bar_Set_Hilite_Bar_Left_End_Image(GameWindow *progress_bar, const Image *image)
{
    progress_bar->Win_Set_Hilite_Image(4, image);
}

inline void Gadget_Progress_Bar_Set_Hilite_Bar_Right_End_Image(GameWindow *progress_bar, const Image *image)
{
    progress_bar->Win_Set_Hilite_Image(5, image);
}

inline void Gadget_Progress_Bar_Set_Hilite_Bar_Repeating_Center_Image(GameWindow *progress_bar, const Image *image)
{
    progress_bar->Win_Set_Hilite_Image(6, image);
}

inline void Gadget_Progress_Bar_Set_Hilite_Bar_Small_Repeating_Center_Image(GameWindow *progress_bar, const Image *image)
{
    progress_bar->Win_Set_Hilite_Image(7, image);
}

inline void Gadget_Progress_Bar_Set_Enabled_Color(GameWindow *progress_bar, int color)
{
    progress_bar->Win_Set_Enabled_Color(0, color);
}

inline void Gadget_Progress_Bar_Set_Enabled_Border_Color(GameWindow *progress_bar, int color)
{
    progress_bar->Win_Set_Enabled_Border_Color(0, color);
}

inline void Gadget_Progress_Bar_Set_Enabled_Bar_Color(GameWindow *progress_bar, int color)
{
    progress_bar->Win_Set_Enabled_Color(4, color);
}

inline void Gadget_Progress_Bar_Set_Enabled_Bar_Border_Color(GameWindow *progress_bar, int color)
{
    progress_bar->Win_Set_Enabled_Border_Color(4, color);
}

inline void Gadget_Progress_Bar_Set_Disabled_Color(GameWindow *progress_bar, int color)
{
    progress_bar->Win_Set_Disabled_Color(0, color);
}

inline void Gadget_Progress_Bar_Set_Disabled_Border_Color(GameWindow *progress_bar, int color)
{
    progress_bar->Win_Set_Disabled_Border_Color(0, color);
}

inline void Gadget_Progress_Bar_Set_Disabled_Bar_Color(GameWindow *progress_bar, int color)
{
    progress_bar->Win_Set_Disabled_Color(4, color);
}

inline void Gadget_Progress_Bar_Set_Disabled_Bar_Border_Color(GameWindow *progress_bar, int color)
{
    progress_bar->Win_Set_Disabled_Border_Color(4, color);
}

inline void Gadget_Progress_Bar_Set_Hilite_Color(GameWindow *progress_bar, int color)
{
    progress_bar->Win_Set_Hilite_Color(0, color);
}

inline void Gadget_Progress_Bar_Set_Hilite_Border_Color(GameWindow *progress_bar, int color)
{
    progress_bar->Win_Set_Hilite_Border_Color(0, color);
}

inline void Gadget_Progress_Bar_Set_Hilite_Bar_Color(GameWindow *progress_bar, int color)
{
    progress_bar->Win_Set_Hilite_Color(4, color);
}

inline void Gadget_Progress_Bar_Set_Hilite_Bar_Border_Color(GameWindow *progress_bar, int color)
{
    progress_bar->Win_Set_Hilite_Border_Color(4, color);
}

WindowMsgHandledType Gadget_Progress_Bar_System(
    GameWindow *progress_bar, unsigned int message, unsigned int data_1, unsigned int data_2);

void Gadget_Progress_Bar_Set_Progress(GameWindow *progress_bar, int progress);
