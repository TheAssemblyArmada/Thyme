/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Special Power
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "specialpower.h"
#include "player.h"

#ifndef GAME_DLL
SpecialPowerStore *g_theSpecialPowerStore;
#endif

SpecialPowerStore::~SpecialPowerStore()
{
    for (unsigned int i = 0; i < m_specialPowerTemplates.size(); i++) {
        m_specialPowerTemplates[i]->Delete_Instance();
    }

    m_specialPowerTemplates.clear();
    m_nextSpecialPowerID = 0;
}

void SpecialPowerStore::Reset()
{
    for (auto it = m_specialPowerTemplates.begin(); it != m_specialPowerTemplates.end();) {
        if ((*it)->Delete_Overrides() == nullptr) {
            // #BUGFIX invalidated iterator dereference.
            it = m_specialPowerTemplates.erase(it);
        } else {
            ++it;
        }
    }
}

bool SpecialPowerStore::Can_Use_Special_Power(Object *obj, const SpecialPowerTemplate *special_power_template)
{
    if (obj == nullptr || special_power_template == nullptr) {
        return false;
    }

    if (obj->Get_Special_Power_Module(special_power_template) == nullptr) {
        return false;
    }

    ScienceType science = special_power_template->Get_Required_Science();

    if (science != SCIENCE_INVALID) {
        if (!obj->Get_Controlling_Player()->Has_Science(science)) {
            return false;
        }
    }

    return true;
}

const SpecialPowerTemplate *SpecialPowerTemplate::Get_FO() const
{
    return static_cast<const SpecialPowerTemplate *>(Friend_Get_Final_Override());
}
