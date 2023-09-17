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
class GameWindow;
class GameFont;
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

void Gadget_Combo_Box_Set_Enabled_Text_Colors(GameWindow *combo_box, int color, int border_color);
void Gadget_Combo_Box_Set_Disabled_Text_Colors(GameWindow *combo_box, int color, int border_color);
void Gadget_Combo_Box_Set_Hilite_Text_Colors(GameWindow *combo_box, int color, int border_color);
void Gadget_Combo_Box_Set_IME_Composite_Text_Colors(GameWindow *combo_box, int color, int border_color);
void Gadget_Combo_Box_Set_Font(GameWindow *combo_box, GameFont *font);
GameWindow *Gadget_Combo_Box_Get_List_Box(GameWindow *combo_box);
GameWindow *Gadget_Combo_Box_Get_Edit_Box(GameWindow *combo_box);
