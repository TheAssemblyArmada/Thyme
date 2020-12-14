/**
 * @file
 *
 * @author Duncans_pumpkin
 *
 * @brief Cave System?
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */

#pragma once

#include "mempoolobj.h"
#include "snapshot.h"
#include "subsysteminterface.h"

class TunnelTracker : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_POOL(TunnelTracker);

protected:
    virtual ~TunnelTracker() override;

public:
    // Snapshot interface methods
    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    int unk08; // Ptr uses new
    int unk0C; // Ptr uses new
    int unk10; // Ptr uses new
    int unk14;
    int unk18;
    int unk1C;
    int unk20;
};

class CaveSystem : public SubsystemInterface, public SnapShot
{
public:
    virtual ~CaveSystem() { Reset(); };

    // Subsystem interface methods.
    virtual void Init() override {}
    virtual void Reset() override;
    virtual void Update() override {}

    // Snapshot interface methods
    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    bool Can_Switch_Index_to_Index(size_t unk1, size_t unk2);
    TunnelTracker *Register_New_Cave(int index);
    TunnelTracker *Get_Tunnel_Tracker_For_Cave_Index(size_t index);

private:
    std::vector<TunnelTracker *> m_caves;
};

#ifdef GAME_DLL
extern CaveSystem *&g_theCaveSystem;
#else
extern CaveSystem *g_theCaveSystem;
#endif