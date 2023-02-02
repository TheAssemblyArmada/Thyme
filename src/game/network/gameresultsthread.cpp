/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Game Results Thread
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "gameresultsthread.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

#ifndef GAME_DLL
GameResultsInterface *g_theGameResultsQueue;
#endif

GameResultsInterface *GameResultsInterface::Create_New_Game_Results_Interface()
{
#ifdef GAME_DLL
    return Call_Function<GameResultsInterface *>(PICK_ADDRESS(0x0047C8C0, 0x00A2E270));
#else
    return nullptr;
#endif
}
