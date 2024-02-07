/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Text Entry
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
#include "gamewindowmanager.h"

struct _EntryData
{
    DisplayString *m_text;
    DisplayString *m_sText;
    DisplayString *m_constructText;
    bool m_secretText;
    bool m_numericalOnly;
    bool m_alphaNumericalOnly;
    bool m_asciiOnly;
    short m_maxTextLen;
    bool m_receivedUnichar;
    bool m_noInput;
    GameWindow *m_constructList;
    unsigned short m_charPos;
    unsigned short m_conCharPos;
};

inline void Gadget_Text_Entry_Set_Enabled_Item_Left_End_Image(GameWindow *text_entry, const Image *image)
{
    text_entry->Win_Set_Enabled_Image(0, image);
}

inline void Gadget_Text_Entry_Set_Enabled_Item_Right_End_Image(GameWindow *text_entry, const Image *image)
{
    text_entry->Win_Set_Enabled_Image(1, image);
}

inline void Gadget_Text_Entry_Set_Enabled_Item_Repeating_Center_Image(GameWindow *text_entry, const Image *image)
{
    text_entry->Win_Set_Enabled_Image(2, image);
}

inline void Gadget_Text_Entry_Set_Enabled_Item_Small_Repeating_Center_Image(GameWindow *text_entry, const Image *image)
{
    text_entry->Win_Set_Enabled_Image(3, image);
}

inline void Gadget_Text_Entry_Set_Disabled_Item_Left_End_Image(GameWindow *text_entry, const Image *image)
{
    text_entry->Win_Set_Disabled_Image(0, image);
}

inline void Gadget_Text_Entry_Set_Disabled_Item_Right_End_Image(GameWindow *text_entry, const Image *image)
{
    text_entry->Win_Set_Disabled_Image(1, image);
}

inline void Gadget_Text_Entry_Set_Disabled_Item_Repeating_Center_Image(GameWindow *text_entry, const Image *image)
{
    text_entry->Win_Set_Disabled_Image(2, image);
}

inline void Gadget_Text_Entry_Set_Disabled_Item_Small_Repeating_Center_Image(GameWindow *text_entry, const Image *image)
{
    text_entry->Win_Set_Disabled_Image(3, image);
}

inline void Gadget_Text_Entry_Set_Hilite_Item_Left_End_Image(GameWindow *text_entry, const Image *image)
{
    text_entry->Win_Set_Hilite_Image(0, image);
}

inline void Gadget_Text_Entry_Set_Hilite_Item_Right_End_Image(GameWindow *text_entry, const Image *image)
{
    text_entry->Win_Set_Hilite_Image(1, image);
}

inline void Gadget_Text_Entry_Set_Hilite_Item_Repeating_Center_Image(GameWindow *text_entry, const Image *image)
{
    text_entry->Win_Set_Hilite_Image(2, image);
}

inline void Gadget_Text_Entry_Set_Hilite_Item_Small_Repeating_Center_Image(GameWindow *text_entry, const Image *image)
{
    text_entry->Win_Set_Hilite_Image(3, image);
}

inline void Gadget_Text_Entry_Set_Enabled_Color(GameWindow *text_entry, int color)
{
    text_entry->Win_Set_Enabled_Color(0, color);
}

inline void Gadget_Text_Entry_Set_Enabled_Border_Color(GameWindow *text_entry, int color)
{
    text_entry->Win_Set_Enabled_Border_Color(0, color);
}

inline void Gadget_Text_Entry_Set_Disabled_Color(GameWindow *text_entry, int color)
{
    text_entry->Win_Set_Disabled_Color(0, color);
}

inline void Gadget_Text_Entry_Set_Disabled_Border_Color(GameWindow *text_entry, int color)
{
    text_entry->Win_Set_Disabled_Border_Color(0, color);
}

inline void Gadget_Text_Entry_Set_Hilite_Color(GameWindow *text_entry, int color)
{
    text_entry->Win_Set_Hilite_Color(0, color);
}

inline void Gadget_Text_Entry_Set_Hilite_Border_Color(GameWindow *text_entry, int color)
{
    text_entry->Win_Set_Hilite_Border_Color(0, color);
}

inline void Gadget_Text_Entry_Set_Text(GameWindow *text_entry, Utf16String text)
{
#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
    g_theWindowManager->Win_Send_System_Msg(text_entry, GEM_SET_TEXT, reinterpret_cast<unsigned int>(&text), 0);
#endif
}

WindowMsgHandledType Gadget_Text_Entry_Input(
    GameWindow *text_entry, unsigned int message, unsigned int data_1, unsigned int data_2);

WindowMsgHandledType Gadget_Text_Entry_System(
    GameWindow *text_entry, unsigned int message, unsigned int data_1, unsigned int data_2);

void Gadget_Text_Entry_Set_Font(GameWindow *text_entry, GameFont *font);

static Utf16String Gadget_Text_Entry_Get_Text(GameWindow *text_entry)
{
    if (text_entry != nullptr) {
        if ((text_entry->Win_Get_Style() & GWS_ENTRY_FIELD) != 0) {
            Utf16String str;
#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
            g_theWindowManager->Win_Send_System_Msg(text_entry, GEM_GET_TEXT, 0, reinterpret_cast<unsigned int>(&str));
#endif
            return str;
        } else {
            return Utf16String::s_emptyString;
        }
    } else {
        return Utf16String::s_emptyString;
    }
}
