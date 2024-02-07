/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Combo Box
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
#include "color.h"
#include "gamewindow.h"

struct _ListboxData;
struct _EntryData;

struct _ComboBoxData
{
    bool m_isEditable;
    int m_maxDisplay;
    int m_maxChars;
    bool m_asciiOnly;
    bool m_lettersAndNumbersOnly;
    _ListboxData *m_listboxData;
    _EntryData *m_entryData;
    bool m_dontHide;
    int m_entryCount;
    GameWindow *m_dropDownButton;
    GameWindow *m_editBox;
    GameWindow *m_listBox;
};

inline GameWindow *Gadget_Combo_Box_Get_List_Box(GameWindow *combo_box)
{
    _ComboBoxData *data = static_cast<_ComboBoxData *>(combo_box->Win_Get_User_Data());

    if (data != nullptr && data->m_listBox != nullptr) {
        return data->m_listBox;
    } else {
        return nullptr;
    }
}

inline GameWindow *Gadget_Combo_Box_Get_Edit_Box(GameWindow *combo_box)
{
    _ComboBoxData *data = static_cast<_ComboBoxData *>(combo_box->Win_Get_User_Data());

    if (data != nullptr && data->m_editBox != nullptr) {
        return data->m_editBox;
    } else {
        return nullptr;
    }
}

inline GameWindow *Gadget_Combo_Box_Get_Drop_Down_Button(GameWindow *combo_box)
{
    _ComboBoxData *data = static_cast<_ComboBoxData *>(combo_box->Win_Get_User_Data());

    if (data != nullptr && data->m_dropDownButton != nullptr) {
        return data->m_dropDownButton;
    } else {
        return nullptr;
    }
}

inline void Gadget_Combo_Box_Set_Enabled_Image(GameWindow *combo_box, const Image *image)
{
    combo_box->Win_Set_Enabled_Image(0, image);
}

inline void Gadget_Combo_Box_Set_Enabled_Selected_Item_Left_End_Image(GameWindow *combo_box, const Image *image)
{
    combo_box->Win_Set_Enabled_Image(1, image);
}

inline void Gadget_Combo_Box_Set_Enabled_Selected_Item_Right_End_Image(GameWindow *combo_box, const Image *image)
{
    combo_box->Win_Set_Enabled_Image(2, image);
}

inline void Gadget_Combo_Box_Set_Enabled_Selected_Item_Repeating_Center_Image(GameWindow *combo_box, const Image *image)
{
    combo_box->Win_Set_Enabled_Image(3, image);
}

inline void Gadget_Combo_Box_Set_Enabled_Selected_Item_Small_Repeating_Center_Image(
    GameWindow *combo_box, const Image *image)
{
    combo_box->Win_Set_Enabled_Image(4, image);
}

inline void Gadget_Combo_Box_Set_Disabled_Image(GameWindow *combo_box, const Image *image)
{
    combo_box->Win_Set_Disabled_Image(0, image);
}

inline void Gadget_Combo_Box_Set_Disabled_Selected_Item_Left_End_Image(GameWindow *combo_box, const Image *image)
{
    combo_box->Win_Set_Disabled_Image(1, image);
}

inline void Gadget_Combo_Box_Set_Disabled_Selected_Item_Right_End_Image(GameWindow *combo_box, const Image *image)
{
    combo_box->Win_Set_Disabled_Image(2, image);
}

inline void Gadget_Combo_Box_Set_Disabled_Selected_Item_Repeating_Center_Image(GameWindow *combo_box, const Image *image)
{
    combo_box->Win_Set_Disabled_Image(3, image);
}

inline void Gadget_Combo_Box_Set_Disabled_Selected_Item_Small_Repeating_Center_Image(
    GameWindow *combo_box, const Image *image)
{
    combo_box->Win_Set_Disabled_Image(4, image);
}

inline void Gadget_Combo_Box_Set_Hilite_Image(GameWindow *combo_box, const Image *image)
{
    combo_box->Win_Set_Hilite_Image(0, image);
}

inline void Gadget_Combo_Box_Set_Hilite_Selected_Item_Left_End_Image(GameWindow *combo_box, const Image *image)
{
    combo_box->Win_Set_Hilite_Image(1, image);
}

inline void Gadget_Combo_Box_Set_Hilite_Selected_Item_Right_End_Image(GameWindow *combo_box, const Image *image)
{
    combo_box->Win_Set_Hilite_Image(2, image);
}

inline void Gadget_Combo_Box_Set_Hilite_Selected_Item_Repeating_Center_Image(GameWindow *combo_box, const Image *image)
{
    combo_box->Win_Set_Hilite_Image(3, image);
}

inline void Gadget_Combo_Box_Set_Hilite_Selected_Item_Small_Repeating_Center_Image(GameWindow *combo_box, const Image *image)
{
    combo_box->Win_Set_Hilite_Image(4, image);
}

inline void Gadget_Combo_Box_Set_Text_Color(GameWindow *combo_box, int color)
{
    int border_color = combo_box->Win_Get_Enabled_Text_Border_Color();
    combo_box->Win_Set_Enabled_Text_Colors(color, border_color);
    combo_box->Win_Set_Disabled_Text_Colors(Darken_Color(color, 25), border_color);
}

inline void Gadget_Combo_Box_Set_Enabled_Color(GameWindow *combo_box, int color)
{
    combo_box->Win_Set_Enabled_Color(0, color);
}

inline void Gadget_Combo_Box_Set_Enabled_Border_Color(GameWindow *combo_box, int color)
{
    combo_box->Win_Set_Enabled_Border_Color(0, color);
}

inline void Gadget_Combo_Box_Set_Enabled_Selected_Item_Color(GameWindow *combo_box, int color)
{
    combo_box->Win_Set_Enabled_Color(1, color);
}

inline void Gadget_Combo_Box_Set_Enabled_Selected_Item_Border_Color(GameWindow *combo_box, int color)
{
    combo_box->Win_Set_Enabled_Border_Color(1, color);
}

inline void Gadget_Combo_Box_Set_Disabled_Color(GameWindow *combo_box, int color)
{
    combo_box->Win_Set_Disabled_Color(0, color);
}

inline void Gadget_Combo_Box_Set_Disabled_Border_Color(GameWindow *combo_box, int color)
{
    combo_box->Win_Set_Disabled_Border_Color(0, color);
}

inline void Gadget_Combo_Box_Set_Disabled_Selected_Item_Color(GameWindow *combo_box, int color)
{
    combo_box->Win_Set_Disabled_Color(1, color);
}

inline void Gadget_Combo_Box_Set_Disabled_Selected_Item_Border_Color(GameWindow *combo_box, int color)
{
    combo_box->Win_Set_Disabled_Border_Color(1, color);
}

inline void Gadget_Combo_Box_Set_Hilite_Color(GameWindow *combo_box, int color)
{
    combo_box->Win_Set_Hilite_Color(0, color);
}

inline void Gadget_Combo_Box_Set_Hilite_Border_Color(GameWindow *combo_box, int color)
{
    combo_box->Win_Set_Hilite_Border_Color(0, color);
}

inline void Gadget_Combo_Box_Set_Hilite_Selected_Item_Color(GameWindow *combo_box, int color)
{
    combo_box->Win_Set_Hilite_Color(1, color);
}

inline void Gadget_Combo_Box_Set_Hilite_Selected_Item_Border_Color(GameWindow *combo_box, int color)
{
    combo_box->Win_Set_Hilite_Border_Color(1, color);
}

WindowMsgHandledType Gadget_Combo_Box_Input(
    GameWindow *combo_box, unsigned int message, unsigned int data_1, unsigned int data_2);

WindowMsgHandledType Gadget_Combo_Box_System(
    GameWindow *combo_box, unsigned int message, unsigned int data_1, unsigned int data_2);

void Gadget_Combo_Box_Set_Colors(GameWindow *combo_box,
    int enabled_color,
    int enabled_border_color,
    int enabled_selected_item_color,
    int enabled_selected_item_border_color,
    int disabled_color,
    int disabled_border_color,
    int disabled_selected_item_color,
    int disabled_selected_item_border_color,
    int hilite_color,
    int hilite_border_color,
    int hilite_selected_item_color,
    int hilite_selected_item_border_color);

void Gadget_Combo_Box_Set_Enabled_Text_Colors(GameWindow *combo_box, int color, int border_color);
void Gadget_Combo_Box_Set_Disabled_Text_Colors(GameWindow *combo_box, int color, int border_color);
void Gadget_Combo_Box_Set_Hilite_Text_Colors(GameWindow *combo_box, int color, int border_color);
void Gadget_Combo_Box_Set_IME_Composite_Text_Colors(GameWindow *combo_box, int color, int border_color);
void Gadget_Combo_Box_Set_Font(GameWindow *combo_box, GameFont *font);
void Gadget_Combo_Box_Set_Is_Editable(GameWindow *combo_box, bool is_editable);
void Gadget_Combo_Box_Set_Letters_And_Numbers_Only(GameWindow *combo_box, bool letters_and_numbers_only);
void Gadget_Combo_Box_Set_Ascii_Only(GameWindow *combo_box, bool ascii_only);
void Gadget_Combo_Box_Set_Max_Chars(GameWindow *combo_box, int max_chars);
void Gadget_Combo_Box_Set_Max_Display(GameWindow *combo_box, int max_display);
Utf16String Gadget_Combo_Box_Get_Text(GameWindow *combo_box);
void Gadget_Combo_Box_Set_Text(GameWindow *combo_box, Utf16String text);
int Gadget_Combo_Box_Add_Entry(GameWindow *combo_box, Utf16String text, int color);
void Gadget_Combo_Box_Reset(GameWindow *combo_box);
void Gadget_Combo_Box_Hide_List(GameWindow *combo_box);
void Gadget_Combo_Box_Get_Selected_Pos(GameWindow *combo_box, int *selected_index);
void Gadget_Combo_Box_Set_Selected_Pos(GameWindow *combo_box, int selected_index, bool dont_hide);
void Gadget_Combo_Box_Set_Item_Data(GameWindow *combo_box, int index, void *data);
void *Gadget_Combo_Box_Get_Item_Data(GameWindow *combo_box, int index);
int Gadget_Combo_Box_Get_Length(GameWindow *combo_box);
