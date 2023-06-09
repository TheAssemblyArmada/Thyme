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

#include "cavesystem.h"
#include "object.h"
#include "xfer.h"

#ifndef GAME_DLL
CaveSystem *g_theCaveSystem = nullptr;
#else
#include "hooker.h"
#endif

// zh: 0x00587CC3 wb: 0x009118AB
void TunnelTracker::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t version = 1;
    xfer->xferVersion(&version, 1);

    xfer->xferSTLObjectIDList(&m_tunnelIDs);

    xfer->xferInt(&m_containListSize);

    switch (xfer->Get_Mode()) {
        case XferType::XFER_SAVE:
            for (auto *obj : m_containList) {
                auto obj_ID = obj->Get_ID();
                xfer->xferObjectID(&obj_ID);
            }
            break;
        default:
            for (int32_t i = 0; i < m_containListSize; ++i) {
                auto obj_ID = ObjectID::INVALID_OBJECT_ID;
                xfer->xferObjectID(&obj_ID);
                m_IDs.push_back(obj_ID);
            }
            break;
    }

    xfer->xferUnsignedInt(&m_tunnelCount);
}

void TunnelTracker::Load_Post_Process()
{
    // Requires AI, PartitionManager classes.
#ifdef GAME_DLL
    Call_Method<void, SnapShot>(0x00587DA0, this);
#else
    // TODO
#endif
}

// 0x004D56D0
void CaveSystem::Reset()
{
    for (auto it = m_tunnelTrackerVector.begin(); it != m_tunnelTrackerVector.end(); ++it) {
        (*it)->Delete_Instance();
    }
    m_tunnelTrackerVector.clear();
}

// zh: 0x004D58A0 wb: 0x0076E80B
void CaveSystem::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t version = 1;
    xfer->xferVersion(&version, 1);

    uint16_t num_tunnels = static_cast<uint16_t>(m_tunnelTrackerVector.size());
    xfer->xferUnsignedShort(&num_tunnels);

    switch (xfer->Get_Mode()) {
        case XferType::XFER_SAVE:
            for (auto *tunnel : m_tunnelTrackerVector) {
                xfer->xferSnapshot(tunnel);
            }
            break;
        default:
            captainslog_relassert(m_tunnelTrackerVector.empty(),
                6,
                "CaveSystem::Xfer_Snapshot - m_tunnelTrackVector should be empty but is not!");

            for (uint16_t i = 0u; i < num_tunnels; ++i) {
                auto *new_tunnel = NEW_POOL_OBJ(TunnelTracker);
                xfer->xferSnapshot(new_tunnel);
                m_tunnelTrackerVector.push_back(new_tunnel);
            }
            break;
    }
}

// 0x004D5730
bool CaveSystem::Can_Switch_Index_to_Index(size_t unk1, size_t unk2)
{
    if (unk1 < m_tunnelTrackerVector.size()) {
        auto *tunnel = m_tunnelTrackerVector[unk1];
        if (tunnel != nullptr) {
            if (tunnel->m_containListSize == 0)
                return false;
        }
    }

    if (unk2 < m_tunnelTrackerVector.size()) {
        auto *tunnel = m_tunnelTrackerVector[unk2];
        if (tunnel != nullptr) {
            if (tunnel->m_containListSize == 0)
                return false;
        }
    }

    return true;
}

// zh: 0x004D5790 wb: 0x0076E766
TunnelTracker *CaveSystem::Register_New_Cave(int index)
{
    if (static_cast<uint32_t>(index) >= m_tunnelTrackerVector.size()) {
        m_tunnelTrackerVector.resize(index + 1, nullptr);
    }

    if (m_tunnelTrackerVector[index] != nullptr) {
        return m_tunnelTrackerVector[index];
    }

    auto *tunnel = NEW_POOL_OBJ(TunnelTracker);
    m_tunnelTrackerVector[index] = tunnel;

    return tunnel;
}

// 0x004D5880
TunnelTracker *CaveSystem::Get_Tunnel_Tracker_For_Cave_Index(size_t index)
{
    if (index > m_tunnelTrackerVector.size())
        return nullptr;

    return m_tunnelTrackerVector[index];
}
