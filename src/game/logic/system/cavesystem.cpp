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

#ifndef GAME_DLL
CaveSystem *g_theCaveSystem = nullptr;
#else
#include "hooker.h"
#endif

void TunnelTracker::Xfer_Snapshot(Xfer *xfer)
{
#ifdef GAME_DLL
    Call_Method<void, TunnelTracker, Xfer *>(PICK_ADDRESS(0x00587CC3, 0x009118AB), this, xfer);
#else
    // TODO
#endif
}

void TunnelTracker::Load_Post_Process()
{
#ifdef GAME_DLL
    Call_Method<void, TunnelTracker>(0x00587DA0, this);
#else
    // TODO
#endif
}

// 0x004D56D0
void CaveSystem::Reset()
{
    for (auto it = m_caves.begin(); it != m_caves.end(); ++it) {
        Delete_Instance(*it);
    }
    m_caves.clear();
}

void CaveSystem::Xfer_Snapshot(Xfer *xfer)
{
#ifdef GAME_DLL
    Call_Method<void, CaveSystem, Xfer *>(PICK_ADDRESS(0x004D58A2, 0x0076E80B), this, xfer);
#else
    // TODO
#endif
}

// 0x004D5730
bool CaveSystem::Can_Switch_Index_to_Index(size_t unk1, size_t unk2)
{
    if (unk1 < m_caves.size()) {
        auto tunnel = m_caves[unk1];
        if (tunnel != nullptr) {
            if (tunnel->unk14 == 0)
                return false;
        }
    }

    if (unk2 < m_caves.size()) {
        auto tunnel = m_caves[unk2];
        if (tunnel != nullptr) {
            if (tunnel->unk14 == 0)
                return false;
        }
    }

    return true;
}

// 0x004D5790
TunnelTracker *CaveSystem::Register_New_Cave(int index)
{
#ifdef GAME_DLL
    return Call_Method<TunnelTracker *, CaveSystem, int>(PICK_ADDRESS(0x004D58A2, 0x0076E69D), this, index);
#else
    // TODO
    return nullptr;
#endif
}

// 0x004D5880
TunnelTracker *CaveSystem::Get_Tunnel_Tracker_For_Cave_Index(size_t index)
{
    if (index > m_caves.size())
        return nullptr;

    return m_caves[index];
}
