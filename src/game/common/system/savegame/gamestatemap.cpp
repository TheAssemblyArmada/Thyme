/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Game State Map
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "gamestatemap.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

#ifndef GAME_DLL
GameStateMap *g_theGameStateMap;
#endif

GameStateMap::~GameStateMap()
{
    Clear_Scratch_Pad_Maps();
}

void GameStateMap::Xfer_Snapshot(Xfer *xfer)
{
#ifdef GAME_DLL
    Call_Method<void, SnapShot, Xfer *>(PICK_ADDRESS(0x00495310, 0x00A01D92), this, xfer);
#endif
}

void GameStateMap::Clear_Scratch_Pad_Maps()
{
#ifdef GAME_DLL
    Call_Method<void, GameStateMap>(PICK_ADDRESS(0x00495BB0, 0x00A028D9), this);
#endif
}
