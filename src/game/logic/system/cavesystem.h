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

#ifndef CAVESYSTEM_H
#define CAVESYSTEM_H

#include "mempoolobj.h"
#include "subsysteminterface.h"
#include "snapshot.h"

class TunnelTracker : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_POOL(TunnelTracker);

    // Snapshot interface methods
    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    int unk08; //Ptr uses new
    int unk0C; //Ptr uses new
    int unk10; //Ptr uses new
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

#ifndef THYME_STANDALONE
    static void Hook_Me();
#endif
private:
    std::vector<TunnelTracker *> m_caves;
};


#ifndef THYME_STANDALONE
#include "hooker.h"
extern CaveSystem *&g_theCaveSystem;

inline void CaveSystem::Hook_Me()
{
    Hook_Method(0x004D5730, &Can_Switch_Index_to_Index);
    Hook_Method(0x004D5790, &Register_New_Cave);
    Hook_Method(0x004D5880, &Get_Tunnel_Tracker_For_Cave_Index);
}
#else
extern CaveSystem *g_theCaveSystem;
#endif

#endif // CAVESYSTEM_H
