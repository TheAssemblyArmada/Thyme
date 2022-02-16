/**
 * @file
 *
 * @author tomsons26
 *
 * @brief
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
#include "gametype.h"
#include "snapshot.h"

class Xfer;

class MissionStats : public SnapShot
{
public:
    MissionStats();

    // SnapShot interface
    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    void Init();

private:
    int32_t m_unitsKilled[MAX_PLAYER_COUNT];
    int32_t m_unitsLost;
    int32_t m_buildingsKilled[MAX_PLAYER_COUNT];
    int32_t m_buildingsLost;
};
