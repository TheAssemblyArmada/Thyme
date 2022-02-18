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
#include "missionstats.h"
#include "xfer.h"

MissionStats::MissionStats()
{
    Init();
}

void MissionStats::Init()
{
    for (int i = 0; i < MAX_PLAYER_COUNT; ++i) {
        m_unitsKilled[i] = 0;
        m_buildingsKilled[i] = 0;
    }

    m_unitsLost = 0;
    m_buildingsLost = 0;
}

void MissionStats::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t version = 1;
    xfer->xferVersion(&version, 1);

    xfer->xferUser(&m_unitsKilled, sizeof(m_unitsKilled));
    xfer->xferInt(&m_unitsLost);
    xfer->xferUser(&m_buildingsKilled, sizeof(m_buildingsKilled));
    xfer->xferInt(&m_buildingsLost);
}
