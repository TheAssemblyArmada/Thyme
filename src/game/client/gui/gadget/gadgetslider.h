/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Slider
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
#include "gadgetpushbutton.h"
#include "gamewindow.h"

struct _SliderData
{
    int m_minVal;
    int m_maxVal;
    float m_numTicks;
    int m_position;
};

inline void Gadget_Slider_Set_Enabled_Left_End_Image(GameWindow *slider, const Image *image)
{
    slider->Win_Set_Enabled_Image(0, image);
}

inline void Gadget_Slider_Set_Enabled_Top_End_Image(GameWindow *slider, const Image *image)
{
    slider->Win_Set_Enabled_Image(0, image);
}

inline void Gadget_Slider_Set_Enabled_Right_End_Image(GameWindow *slider, const Image *image)
{
    slider->Win_Set_Enabled_Image(1, image);
}

inline void Gadget_Slider_Set_Enabled_Bottom_End_Image(GameWindow *slider, const Image *image)
{
    slider->Win_Set_Enabled_Image(1, image);
}

inline void Gadget_Slider_Set_Enabled_Repeating_Centre_Image(GameWindow *slider, const Image *image)
{
    slider->Win_Set_Enabled_Image(2, image);
}

inline void Gadget_Slider_Set_Enabled_Small_Repeating_Centre_Image(GameWindow *slider, const Image *image)
{
    slider->Win_Set_Enabled_Image(3, image);
}

inline void Gadget_Slider_Set_Disabled_Left_End_Image(GameWindow *slider, const Image *image)
{
    slider->Win_Set_Disabled_Image(0, image);
}

inline void Gadget_Slider_Set_Disabled_Top_End_Image(GameWindow *slider, const Image *image)
{
    slider->Win_Set_Disabled_Image(0, image);
}

inline void Gadget_Slider_Set_Disabled_Right_End_Image(GameWindow *slider, const Image *image)
{
    slider->Win_Set_Disabled_Image(1, image);
}

inline void Gadget_Slider_Set_Disabled_Bottom_End_Image(GameWindow *slider, const Image *image)
{
    slider->Win_Set_Disabled_Image(1, image);
}

inline void Gadget_Slider_Set_Disabled_Repeating_Centre_Image(GameWindow *slider, const Image *image)
{
    slider->Win_Set_Disabled_Image(2, image);
}

inline void Gadget_Slider_Set_Disabled_Small_Repeating_Centre_Image(GameWindow *slider, const Image *image)
{
    slider->Win_Set_Disabled_Image(3, image);
}

inline void Gadget_Slider_Set_Hilite_Left_End_Image(GameWindow *slider, const Image *image)
{
    slider->Win_Set_Hilite_Image(0, image);
}

inline void Gadget_Slider_Set_Hilite_Top_End_Image(GameWindow *slider, const Image *image)
{
    slider->Win_Set_Hilite_Image(0, image);
}

inline void Gadget_Slider_Set_Hilite_Right_End_Image(GameWindow *slider, const Image *image)
{
    slider->Win_Set_Hilite_Image(1, image);
}

inline void Gadget_Slider_Set_Hilite_Bottom_End_Image(GameWindow *slider, const Image *image)
{
    slider->Win_Set_Hilite_Image(1, image);
}

inline void Gadget_Slider_Set_Hilite_Repeating_Centre_Image(GameWindow *slider, const Image *image)
{
    slider->Win_Set_Hilite_Image(2, image);
}

inline void Gadget_Slider_Set_Hilite_Small_Repeating_Centre_Image(GameWindow *slider, const Image *image)
{
    slider->Win_Set_Hilite_Image(3, image);
}

inline void Gadget_Slider_Set_Thumb_Enabled_Image(GameWindow *slider, const Image *image)
{
    GameWindow *thumb = slider->Win_Get_Child();

    if (thumb != nullptr) {
        Gadget_Button_Set_Enabled_Image(thumb, image);
    }
}

inline void Gadget_Slider_Set_Thumb_Enabled_Selected_Image(GameWindow *slider, const Image *image)
{
    GameWindow *thumb = slider->Win_Get_Child();

    if (thumb != nullptr) {
        Gadget_Button_Set_Enabled_Selected_Image(thumb, image);
    }
}

inline void Gadget_Slider_Set_Thumb_Disabled_Image(GameWindow *slider, const Image *image)
{
    GameWindow *thumb = slider->Win_Get_Child();

    if (thumb != nullptr) {
        Gadget_Button_Set_Disabled_Image(thumb, image);
    }
}

inline void Gadget_Slider_Set_Thumb_Disabled_Selected_Image(GameWindow *slider, const Image *image)
{
    GameWindow *thumb = slider->Win_Get_Child();

    if (thumb != nullptr) {
        Gadget_Button_Set_Disabled_Selected_Image(thumb, image);
    }
}

inline void Gadget_Slider_Set_Thumb_Hilite_Image(GameWindow *slider, const Image *image)
{
    GameWindow *thumb = slider->Win_Get_Child();

    if (thumb != nullptr) {
        Gadget_Button_Set_Hilite_Image(thumb, image);
    }
}

inline void Gadget_Slider_Set_Thumb_Hilite_Selected_Image(GameWindow *slider, const Image *image)
{
    GameWindow *thumb = slider->Win_Get_Child();

    if (thumb != nullptr) {
        Gadget_Button_Set_Hilite_Selected_Image(thumb, image);
    }
}

inline void Gadget_Slider_Set_Enabled_Color(GameWindow *slider, int color)
{
    slider->Win_Set_Enabled_Color(0, color);
}

inline void Gadget_Slider_Set_Enabled_Border_Color(GameWindow *slider, int color)
{
    slider->Win_Set_Enabled_Border_Color(0, color);
}

inline int Gadget_Slider_Get_Enabled_Color(GameWindow *slider)
{
    return slider->Win_Get_Enabled_Color(0);
}

inline int Gadget_Slider_Get_Enabled_Border_Color(GameWindow *slider)
{
    return slider->Win_Get_Enabled_Border_Color(0);
}

inline void Gadget_Slider_Set_Disabled_Color(GameWindow *slider, int color)
{
    slider->Win_Set_Disabled_Color(0, color);
}

inline void Gadget_Slider_Set_Disabled_Border_Color(GameWindow *slider, int color)
{
    slider->Win_Set_Disabled_Border_Color(0, color);
}

inline int Gadget_Slider_Get_Disabled_Color(GameWindow *slider)
{
    return slider->Win_Get_Disabled_Color(0);
}

inline int Gadget_Slider_Get_Disabled_Border_Color(GameWindow *slider)
{
    return slider->Win_Get_Disabled_Border_Color(0);
}

inline void Gadget_Slider_Set_Hilite_Color(GameWindow *slider, int color)
{
    slider->Win_Set_Hilite_Color(0, color);
}

inline void Gadget_Slider_Set_Hilite_Border_Color(GameWindow *slider, int color)
{
    slider->Win_Set_Hilite_Border_Color(0, color);
}

inline int Gadget_Slider_Get_Hilite_Color(GameWindow *slider)
{
    return slider->Win_Get_Hilite_Color(0);
}

inline int Gadget_Slider_Get_Hilite_Border_Color(GameWindow *slider)
{
    return slider->Win_Get_Hilite_Border_Color(0);
}

inline void Gadget_Slider_Set_Thumb_Enabled_Color(GameWindow *slider, int color)
{
    GameWindow *thumb = slider->Win_Get_Child();

    if (thumb != nullptr) {
        Gadget_Button_Set_Enabled_Color(thumb, color);
    }
}

inline void Gadget_Slider_Set_Thumb_Enabled_Border_Color(GameWindow *slider, int color)
{
    GameWindow *thumb = slider->Win_Get_Child();

    if (thumb != nullptr) {
        Gadget_Button_Set_Enabled_Border_Color(thumb, color);
    }
}

inline void Gadget_Slider_Set_Thumb_Enabled_Selected_Color(GameWindow *slider, int color)
{
    GameWindow *thumb = slider->Win_Get_Child();

    if (thumb != nullptr) {
        Gadget_Button_Set_Enabled_Selected_Color(thumb, color);
    }
}

inline void Gadget_Slider_Set_Thumb_Enabled_Selected_Border_Color(GameWindow *slider, int color)
{
    GameWindow *thumb = slider->Win_Get_Child();

    if (thumb != nullptr) {
        Gadget_Button_Set_Enabled_Selected_Border_Color(thumb, color);
    }
}

inline void Gadget_Slider_Set_Thumb_Disabled_Color(GameWindow *slider, int color)
{
    GameWindow *thumb = slider->Win_Get_Child();

    if (thumb != nullptr) {
        Gadget_Button_Set_Disabled_Color(thumb, color);
    }
}

inline void Gadget_Slider_Set_Thumb_Disabled_Border_Color(GameWindow *slider, int color)
{
    GameWindow *thumb = slider->Win_Get_Child();

    if (thumb != nullptr) {
        Gadget_Button_Set_Disabled_Border_Color(thumb, color);
    }
}

inline void Gadget_Slider_Set_Thumb_Disabled_Selected_Color(GameWindow *slider, int color)
{
    GameWindow *thumb = slider->Win_Get_Child();

    if (thumb != nullptr) {
        Gadget_Button_Set_Disabled_Selected_Color(thumb, color);
    }
}

inline void Gadget_Slider_Set_Thumb_Disabled_Selected_Border_Color(GameWindow *slider, int color)
{
    GameWindow *thumb = slider->Win_Get_Child();

    if (thumb != nullptr) {
        Gadget_Button_Set_Disabled_Selected_Border_Color(thumb, color);
    }
}

inline void Gadget_Slider_Set_Thumb_Hilite_Color(GameWindow *slider, int color)
{
    GameWindow *thumb = slider->Win_Get_Child();

    if (thumb != nullptr) {
        Gadget_Button_Set_Hilite_Color(thumb, color);
    }
}

inline void Gadget_Slider_Set_Thumb_Hilite_Border_Color(GameWindow *slider, int color)
{
    GameWindow *thumb = slider->Win_Get_Child();

    if (thumb != nullptr) {
        Gadget_Button_Set_Hilite_Border_Color(thumb, color);
    }
}

inline void Gadget_Slider_Set_Thumb_Hilite_Selected_Color(GameWindow *slider, int color)
{
    GameWindow *thumb = slider->Win_Get_Child();

    if (thumb != nullptr) {
        Gadget_Button_Set_Hilite_Selected_Color(thumb, color);
    }
}

inline void Gadget_Slider_Set_Thumb_Hilite_Selected_Border_Color(GameWindow *slider, int color)
{
    GameWindow *thumb = slider->Win_Get_Child();

    if (thumb != nullptr) {
        Gadget_Button_Set_Hilite_Selected_Border_Color(thumb, color);
    }
}

inline int Gadget_Slider_Get_Thumb_Enabled_Selected_Color(GameWindow *slider)
{
    GameWindow *thumb = slider->Win_Get_Child();

    if (thumb != nullptr) {
        return Gadget_Button_Get_Enabled_Selected_Color(thumb);
    } else {
        return 0xFFFFFF;
    }
}

inline int Gadget_Slider_Get_Thumb_Enabled_Selected_Border_Color(GameWindow *slider)
{
    GameWindow *thumb = slider->Win_Get_Child();

    if (thumb != nullptr) {
        return Gadget_Button_Get_Enabled_Selected_Border_Color(thumb);
    } else {
        return 0xFFFFFF;
    }
}

inline int Gadget_Slider_Get_Thumb_Disabled_Selected_Color(GameWindow *slider)
{
    GameWindow *thumb = slider->Win_Get_Child();

    if (thumb != nullptr) {
        return Gadget_Button_Get_Disabled_Selected_Color(thumb);
    } else {
        return 0xFFFFFF;
    }
}

inline int Gadget_Slider_Get_Thumb_Disabled_Selected_Border_Color(GameWindow *slider)
{
    GameWindow *thumb = slider->Win_Get_Child();

    if (thumb != nullptr) {
        return Gadget_Button_Get_Disabled_Selected_Border_Color(thumb);
    } else {
        return 0xFFFFFF;
    }
}

inline int Gadget_Slider_Get_Thumb_Hilite_Selected_Color(GameWindow *slider)
{
    GameWindow *thumb = slider->Win_Get_Child();

    if (thumb != nullptr) {
        return Gadget_Button_Get_Hilite_Selected_Color(thumb);
    } else {
        return 0xFFFFFF;
    }
}

inline int Gadget_Slider_Get_Thumb_Hilite_Selected_Border_Color(GameWindow *slider)
{
    GameWindow *thumb = slider->Win_Get_Child();

    if (thumb != nullptr) {
        return Gadget_Button_Get_Hilite_Selected_Border_Color(thumb);
    } else {
        return 0xFFFFFF;
    }
}

WindowMsgHandledType Gadget_Vertical_Slider_Input(
    GameWindow *slider, unsigned int message, unsigned int data_1, unsigned int data_2);

WindowMsgHandledType Gadget_Vertical_Slider_System(
    GameWindow *slider, unsigned int message, unsigned int data_1, unsigned int data_2);

WindowMsgHandledType Gadget_Horizontal_Slider_Input(
    GameWindow *slider, unsigned int message, unsigned int data_1, unsigned int data_2);

WindowMsgHandledType Gadget_Horizontal_Slider_System(
    GameWindow *slider, unsigned int message, unsigned int data_1, unsigned int data_2);
