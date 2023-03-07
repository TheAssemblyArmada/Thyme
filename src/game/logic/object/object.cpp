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
#include "bodymodule.h"
#include "drawable.h"
#include "experiencetracker.h"
#include "gamelogic.h"
#include "globaldata.h"
#include "opencontain.h"
#include "playerlist.h"
#include "specialpower.h"
#include "team.h"
#include "updatemodule.h"
#include "w3ddebugicons.h"
#include "weaponset.h"

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

// #TODO This will cause compile error as soon as DamageModule is properly implemented elsewhere. Delete this then.
class DamageModule : public BehaviorModule
{
};

DamageModule *Object::Find_Damage_Module(NameKeyType type) const
{
    return static_cast<DamageModule *>(Find_Module(type));
}

UpdateModule *Object::Find_Update_Module(NameKeyType type) const
{
    return static_cast<UpdateModule *>(Find_Module(type));
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

bool Object::Can_Crush_Or_Squish(Object *obj, CrushSquishTestType type)
{
#ifdef GAME_DLL
    return Call_Method<bool, Object, Object *, CrushSquishTestType>(PICK_ADDRESS(0x005471D0, 0x007D0486), this, obj, type);
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

ExitInterface *Object::Get_Object_Exit_Interface() const
{
    ExitInterface *exit = nullptr;

    for (BehaviorModule **module = m_allModules; *module != nullptr; module++) {
        exit = (*module)->Get_Update_Exit_Interface();

        if (exit != nullptr) {
            break;
        }
    }

    if (exit == nullptr) {
        ContainModuleInterface *contain = Get_Contain();

        if (contain != nullptr) {
            return contain->Get_Contain_Exit_Interface();
        }
    }

    return exit;
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

const Weapon *Object::Get_Current_Weapon(WeaponSlotType *wslot) const
{
    if (!m_weaponSet.Has_Any_Weapon()) {
        return nullptr;
    }

    if (wslot) {
        *wslot = m_weaponSet.Get_Cur_Weapon_Slot();
    }

    return m_weaponSet.Get_Cur_Weapon();
}

Weapon *Object::Get_Current_Weapon(WeaponSlotType *wslot)
{
    if (!m_weaponSet.Has_Any_Weapon()) {
        return nullptr;
    }

    if (wslot) {
        *wslot = m_weaponSet.Get_Cur_Weapon_Slot();
    }

    return m_weaponSet.Get_Cur_Weapon();
}

bool Object::Is_Able_To_Attack() const
{
#ifdef GAME_DLL
    return Call_Method<bool, const Object>(PICK_ADDRESS(0x0054A0C0, 0x007D3B0D), this);
#else
    return false;
#endif
}

bool Object::Choose_Best_Weapon_For_Target(const Object *target, WeaponChoiceCriteria criteria, CommandSourceType source)
{
#ifdef GAME_DLL
    return Call_Method<bool, Object, const Object *, WeaponChoiceCriteria, CommandSourceType>(
        PICK_ADDRESS(0x00547810, 0x007D0B27), this, target, criteria, source);
#else
    return false;
#endif
}

CanAttackResult Object::Get_Able_To_Attack_Specific_Object(
    AbleToAttackType type, const Object *obj, CommandSourceType source, WeaponSlotType slot) const
{
#ifdef GAME_DLL
    return Call_Method<CanAttackResult, const Object, AbleToAttackType, const Object *, CommandSourceType, WeaponSlotType>(
        PICK_ADDRESS(0x005477B0, 0x007D0AC5), this, type, obj, source, slot);
#else
    return ATTACK_RESULT_0;
#endif
}

void Object::Set_Team(Team *team)
{
#ifdef GAME_DLL
    Call_Method<void, Object, Team *>(PICK_ADDRESS(0x00546800, 0x007CFB7A), this, team);
#endif
}

void Object::Heal_Completely()
{
#ifdef GAME_DLL
    Call_Method<void, Object>(PICK_ADDRESS(0x00548430, 0x007D1800), this);
#endif
}

float Object::Get_Vision_Range() const
{
#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData->m_debugVisibility) {
        Vector3 v(m_visionRange, 0.0f, 0.0f);

        for (int i = 0; i < g_theWriteableGlobalData->m_debugVisibilityTileCount; i++) {
            float f1 = i * 1.0f / g_theWriteableGlobalData->m_debugVisibilityTileCount;
            float angle = (f1 + f1) * GAMEMATH_PI;
            v.Rotate_Z(angle);
            Coord3D pos;
            pos.x = v.X + Get_Position()->x;
            pos.y = v.Y + Get_Position()->y;
            pos.z = v.Z + Get_Position()->z;
            Add_Icon(&pos,
                g_theWriteableGlobalData->m_debugVisibilityTileWidth,
                g_theWriteableGlobalData->m_debugVisibilityTileDuration,
                g_theWriteableGlobalData->m_debugVisibilityTileTargettableColor);
        }
    }
#endif
    return m_visionRange;
}

void Object::Handle_Partition_Cell_Maintenance()
{
#ifdef GAME_DLL
    Call_Method<void, Object>(PICK_ADDRESS(0x0054CD30, 0x007D6DFE), this);
#endif
}

bool Object::Is_Inside(const PolygonTrigger *trigger) const
{
    captainslog_dbgassert(!Is_KindOf(KINDOF_INERT), "Asking whether an inert is inside a trigger area. This is invalid.");

    for (int i = 0; i < m_numTriggerAreasActive; i++) {
        if (m_triggerInfo[i].inside && m_triggerInfo[i].polygon_trigger == trigger) {
            return true;
        }
    }

    return false;
}

bool Object::Did_Enter(const PolygonTrigger *trigger) const
{
    if (!Did_Enter_Or_Exit()) {
        return false;
    }

    captainslog_dbgassert(!Is_KindOf(KINDOF_INERT), "Asking whether an inert object entered or exited. This is invalid.");

    for (int i = 0; i < m_numTriggerAreasActive; i++) {
        if (m_triggerInfo[i].entered && m_triggerInfo[i].polygon_trigger == trigger) {
            return true;
        }
    }

    return false;
}

bool Object::Did_Enter_Or_Exit() const
{
    if (Is_KindOf(KINDOF_INERT)) {
        return false;
    }

    unsigned int frame = g_theGameLogic->Get_Frame();
    return m_enteredOrExited == frame || m_enteredOrExited == frame - 1;
}

bool Object::Did_Exit(const PolygonTrigger *trigger) const
{
    if (!Did_Enter_Or_Exit()) {
        return false;
    }

    captainslog_dbgassert(!Is_KindOf(KINDOF_INERT), "Asking whether an inert object entered or exited. This is invalid.");

    for (int i = 0; i < m_numTriggerAreasActive; i++) {
        if (m_triggerInfo[i].exited && m_triggerInfo[i].polygon_trigger == trigger) {
            return true;
        }
    }

    return false;
}

CanAttackResult Object::Get_Able_To_Use_Weapon_Against_Target(
    AbleToAttackType type, const Object *obj, const Coord3D *pos, CommandSourceType source, WeaponSlotType slot) const
{
    return m_weaponSet.Get_Able_To_Use_Weapon_Against_Target(type, this, obj, pos, source, slot);
}

SpawnBehaviorInterface *Object::Get_Spawn_Behavior_Interface() const
{
    for (BehaviorModule **module = m_allModules; *module != nullptr; module++) {
        SpawnBehaviorInterface *behavior = (*module)->Get_Spawn_Behavior_Interface();

        if (behavior != nullptr) {
            return behavior;
        }
    }

    return nullptr;
}

bool Object::Has_Countermeasures() const
{
#ifdef GAME_DLL
    return Call_Method<bool, Object const>(PICK_ADDRESS(0x0054F460, 0x007D981D), this);
#else
    return false;
#endif
}

SpecialPowerCompletionDie *Object::Find_Special_Power_Completion_Die() const
{
#ifdef GAME_DLL
    return Call_Method<SpecialPowerCompletionDie *, Object const>(PICK_ADDRESS(0x0054E8E0, 0x007D8D93), this);
#else
    return nullptr;
#endif
}

void Object::Set_Producer(const Object *obj)
{
    if (obj != nullptr) {
        m_producerID = obj->Get_ID();
    } else {
        m_producerID = OBJECT_UNK;
    }
}

void Object::Report_Missile_For_Countermeasures(Object *obj)
{
#ifdef GAME_DLL
    Call_Method<void, Object, Object *>(PICK_ADDRESS(0x0054F4A0, 0x007D9855), this, obj);
#endif
}

float Object::Estimate_Damage(DamageInfoInput &info) const
{
    const BodyModuleInterface *body = Get_Body_Module();

    if (body == nullptr) {
        return 0.0f;
    } else {
        return body->Estimate_Damage(info);
    }
}

int Object::Get_Num_Consecutive_Shots_Fired_At_Target(const Object *target) const
{
#ifdef GAME_DLL
    return Call_Method<int, const Object, const Object *>(PICK_ADDRESS(0x0054E950, 0x007D8DDC), this, target);
#else
    return 0;
#endif
}

void Object::Update_Upgrade_Modules()
{
#ifdef GAME_DLL
    Call_Method<void, Object>(PICK_ADDRESS(0x00548E90, 0x007D25F5), this);
#endif
}

void Object::Friend_Adjust_Power_For_Player(bool power)
{
    if (!Is_Disabled() || Get_Template()->Get_Energy_Production() <= 0) {
        Energy *energy = Get_Controlling_Player()->Get_Energy();

        if (power) {
            energy->Object_Entering_Influence(this);
        } else {
            energy->Object_Leaving_Influence(this);
        }
    }
}

void Object::Set_Weapon_Bonus_Condition(WeaponBonusConditionType bonus)
{
    unsigned int condition = m_weaponBonusCondition;
    m_weaponBonusCondition = (1 << bonus) | condition;

    if (condition != m_weaponBonusCondition) {
        m_weaponSet.Weapon_Set_On_Weapon_Bonus_Change(this);
    }
}

void Object::Set_Vision_Range(float range)
{
    m_visionRange = range;
}

void Object::Set_Shroud_Clearing_Range(float range)
{
    if (range != m_shroudClearingRange) {
        m_shroudClearingRange = range;
        const Coord3D *pos = Get_Position();

        if (pos->x != 0.0f || pos->y != 0.0f || pos->z != 0.0f) {
            Handle_Partition_Cell_Maintenance();
        }
    }
}

void Object::Clear_Weapon_Bonus_Condition(WeaponBonusConditionType bonus)
{
    unsigned int condition = m_weaponBonusCondition;
    m_weaponBonusCondition = ~(1 << bonus) & condition;

    if (condition != m_weaponBonusCondition) {
        m_weaponSet.Weapon_Set_On_Weapon_Bonus_Change(this);
    }
}

float Object::Get_Shroud_Clearing_Range() const
{
    float range = m_shroudClearingRange;

    if (Get_Status_Bits().Test(OBJECT_STATUS_UNDER_CONSTRUCTION)) {
        range = Get_Geometry_Info().Get_Bounding_Circle_Radius();
    }

#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData->m_debugVisibility) {
        Vector3 v(range, 0.0f, 0.0f);

        for (int i = 0; i < g_theWriteableGlobalData->m_debugVisibilityTileCount; i++) {
            float f1 = i * 1.0f / g_theWriteableGlobalData->m_debugVisibilityTileCount;
            float angle = (f1 + f1) * GAMEMATH_PI;
            v.Rotate_Z(angle);
            Coord3D pos;
            pos.x = v.X + Get_Position()->x;
            pos.y = v.Y + Get_Position()->y;
            pos.z = v.Z + Get_Position()->z;
            Add_Icon(&pos,
                g_theWriteableGlobalData->m_debugVisibilityTileWidth,
                g_theWriteableGlobalData->m_debugVisibilityTileDuration,
                g_theWriteableGlobalData->m_debugVisibilityTileTargettableColor);
        }
    }
#endif
    return range;
}

bool Object::Has_Any_Special_Power() const
{
    return m_specialPowers.Any();
}

bool Object::Has_Special_Power(SpecialPowerType spt) const
{
    return m_specialPowers.Test(spt);
}

SpecialPowerModuleInterface *Object::Find_Special_Power_Module_Interface(SpecialPowerType type) const
{
    for (BehaviorModule **module = m_allModules; *module != nullptr; module++) {
        SpecialPowerModuleInterface *power = (*module)->Get_Special_Power();

        if (power != nullptr) {
            const SpecialPowerTemplate *tmplate = power->Get_Special_Power_Template();

            if (tmplate != nullptr) {
                if (tmplate->Get_Type() == type) {
                    return power;
                }

                if (type == SPECIAL_INVALID) {
                    return power;
                }
            }
        }
    }

    return nullptr;
}

SpecialPowerModuleInterface *Object::Find_Any_Shortcut_Special_Power_Module_Interface() const
{
    for (BehaviorModule **module = m_allModules; *module != nullptr; module++) {
        SpecialPowerModuleInterface *power = (*module)->Get_Special_Power();

        if (power != nullptr) {
            const SpecialPowerTemplate *tmplate = power->Get_Special_Power_Template();

            if (tmplate != nullptr) {
                if (tmplate->Get_Shortcut_Power()) {
                    return power;
                }
            }
        }
    }

    return nullptr;
}

bool Object::Has_Any_Weapon() const
{
    return m_weaponSet.Has_Any_Weapon();
}

unsigned int Object::Get_Most_Percent_Ready_To_Fire_Any_Weapon() const
{
    return m_weaponSet.Get_Most_Percent_Ready_To_Fire_Any_Weapon();
}

void Object::Leave_Group()
{
    if (m_aiGroup != nullptr) {
        AIGroup *group = m_aiGroup;
        m_aiGroup = nullptr;
        group->Remove(this);
    }
}

bool Object::Set_Script_Status(ObjectScriptStatusBit bit, bool set)
{
#ifdef GAME_DLL
    return Call_Method<bool, Object, ObjectScriptStatusBit, bool>(PICK_ADDRESS(0x00547100, 0x007D0360), this, bit, set);
#else
    return false;
#endif
}

bool Object::Is_Faction_Structure() const
{
    return Is_Any_KindOf(KINDOFMASK_FS);
}

ProductionUpdateInterface *Object::Get_Production_Update_Interface()
{
    for (BehaviorModule **module = m_allModules; *module != nullptr; module++) {
        ProductionUpdateInterface *update = (*module)->Get_Production_Update_Interface();

        if (update != nullptr) {
            return update;
        }
    }

    return nullptr;
}

void Object::Set_Vision_Spied(bool vision_spied, int player_index)
{
    bool changed = false;

    if (vision_spied) {
        if (++m_spiedOnByPlayer[player_index] == 1) {
            changed = true;
        }
    } else if (--m_spiedOnByPlayer[player_index] == 0) {
        changed = true;
    }

    if (changed) {
        unsigned short flags = 0;

        for (int i = 0; i < MAX_PLAYER_COUNT; i++) {
            if (m_spiedOnByPlayer[i] <= 0) {
                flags &= ~(1 << i);
            } else {
                flags |= 1 << i;
            }
        }

        m_spiedOnPlayers = flags;
        Handle_Partition_Cell_Maintenance();
    }
}
