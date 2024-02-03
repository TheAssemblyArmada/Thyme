/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief GUI Command Translator
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "guicommandtranslator.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

GameMessageDisposition GUICommandTranslator::Translate_Game_Message(const GameMessage *msg)
{
#ifdef GAME_DLL
    return Call_Method<GameMessageDisposition, GUICommandTranslator, const GameMessage *>(
        PICK_ADDRESS(0x00616370, 0x00A0EC0D), this, msg);
#else
    return KEEP_MESSAGE;
#endif
}
