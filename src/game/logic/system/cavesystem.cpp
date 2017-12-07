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

CaveSystem::CaveSystem() : m_firstCave(0), m_lastCave(0), m_unk14(0) {};

void CaveSystem::Reset()
{
#ifndef THYME_STANDALONE
    Call_Method<void, CaveSystem>(0x004D56D0, this);
#else
    // TODO 
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