/**
 * @file
 *
 * @Author Duncans_pumpkin
 *
 * @brief Cave System?
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
    virtual ~CaveSystem() {};

    // Subsystem interface methods.
    virtual void Init() override {}
    virtual void Reset() override;
    virtual void Update() override {}

    // Snapshot interface methods
    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

private:
    std::vector<TunnelTracker *> caves;
};

#endif // CAVESYSTEM_H