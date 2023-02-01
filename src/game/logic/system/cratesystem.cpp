/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Crate System
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "cratesystem.h"

#ifndef GAME_DLL
CrateSystem *g_theCrateSystem;
#endif

CrateSystem::CrateSystem()
{
    m_crateTemplateVector.clear();
}

CrateSystem::~CrateSystem()
{
    for (int i = 0; i < m_crateTemplateVector.size(); i++) {
        CrateTemplate *tmplate = m_crateTemplateVector[i];

        if (tmplate != nullptr) {
            tmplate->Delete_Instance();
        }
    }

    m_crateTemplateVector.clear();
}

void CrateSystem::Init()
{
    Reset();
}

void CrateSystem::Reset()
{
#ifdef GAME_DLL
    Call_Method<void, CrateSystem>(PICK_ADDRESS(0x0049B380, 0x007489C0), this);
#endif
}
