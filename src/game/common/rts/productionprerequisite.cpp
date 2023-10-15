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
#include "gametext.h"
#include "player.h"
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
    for (PrereqUnitRec &prereqUnit : m_prereqUnit) {
        if (prereqUnit.name.Is_Not_Empty()) {
            prereqUnit.unit = g_theThingFactory->Find_Template(prereqUnit.name, true);
            captainslog_dbgassert(prereqUnit.unit != nullptr, "could not find prereq %s", prereqUnit.name.Str());
        }

        prereqUnit.name.Clear();
    }
}

int ProductionPrerequisite::Get_All_Possible_Build_Facility_Templates(ThingTemplate **tmpls, int max_tmpls) const
{
    int i = 0;
    for (const PrereqUnitRec &prereq_unit : m_prereqUnit) {
        if (i > 0 && (prereq_unit.flags & 1)) {
            break;
        }
        if (i >= max_tmpls) {
            break;
        }
        tmpls[i++] = prereq_unit.unit;
    }
    return i;
}

int ProductionPrerequisite::Calc_Num_Prereq_Units_Owned(const Player *player, int *counts) const
{
    size_t size = m_prereqUnit.size();

    if (size > 32) {
        size = 32;
    }

    ThingTemplate *things[32];

    for (size_t i = 0; i < size; i++) {
        things[i] = m_prereqUnit[i].unit;
    }

    player->Count_Objects_By_Thing_Template(size, things, false, counts, true);
    return size;
}

bool ProductionPrerequisite::Is_Satisifed(const Player *player) const
{
    if (player == nullptr) {
        return false;
    }

    for (size_t i = 0; i < m_prereqScience.size(); i++) {
        if (!player->Has_Science(m_prereqScience[i])) {
            return false;
        }
    }

    int counts[32];
    int num = Calc_Num_Prereq_Units_Owned(player, counts);

    for (int i = 1; i < num; i++) {
        if ((m_prereqUnit[i].flags & 1) != 0) {
            counts[i] += counts[i - 1];
            counts[i - 1] = -1;
        }
    }

    for (int i = 0; i < num; i++) {
        if (counts[i] != -1 && counts[i] == 0) {
            return false;
        }
    }

    return true;
}

Utf16String ProductionPrerequisite::Get_Requires_List(const Player *player) const
{
    if (player != nullptr) {
        Utf16String str(Utf16String::s_emptyString);
        int counts[32];
        int num = Calc_Num_Prereq_Units_Owned(player, counts);
        bool required[32];

        for (int i = 0; i < 32; i++) {
            required[i] = false;
        }

        for (int i = 1; i < num; i++) {
            if ((m_prereqUnit[i].flags & 1) != 0) {
                required[i] = true;
                counts[i] += counts[i - 1];
                counts[i - 1] = -1;
            }
        }

        Utf16String str2;
        bool newline = true;

        for (int i = 0; i < num; i++) {
            if (counts[i] == 0) {
                if (required[i]) {
                    str2 = m_prereqUnit[i - 1].unit->Get_Display_Name();
                    str2.Concat(U_CHAR(' '));
                    str2.Concat(g_theGameText->Fetch("CONTROLBAR:OrRequirement"));
                    str2.Concat(U_CHAR(' '));
                    str.Concat(str2);
                }

                str2 = m_prereqUnit[i].unit->Get_Display_Name();

                if (newline) {
                    newline = false;
                } else {
                    str2.Concat('\n');
                }

                str.Concat(str2);
            }
        }

        bool has_sciences = true;

        for (size_t i = 0; i < m_prereqScience.size(); i++) {
            if (!player->Has_Science(m_prereqScience[i])) {
                has_sciences = false;
            }
        }

        if (!has_sciences) {
            if (newline) {
                newline = false;
            } else {
                str2.Concat(U_CHAR('\n'));
            }

            str.Concat(g_theGameText->Fetch("CONTROLBAR:GeneralsPromotion"));
        }

        return str;
    } else {
        return Utf16String::s_emptyString;
    }
}

ThingTemplate *ProductionPrerequisite::Get_Existing_Build_Facility_Template(const Player *player) const
{
    captainslog_dbgassert(player != nullptr, "player may not be null");

    if (m_prereqUnit.size() != 0) {
        int counts[32];
        int num = Calc_Num_Prereq_Units_Owned(player, counts);

        for (int i = 0; i < num && (i <= 0 || (m_prereqUnit[i].flags & 1) != 0); i++) {
            if (counts[i] != 0) {
                return m_prereqUnit[i].unit;
            }
        }
    }

    return nullptr;
}
