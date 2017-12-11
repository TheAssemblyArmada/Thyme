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

#include "cavesystem.h"

void TunnelTracker::Xfer_Snapshot(Xfer *xfer)
{
#ifndef THYME_STANDALONE
    Call_Method<void, TunnelTracker>(0x00587CC3, this);
#else
    // TODO 
#endif
}

void TunnelTracker::Load_Post_Process()
{
#ifndef THYME_STANDALONE
    Call_Method<void, TunnelTracker>(0x00587DA0, this);
#else
    // TODO 
#endif
}

void CaveSystem::Reset()
{

    
#ifndef THYME_STANDALONE
    Call_Method<void, CaveSystem>(0x004D56D0, this);
#else
    // Todo verify this
    for (auto it = caves.begin(); it != caves.end(); ++it) {
        if (*it != nullptr) {
            caves.erase(it);
        }
    }
#endif
}

void CaveSystem::Xfer_Snapshot(Xfer *xfer)
{
#ifndef THYME_STANDALONE
    Call_Method<void, CaveSystem>(0x004D58A2, this);
#else
    // TODO 
#endif
}