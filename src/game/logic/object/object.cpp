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
#include "behaviormodule.h"
#include "drawable.h"
#include "experiencetracker.h"
#include "playerlist.h"

ObjectShroudStatus Object::Get_Shrouded_Status(int index) const
{
#ifdef GAME_DLL
    return Call_Method<ObjectShroudStatus, const Object, int>(PICK_ADDRESS(0x00547D60, 0x007D12FB), this, index);
#else
    return ObjectShroudStatus(0);
#endif
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

int Object::Get_Night_Indicator_Color() const
{
#ifdef GAME_DLL
    return Call_Method<int, const Object>(PICK_ADDRESS(0x00547AF0, 0x007D0FEC), this);
#else
    return 0;
#endif
}

int Object::Get_Indicator_Color() const
{
#ifdef GAME_DLL
    return Call_Method<int, const Object>(PICK_ADDRESS(0x00547AC0, 0x007D0F89), this);
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

bool Object::Get_Ammo_Pip_Showing_Info(int &clip_size, int &ammo_in_clip) const
{
#ifdef GAME_DLL
    return Call_Method<bool, const Object, int &, int &>(
        PICK_ADDRESS(0x00547760, 0x007D0A7F), this, clip_size, ammo_in_clip);
#else
    return false;
#endif
}

BehaviorModule *Object::Find_Module(NameKeyType type) const
{
    for (BehaviorModule **module = m_allModules; *module != nullptr; module++) {
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

void Object::Init_Object()
{
#ifdef GAME_DLL
    Call_Method<void, Object>(PICK_ADDRESS(0x00545D90, 0x007CF205), this);
#endif
}

float Object::Get_Carrier_Deck_Height() const
{
#ifdef GAME_DLL
    return Call_Method<float, const Object>(PICK_ADDRESS(0x0054F3D0, 0x007D96FE), this);
#else
    return 0.0f;
#endif
}

void Object::Set_Status(BitFlags<OBJECT_STATUS_COUNT> bits, bool set)
{
#ifdef GAME_DLL
    Call_Method<void, Object, BitFlags<OBJECT_STATUS_COUNT>, bool>(PICK_ADDRESS(0x00546E20, 0x007D011C), this, bits, set);
#endif
}

void Object::Set_Model_Condition_State(ModelConditionFlagType a)
{
    if (m_drawable != nullptr) {
        m_drawable->Set_Model_Condition_State(a);
    }
}

void Object::Clear_Model_Condition_State(ModelConditionFlagType a)
{
    if (m_drawable != nullptr) {
        m_drawable->Clear_Model_Condition_State(a);
    }
}

void Object::Attempt_Damage(DamageInfo *info)
{
#ifdef GAME_DLL
    Call_Method<void, Object, DamageInfo *>(PICK_ADDRESS(0x00547DA0, 0x007D134E), this, info);
#endif
}

void Object::Clear_Disabled(DisabledType type)
{
#ifdef GAME_DLL
    Call_Method<void, Object, DisabledType>(PICK_ADDRESS(0x005489F0, 0x007D1FE8), this, type);
#endif
}

bool Object::Can_Crush_Or_Squish(Object *obj)
{
#ifdef GAME_DLL
    return Call_Method<bool, Object, Object *>(PICK_ADDRESS(0x005471D0, 0x007D0486), this, obj);
#else
    return false;
#endif
}

void Object::Defect(Team *team, unsigned int i)
{
#ifdef GAME_DLL
    Call_Method<void, Object, Team *, unsigned int>(PICK_ADDRESS(0x0054EEC0, 0x007D924B), this, team, i);
#endif
}

unsigned char Object::Get_Crushable_Level() const
{
    return Get_Template()->Get_Crushable_Level();
}

unsigned char Object::Get_Crusher_Level() const
{
    return Get_Template()->Get_Crusher_Level();
}

void Object::Kill(DamageType damage, DeathType death)
{
#ifdef GAME_DLL
    Call_Method<void, Object, DamageType, DeathType>(PICK_ADDRESS(0x00548300, 0x007D1733), this, damage, death);
#endif
}

void Object::On_Collide(Object *other, const Coord3D *loc, const Coord3D *normal)
{
    for (BehaviorModule **module = m_allModules; *module != nullptr; module++) {
        CollideModuleInterface *collide = (*module)->Get_Collide();

        if (collide != nullptr) {
            if (Get_Status_Bits().Test(OBJECT_STATUS_NO_COLLISIONS)) {
                return;
            }

            collide->On_Collide(other, loc, normal);
        }
    }
}

void Object::Set_Disabled(DisabledType type)
{
    Set_Disabled_Until(type, 0x3FFFFFFF);
}

void Object::Set_Disabled_Until(DisabledType type, unsigned int i)
{
#ifdef GAME_DLL
    Call_Method<void, Object, DisabledType, unsigned int>(PICK_ADDRESS(0x005485C0, 0x007D1A3F), this, type, i);
#endif
}

void Object::Set_Captured(bool captured)
{
    if (captured) {
        m_privateStatus |= STATUS_CAPTURED;
    } else {
        captainslog_debug("Clearing Captured Status. This should never happen.");
        m_privateStatus &= ~STATUS_CAPTURED;
    }
}
