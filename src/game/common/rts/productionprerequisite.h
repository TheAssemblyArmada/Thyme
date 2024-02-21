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
#pragma once

#include "always.h"
#include "asciistring.h"
#include "science.h"
#include <vector>

class ThingTemplate;
class Player;

class ProductionPrerequisite
{
    struct PrereqUnitRec
    {
        ThingTemplate *unit;
        int flags;
        Utf8String name;
    };

public:
    ProductionPrerequisite() { Init(); }
    ~ProductionPrerequisite() {}

    void Init()
    {
        m_prereqUnit.clear();
        m_prereqScience.clear();
    }

    void Add_Science_Prereq(ScienceType science) { m_prereqScience.push_back(science); }
    void Add_Unit_Prereq(Utf8String name, bool or_with_previous);
    void Resolve_Names();
    int Get_All_Possible_Build_Facility_Templates(ThingTemplate **tmpls, int max_tmpls) const;
    int Calc_Num_Prereq_Units_Owned(const Player *player, int *counts) const;
    bool Is_Satisifed(const Player *player) const;
    Utf16String Get_Requires_List(const Player *player) const;
    ThingTemplate *Get_Existing_Build_Facility_Template(const Player *player) const;

    // TODO member functions.
private:
    std::vector<PrereqUnitRec> m_prereqUnit;
    std::vector<ScienceType> m_prereqScience;
};
