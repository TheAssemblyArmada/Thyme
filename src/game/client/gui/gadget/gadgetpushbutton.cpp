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
#include "gadgetpushbutton.h"
#include "gamewindowmanager.h"

void Gadget_Button_Set_Text(GameWindow *push_button, Utf16String text)
{
#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
    g_theWindowManager->Win_Send_System_Msg(push_button, GGM_SET_LABEL, reinterpret_cast<unsigned int>(&text), 0);
#endif
}

WindowMsgHandledType Gadget_Push_Button_Input(
    GameWindow *push_button, unsigned int message, unsigned int data_1, unsigned int data_2)
{
#ifdef GAME_DLL
    return Call_Function<WindowMsgHandledType, GameWindow *, unsigned int, unsigned int, unsigned int>(
        PICK_ADDRESS(0x005ABB40, 0x008F44B0), push_button, message, data_1, data_2);
#else
    return MSG_IGNORED;
#endif
}

WindowMsgHandledType Gadget_Push_Button_System(
    GameWindow *push_button, unsigned int message, unsigned int data_1, unsigned int data_2)
{
#ifdef GAME_DLL
    return Call_Function<WindowMsgHandledType, GameWindow *, unsigned int, unsigned int, unsigned int>(
        PICK_ADDRESS(0x005AC1B0, 0x008F4D18), push_button, message, data_1, data_2);
#else
    return MSG_IGNORED;
#endif
}

void *Gadget_Button_Get_Data(GameWindow *push_button)
{
#ifdef GAME_DLL
    return Call_Function<void *, GameWindow *>(PICK_ADDRESS(0x005AC4D0, 0x008F5209), push_button);
#else
    return nullptr;
#endif
}

void Gadget_Button_Enable_Check_Like(GameWindow *push_button, bool is_enabled, bool is_checked)
{
#ifdef GAME_DLL
    Call_Function<void, GameWindow *, bool, bool>(PICK_ADDRESS(0x005AC300, 0x008F4F1C), push_button, is_enabled, is_checked);
#endif
}

void Gadget_Button_Set_Data(GameWindow *push_button, void *data)
{
#ifdef GAME_DLL
    Call_Function<void, GameWindow *, void *>(PICK_ADDRESS(0x005AC4A0, 0x008F51CB), push_button, data);
#endif
}

void Gadget_Button_Set_Alt_Sound(GameWindow *push_button, Utf8String alt_sound)
{
#ifdef GAME_DLL
    Call_Function<void, GameWindow *, Utf8String>(PICK_ADDRESS(0x005AC4F0, 0x008F5236), push_button, alt_sound);
#endif
}

void Gadget_Button_Set_Border(GameWindow *push_button, int color, bool draw_border)
{
#ifdef GAME_DLL
    Call_Function<void, GameWindow *, int, bool>(PICK_ADDRESS(0x005AC3F0, 0x008F50AC), push_button, color, draw_border);
#endif
}

void Gadget_Button_Draw_Overlay_Image(GameWindow *push_button, const Image *image)
{
#ifdef GAME_DLL
    Call_Function<void, GameWindow *, const Image *>(PICK_ADDRESS(0x005AC470, 0x008F518D), push_button, image);
#endif
}

void Gadget_Button_Draw_Inverse_Clock(GameWindow *push_button, int percent, int color)
{
#ifdef GAME_DLL
    Call_Function<void, GameWindow *, int, int>(PICK_ADDRESS(0x005AC430, 0x008F5140), push_button, percent, color);
#endif
}

void Gadget_Check_Like_Button_Set_Visual_Check(GameWindow *push_button, bool is_checked)
{
#ifdef GAME_DLL
    Call_Function<void, GameWindow *, bool>(PICK_ADDRESS(0x005AC2B0, 0x008F4E41), push_button, is_checked);
#endif
}
