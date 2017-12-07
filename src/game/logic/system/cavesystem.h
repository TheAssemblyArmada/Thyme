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


#include "subsysteminterface.h"
#include "snapshot.h"

class CaveSystem : public SubsystemInterface, public SnapShot
{
public:
    CaveSystem();
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
    int m_firstCave; // 0x0C TunnelTracker *
    int m_lastCave; // 0x10 TunnelTrack *
    int m_unk14;
};

#endif // CAVESYSTEM_H