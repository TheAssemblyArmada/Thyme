/**
 * @file
 *
 * @author Jonathan Wilson
 * @author tomsons26
 *
 * @brief
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "object.h"

ObjectShroudStatus Object::Get_Shrouded_Status(int index) const
{
#ifdef GAME_DLL
    return Call_Method<ObjectShroudStatus, const Object, int>(PICK_ADDRESS(0x00547D60, 0x007D12FB), this, index);
#endif
    return ObjectShroudStatus(0);
}

// zh: 0x005479B0 wb: 0x007D0E3D
Relationship Object::Get_Relationship(const Object *that) const
{
#ifdef GAME_DLL
    return Call_Method<Relationship, const Object, const Object *>(PICK_ADDRESS(0x005479B0, 0x007D0E3D), this, that);
#else
    return Relationship();
#endif
}

Player *Object::Get_Controlling_Player() const
{
#ifdef GAME_DLL
    return Call_Method<Player *, const Object>(PICK_ADDRESS(0x00547A00, 0x007D0EAB), this);
#endif
    return nullptr;
}

// zh: 0x00549BD0 wb: 0x007D370C
void Object::On_Veterancy_Level_Changed(VeterancyLevel v1, VeterancyLevel v2, bool b)
{
#ifdef GAME_DLL
    Call_Method<void, Object, VeterancyLevel, VeterancyLevel, bool>(PICK_ADDRESS(0x00549BD0, 0x007D370C), this, v1, v2, b);
#endif
}
