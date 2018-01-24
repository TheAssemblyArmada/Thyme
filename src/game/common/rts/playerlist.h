/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Contains a list of players in the game.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#ifndef PLAYERLIST_H
#define PLAYERLIST_H

#include "player.h"
#include "snapshot.h"
#include "subsysteminterface.h"

class PlayerList : public SubsystemInterface, public SnapShot
{
public:
    PlayerList();
    virtual ~PlayerList();

    // SubsystemInterface interface
    virtual void Init();
    virtual void Reset();
    virtual void Update();

    virtual void New_Game();
    virtual void New_Map();

    // SnapShot interface
    virtual void CRC_Snapshot(Xfer *xfer);
    virtual void Xfer_Snapshot(Xfer *xfer);
    virtual void Load_Post_Process();

    // TODO implementations and none virtual functions.
    Player *Get_Local_Player() { return m_local; }

private:
    Player *m_local;
    int m_playerCount;
    Player *m_players;
};

#ifndef THYME_STANDALONE
#include "hooker.h"

extern PlayerList *&g_thePlayerList;
#else
extern PlayerList *g_thePlayerList;
#endif

#endif
