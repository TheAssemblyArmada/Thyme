/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Game state tracker.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "asciistring.h"
#include "snapshot.h"
#include "subsysteminterface.h"
#include "xfer.h"

class GameState : public SubsystemInterface, public SnapShot
{
public:
    GameState();
    virtual ~GameState();

    // SubsystemInterface implementations.
    virtual void Init();
    virtual void Reset();
    virtual void Update() {}

    // SnapShot implementations.
    virtual void CRC_Snapshot(Xfer *xfer) {}
    virtual void Xfer_Snapshot(Xfer *xfer);
    virtual void Load_Post_Process() {}

    Utf8String Get_Save_Dir();
    Utf8String Real_To_Portable_Map_Path(const Utf8String &path);
    Utf8String Portable_To_Real_Map_Path(const Utf8String &path);

private:
};

Utf8String Get_Leaf_And_Dir_Name(const Utf8String &path);

#ifdef GAME_DLL
extern GameState *&g_theGameState;
#else
extern GameState *g_theGameState;
#endif