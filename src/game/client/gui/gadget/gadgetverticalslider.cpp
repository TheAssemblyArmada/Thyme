/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Vertical Slider
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "gadgetslider.h"

WindowMsgHandledType Gadget_Vertical_Slider_Input(
    GameWindow *slider, unsigned int message, unsigned int data_1, unsigned int data_2)
{
#ifdef GAME_DLL
    return Call_Function<WindowMsgHandledType, GameWindow *, unsigned int, unsigned int, unsigned int>(
        PICK_ADDRESS(0x006D6F10, 0x00A5AE40), slider, message, data_1, data_2);
#else
    return MSG_IGNORED;
#endif
}

WindowMsgHandledType Gadget_Vertical_Slider_System(
    GameWindow *slider, unsigned int message, unsigned int data_1, unsigned int data_2)
{
#ifdef GAME_DLL
    return Call_Function<WindowMsgHandledType, GameWindow *, unsigned int, unsigned int, unsigned int>(
        PICK_ADDRESS(0x006D7370, 0x00A5B339), slider, message, data_1, data_2);
#else
    return MSG_IGNORED;
#endif
}
