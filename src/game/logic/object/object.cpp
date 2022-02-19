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
#include "experiencetracker.h"
#include "module.h"
#include "playerlist.h"

ObjectShroudStatus Object::Get_Shrouded_Status(int32_t index) const
{
#ifdef GAME_DLL
    return Call_Method<ObjectShroudStatus, const Object, int32_t>(PICK_ADDRESS(0x00547D60, 0x007D12FB), this, index);
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

bool Object::Is_In_List(Object **list) const
{
    return m_prev != nullptr || m_next != nullptr || *list == this;
}

// zh: 0x00549BD0 wb: 0x007D370C
void Object::On_Veterancy_Level_Changed(VeterancyLevel v1, VeterancyLevel v2, bool b)
{
#ifdef GAME_DLL
    Call_Method<void, Object, VeterancyLevel, VeterancyLevel, bool>(PICK_ADDRESS(0x00549BD0, 0x007D370C), this, v1, v2, b);
#endif
}

int32_t Object::Get_Night_Indicator_Color() const
{
#ifdef GAME_DLL
    return Call_Method<int32_t, const Object>(PICK_ADDRESS(0x00547AF0, 0x007D0FEC), this);
#else
    return 0;
#endif
}

int32_t Object::Get_Indicator_Color() const
{
#ifdef GAME_DLL
    return Call_Method<int32_t, const Object>(PICK_ADDRESS(0x00547AC0, 0x007D0F89), this);
#else
    return 0;
#endif
}

bool Object::Is_Locally_Controlled() const
{
    return g_thePlayerList->Get_Local_Player() == Get_Controlling_Player();
}

void Object::Get_Health_Box_Position(Coord3D &pos) const
{
    pos = *Get_Position();
    pos.z += Get_Geometry_Info().Get_Max_Height_Above_Position() + 10.0f;
    pos.Add(&m_healthBoxOffset);

    if (Is_KindOf(KINDOF_MOB_NEXUS)) {
        pos.z += 20.0f;
    }
}

bool Object::Get_Health_Box_Dimensions(float &width, float &height) const
{
    if (Is_KindOf(KINDOF_IGNORED_IN_GUI)) {
        width = 0.0f;
        height = 0.0f;
        return false;
    }

    float f1;

    if (Get_Geometry_Info().Get_Minor_Radius() + Get_Geometry_Info().Get_Major_Radius() > 150.0f) {
        f1 = 150.0f;
    } else {
        f1 = Get_Geometry_Info().Get_Minor_Radius() + Get_Geometry_Info().Get_Major_Radius();
    }

    float f2 = std::max(f1, 20.0f);

    width = 3.0f;

    if (f2 + f2 < 20.0f) {
        height = 20.0f;
    } else {
        height = f2 + f2;
    }

    return true;
}

bool Object::Get_Ammo_Pip_Showing_Info(int32_t &clip_size, int32_t &ammo_in_clip) const
{
#ifdef GAME_DLL
    return Call_Method<bool, const Object, int32_t &, int32_t &>(
        PICK_ADDRESS(0x00547760, 0x007D0A7F), this, clip_size, ammo_in_clip);
#else
    return false;
#endif
}

Module *Object::Find_Module(NameKeyType type) const
{
    for (Module **module = m_allModules; *module != nullptr; module++) {
        if ((*module)->Get_Module_Name_Key() == type) {
            return *module;
        }
    }

    return nullptr;
}

VeterancyLevel Object::Get_Veterancy_Level() const
{
    if (m_experienceTracker) {
        return m_experienceTracker->Get_Current_Level();
    } else {
        return VETERANCY_REGULAR;
    }
}

bool Object::Is_Selectable() const
{
    if (Get_Template()->Is_KindOf(KINDOF_ALWAYS_SELECTABLE)) {
        return true;
    }

    return m_isSelectable && !Get_Status(OBJECT_STATUS_UNSELECTABLE) && !Is_Effectively_Dead();
}

bool Object::Is_Mass_Selectable() const
{
    return Is_Selectable() && !Is_KindOf(KINDOF_STRUCTURE);
}
