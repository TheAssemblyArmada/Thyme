/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Window Xlat
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "windowxlat.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

GameMessageDisposition WindowTranslator::Translate_Game_Message(const GameMessage *msg)
{
#ifdef GAME_DLL
    return Call_Method<GameMessageDisposition, WindowTranslator, const GameMessage *>(
        PICK_ADDRESS(0x006170F0, 0x00A24A6B), this, msg);
#else
    return KEEP_MESSAGE;
#endif
}
