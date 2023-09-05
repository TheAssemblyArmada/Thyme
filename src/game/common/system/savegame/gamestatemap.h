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
#pragma once
#include "always.h"
#include "snapshot.h"
#include "subsysteminterface.h"

class GameStateMap : public SubsystemInterface, public SnapShot
{
public:
    GameStateMap() {}
    ~GameStateMap() override;
    void Init() override {}
    void Reset() override {}
    void Update() override {}

    void CRC_Snapshot(Xfer *xfer) override {}
    void Xfer_Snapshot(Xfer *xfer) override;
    void Load_Post_Process() override {}

    void Clear_Scratch_Pad_Maps();
};

#ifdef GAME_DLL
extern GameStateMap *&g_theGameStateMap;
#else
extern GameStateMap *g_theGameStateMap;
#endif
