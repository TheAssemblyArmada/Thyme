/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Skirmish Battle Honors
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "always.h"
#include "skirmishbattlehonors.h"

SkirmishBattleHonors::SkirmishBattleHonors()
{
    Load("SkirmishStats.ini");
}

SkirmishBattleHonors::~SkirmishBattleHonors() {}

int SkirmishBattleHonors::Get_Endurance_Medal(Utf8String name, int level) const
{
    Utf8String str;
    str.Format("%s_%d", name.Str(), level);
    return Get_Int(str, 0);
}
