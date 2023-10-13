/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Command Translate
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "commandxlat.h"

void Pick_And_Play_Unit_Voice_Response(
    const std::list<Drawable *> *list, GameMessage::MessageType type, PickAndPlayInfo *info)
{
#ifdef GAME_DLL
    Call_Function<void, const std::list<Drawable *> *, GameMessage::MessageType, PickAndPlayInfo *>(
        PICK_ADDRESS(0x005E92B0, 0x007E8AE3), list, type, info);
#endif
}
