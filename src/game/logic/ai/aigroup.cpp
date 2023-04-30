/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief AI Group
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "aigroup.h"

void AIGroup::Add(Object *obj)
{
#ifdef GAME_DLL
    Call_Method<void, AIGroup, Object *>(PICK_ADDRESS(0x0054FB60, 0x008D27B0), this, obj);
#endif
}

void AIGroup::Remove(Object *obj)
{
#ifdef GAME_DLL
    Call_Method<void, AIGroup, Object *>(PICK_ADDRESS(0x0054FBF0, 0x008D287E), this, obj);
#endif
}

const std::vector<ObjectID> &AIGroup::Get_All_IDs() const
{
#ifdef GAME_DLL
    return Call_Method<const std::vector<ObjectID> &, const AIGroup>(PICK_ADDRESS(0x0054FAC0, 0x008D26A1), this);
#else
    return std::vector<ObjectID>();
#endif
}
