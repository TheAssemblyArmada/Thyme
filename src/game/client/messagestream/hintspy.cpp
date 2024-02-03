/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Hint Spy
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "hintspy.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

GameMessageDisposition HintSpyTranslator::Translate_Game_Message(const GameMessage *msg)
{
#ifdef GAME_DLL
    return Call_Method<GameMessageDisposition, HintSpyTranslator, const GameMessage *>(
        PICK_ADDRESS(0x00613BC0, 0x00A25090), this, msg);
#else
    return KEEP_MESSAGE;
#endif
}
