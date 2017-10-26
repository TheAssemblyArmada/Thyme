/**
 * @file
 *
 * @Author OmniBlade
 *
 * @brief Game state tracker.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "asciistring.h"
#include "snapshot.h"
#include "subsysteminterface.h"
#include "xfer.h"

#ifndef THYME_STANDALONE
#include "hooker.h"
#endif

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

    AsciiString Get_Save_Dir();
    AsciiString Real_To_Portable_Map_Path(const AsciiString &path);
    AsciiString Portable_To_Real_Map_Path(const AsciiString &path);

#ifndef THYME_STANDALONE
    static void Hook_Me();
#endif
private:
};

AsciiString Get_Leaf_And_Dir_Name(const AsciiString &path);

#ifndef THYME_STANDALONE
extern GameState *&g_theGameState;

inline void GameState::Hook_Me()
{
    Hook_Method(0x004939A0, &Real_To_Portable_Map_Path);
    Hook_Method(0x00493C90, &Portable_To_Real_Map_Path);
}
#else
extern GameState *g_theGameState;
#endif

#endif
