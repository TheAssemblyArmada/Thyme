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

#include "gametype.h"
#include "mempoolobj.h"
#include "snapshot.h"
#include "subsysteminterface.h"
#include <list>

class Object;

class TunnelTracker : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_POOL(TunnelTracker);

protected:
    ~TunnelTracker() override{};

public:
    TunnelTracker() : m_containListSize(0), m_tunnelCount(0), m_nemesisID(0), m_lastNemesisFrame(0) {}

    // Snapshot interface methods
    void CRC_Snapshot(Xfer *xfer) override {}
    void Xfer_Snapshot(Xfer *xfer) override;
    void Load_Post_Process() override;

    std::list<ObjectID> m_tunnelIDs;
    std::list<Object *> m_containList;
    std::list<ObjectID> m_IDs;
    int32_t m_containListSize;
    uint32_t m_tunnelCount;
    int m_nemesisID;
    int m_lastNemesisFrame;
};

class CaveSystem : public SubsystemInterface, public SnapShot
{
public:
    CaveSystem() {}
    ~CaveSystem() override { Reset(); };

    // Subsystem interface methods.
    void Init() override {}
    void Reset() override;
    void Update() override {}

    // Snapshot interface methods
    void CRC_Snapshot(Xfer *xfer) override {}
    void Xfer_Snapshot(Xfer *xfer) override;
    void Load_Post_Process() override {}

    bool Can_Switch_Index_to_Index(size_t unk1, size_t unk2);
    TunnelTracker *Register_New_Cave(int index);
    TunnelTracker *Get_Tunnel_Tracker_For_Cave_Index(size_t index);

    // zh: 0x004D55D0 wb: 0x0076E506
    CaveSystem *Hook_Ctor() { return new (this) CaveSystem; }

private:
    std::vector<TunnelTracker *> m_tunnelTrackerVector;
};

#ifdef GAME_DLL
extern CaveSystem *&g_theCaveSystem;
#else
extern CaveSystem *g_theCaveSystem;
#endif