/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Tracks prerequisites needed for a thing to be built.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "productionprerequisite.h"
#include "thingfactory.h"

void ProductionPrerequisite::Add_Unit_Prereq(Utf8String name, bool or_with_previous)
{
    PrereqUnitRec rec;
    rec.name = name;
    rec.flags = or_with_previous != 0;
    rec.unit = nullptr;
    m_prereqUnit.push_back(rec);
}

void ProductionPrerequisite::Resolve_Names()
{
    for (unsigned int i = 0; i < m_prereqUnit.size(); i++) {
        if (m_prereqUnit[i].name.Is_Not_Empty()) {
            m_prereqUnit[i].unit = g_theThingFactory->Find_Template(m_prereqUnit[i].name, true);
            captainslog_dbgassert(m_prereqUnit[i].unit != nullptr, "could not find prereq %s", m_prereqUnit[i].name.Str());
        }

        m_prereqUnit[i].name.Clear();
    }
}

int ProductionPrerequisite::Get_All_Possible_Build_Facility_Templates(ThingTemplate **tmpls, int max_tmpls)
{
    int count = 0;

    for (unsigned int i = 0; i < m_prereqUnit.size() && (i <= 0 || (m_prereqUnit[i].flags & 1) != 0) && count < max_tmpls;
         i++) {
        tmpls[count++] = m_prereqUnit[i].unit;
    }

    return count;
}
