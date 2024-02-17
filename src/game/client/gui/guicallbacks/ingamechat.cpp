/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief In Game Chat
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "always.h"
#include "ingamechat.h"

#ifdef GAME_DLL
#include "hooker.h"
#endif

bool Is_In_Game_Chat_Active()
{
#ifdef GAME_DLL
    return Call_Function<bool>(PICK_ADDRESS(0x005AE1A0, 0x008F7E4F));
#else
    return false;
#endif
}

void Reset_In_Game_Chat()
{
#ifdef GAME_DLL
    Call_Function<void>(PICK_ADDRESS(0x005ADFC0, 0x008F7C03));
#endif
}
