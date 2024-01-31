/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Weapon Set
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "weaponset.h"
#include "object.h"
#include "opencontain.h"
#include "player.h"
#include "playerlist.h"
#include "spawnbehavior.h"
#include "stealthupdate.h"
#include "team.h"
#include "thingfactory.h"
#include "weapon.h"
#include "weapontemplateset.h"

template<>
const char *BitFlags<DAMAGE_NUM_TYPES>::s_bitNamesList[] = {
    "EXPLOSION",
    "CRUSH",
    "ARMOR_PIERCING",
    "SMALL_ARMS",
    "GATTLING",
    "RADIATION",
    "FLAME",
    "LASER",
    "SNIPER",
    "POISON",
    "HEALING",
    "UNRESISTABLE",
    "WATER",
    "DEPLOY",
    "SURRENDER",
    "HACK",
    "KILL_PILOT",
    "PENALTY",
    "FALLING",
    "MELEE",
    "DISARM",
    "HAZARD_CLEANUP",
    "PARTICLE_BEAM",
    "TOPPLING",
    "INFANTRY_MISSILE",
    "AURORA_BOMB",
    "LAND_MINE",
    "JET_MISSILES",
    "STEALTHJET_MISSILES",
    "MOLOTOV_COCKTAIL",
    "COMANCHE_VULCAN",
    "SUBDUAL_MISSILE",
    "SUBDUAL_VEHICLE",
    "SUBDUAL_BUILDING",
    "SUBDUAL_UNRESISTABLE",
    "MICROWAVE",
    "KILL_GARRISONED",
    "STATUS",
    nullptr,
};

WeaponSet::WeaponSet() :
    m_curWeaponTemplateSet(nullptr),
    m_curWeapon(WEAPONSLOT_PRIMARY),
    m_curWeaponLocked(NOT_LOCKED),
    m_filledWeaponSlotMask(0),
    m_totalAntiMask(0),
    m_pitchLimited(false),
    m_damageWeapon(false)
{
    m_damageTypes.Clear();

    for (int i = 0; i < WEAPONSLOT_COUNT; i++) {
        m_weapons[i] = nullptr;
    }
}

WeaponSet::~WeaponSet()
{
    for (int i = 0; i < WEAPONSLOT_COUNT; i++) {
        if (m_weapons[i] != nullptr) {
            m_weapons[i]->Delete_Instance();
        }
    }
}

void WeaponSet::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);

    if (xfer->Get_Mode() == XFER_LOAD) {
        Utf8String name;
        BitFlags<WEAPONSET_COUNT> flags;
        xfer->xferAsciiString(&name);
        flags.Xfer(xfer);

        if (name.Is_Empty()) {
            m_curWeaponTemplateSet = nullptr;
        } else {
            ThingTemplate *tmplate = g_theThingFactory->Find_Template(name, true);

            if (tmplate == nullptr) {
                throw CODE_06;
            }

            m_curWeaponTemplateSet = tmplate->Find_Weapon_Template_Set(flags);

            if (m_curWeaponTemplateSet == nullptr) {
                throw CODE_06;
            }
        }
    } else if (xfer->Get_Mode() == XFER_SAVE) {
        Utf8String name;
        BitFlags<WEAPONSET_COUNT> flags;

        if (m_curWeaponTemplateSet != nullptr) {
            const ThingTemplate *tmplate = m_curWeaponTemplateSet->Get_Type();

            if (tmplate == nullptr) {
                throw CODE_06;
            }

            name = tmplate->Get_Name();
            flags = m_curWeaponTemplateSet->Get_Conditions();
        }

        xfer->xferAsciiString(&name);
        flags.Xfer(xfer);
    }

    for (int i = 0; i < WEAPONSLOT_COUNT; i++) {
        bool has_weapon = m_weapons[i] != nullptr;
        xfer->xferBool(&has_weapon);

        if (has_weapon) {
            if (xfer->Get_Mode() == XFER_LOAD && !m_weapons[i]) {
                WeaponTemplate *weapon = m_curWeaponTemplateSet->Get_Nth(static_cast<WeaponSlotType>(i));

                if (weapon == nullptr) {
                    captainslog_dbgassert(false, "xfer backwards compatibility code - old save file???");
                    weapon = m_curWeaponTemplateSet->Get_Nth(WEAPONSLOT_PRIMARY);
                }

                m_weapons[i] = g_theWeaponStore->Allocate_New_Weapon(weapon, static_cast<WeaponSlotType>(i));
            }

            m_weapons[i]->Xfer_Snapshot(xfer);
        }
    }

    xfer->xferUser(&m_curWeapon, sizeof(m_curWeapon));
    xfer->xferUser(&m_curWeaponLocked, sizeof(m_curWeaponLocked));
    xfer->xferUnsignedInt(&m_filledWeaponSlotMask);
    xfer->xferInt(&m_totalAntiMask);
    xfer->xferBool(&m_damageWeapon);
    xfer->xferBool(&m_damageWeapon);
    m_damageTypes.Xfer(xfer);
}

BitFlags<MODELCONDITION_COUNT> WeaponSet::Get_Model_Condition_For_Weapon_Slot(
    WeaponSlotType wslot, WeaponSetConditionType condition)
{
    static_assert(WEAPONSLOT_COUNT == 3, "WEAPONSLOT_COUNT is wrong size");
    static_assert(WSF_COUNT == 5, "WSF_COUNT is wrong size");

    static ModelConditionFlagType Nothing[WEAPONSLOT_COUNT] = {
        MODELCONDITION_INVALID, MODELCONDITION_INVALID, MODELCONDITION_INVALID
    };
    static ModelConditionFlagType Firing[WEAPONSLOT_COUNT] = {
        MODELCONDITION_FIRING_A, MODELCONDITION_FIRING_B, MODELCONDITION_FIRING_C
    };
    static ModelConditionFlagType Betweening[WEAPONSLOT_COUNT] = {
        MODELCONDITION_BETWEEN_FIRING_SHOTS_A, MODELCONDITION_BETWEEN_FIRING_SHOTS_B, MODELCONDITION_BETWEEN_FIRING_SHOTS_C
    };
    static ModelConditionFlagType Reloading[WEAPONSLOT_COUNT] = {
        MODELCONDITION_RELOADING_A, MODELCONDITION_RELOADING_B, MODELCONDITION_RELOADING_C
    };
    static ModelConditionFlagType PreAttack[WEAPONSLOT_COUNT] = {
        MODELCONDITION_PREATTACK_A, MODELCONDITION_PREATTACK_B, MODELCONDITION_PREATTACK_C
    };
    static ModelConditionFlagType Using[WEAPONSLOT_COUNT] = {
        MODELCONDITION_USING_WEAPON_A, MODELCONDITION_USING_WEAPON_B, MODELCONDITION_USING_WEAPON_C
    };
    static ModelConditionFlagType *Lookup[WSF_COUNT] = { Nothing, Firing, Betweening, Reloading, PreAttack };

    BitFlags<MODELCONDITION_COUNT> flags;

    ModelConditionFlagType flag = Lookup[condition][wslot];

    if (flag != MODELCONDITION_INVALID) {
        flags.Set(flag, true);
    }

    if (condition != WSF_NONE) {
        flags.Set(Using[wslot], true);
    }

    return flags;
}

bool WeaponSet::Is_Share_Weapon_Reload_Time() const
{
    if (m_curWeaponTemplateSet != nullptr) {
        return m_curWeaponTemplateSet->Is_Share_Weapon_Reload_Time();
    }

    return false;
}

bool WeaponSet::Has_Any_Damage_Weapon() const
{
    return m_damageWeapon;
}

void WeaponSet::Update_Weapon_Set(const Object *source_obj)
{
    const WeaponTemplateSet *templateset =
        source_obj->Get_Template()->Find_Weapon_Template_Set(source_obj->Get_Weapon_Set_Flags());
    captainslog_dbgassert(templateset != nullptr, "findWeaponSet should never return null");

    if (templateset != nullptr && templateset != m_curWeaponTemplateSet) {
        if (!templateset->Is_Lock_Shared_Across_Sets()) {
            if (Is_Cur_Weapon_Locked()) {
                captainslog_debug("changing WeaponSet while Weapon is Locked... implicit unlock occurring!");
            }

            Release_Weapon_Lock(LOCKED_LEVEL_2);
            m_curWeapon = WEAPONSLOT_PRIMARY;
        }

        m_filledWeaponSlotMask = 0;
        m_totalAntiMask = 0;
        m_damageTypes.Clear();
        m_pitchLimited = false;
        m_damageWeapon = false;

        for (int i = WEAPONSLOT_TERTIARY; i >= WEAPONSLOT_PRIMARY; i--) {
            if (m_weapons[i] != nullptr) {
                m_weapons[i]->Delete_Instance();
                m_weapons[i] = nullptr;
            }

            if (templateset->Get_Nth(static_cast<WeaponSlotType>(i)) != nullptr) {
                m_weapons[i] = g_theWeaponStore->Allocate_New_Weapon(
                    templateset->Get_Nth(static_cast<WeaponSlotType>(i)), static_cast<WeaponSlotType>(i));
                m_weapons[i]->Load_Ammo_Now(source_obj);
                m_filledWeaponSlotMask |= 1 << i;
                m_totalAntiMask |= m_weapons[i]->Get_Anti_Mask();
                m_damageTypes.Set(m_weapons[i]->Get_Damage_Type(), 1);

                if (m_weapons[i]->Is_Pitch_Limited()) {
                    m_pitchLimited = true;
                }

                if (m_weapons[i]->Is_Damage_Weapon()) {
                    m_damageWeapon = true;
                }
            }
        }

        m_curWeaponTemplateSet = templateset;
    }
}

void WeaponSet::Weapon_Set_On_Weapon_Bonus_Change(const Object *source_obj)
{
    for (int i = 0; i < WEAPONSLOT_COUNT; i++) {
        if (m_weapons[i] != nullptr) {
            m_weapons[i]->On_Weapon_Bonus_Change(source_obj);
        }
    }
}

bool WeaponSet::Is_Any_Within_Target_Pitch(const Object *source_obj, const Object *victim_obj) const
{
    if (!m_pitchLimited) {
        return true;
    }

    for (int i = 0; i < WEAPONSLOT_COUNT; i++) {
        if (m_weapons[i] != nullptr && m_weapons[i]->Is_Within_Target_Pitch(source_obj, victim_obj)) {
            return true;
        }
    }

    return false;
}

void WeaponSet::Reload_All_Ammo(const Object *source_obj, bool now)
{
    for (int i = 0; i < WEAPONSLOT_COUNT; i++) {
        if (m_weapons[i] != nullptr) {
            if (now) {
                m_weapons[i]->Load_Ammo_Now(source_obj);
            } else {
                m_weapons[i]->Reload_Ammo(source_obj);
            }
        }
    }
}

bool WeaponSet::Is_Out_Of_Ammo() const
{
    for (int i = 0; i < WEAPONSLOT_COUNT; i++) {
        if (m_weapons[i] != nullptr && m_weapons[i]->Get_Status() != Weapon::OUT_OF_AMMO) {
            return false;
        }
    }

    return true;
}

const Weapon *WeaponSet::Find_Ammo_Pip_Showing_Weapon() const
{
    for (int i = 0; i < WEAPONSLOT_COUNT; i++) {
        if (m_weapons[i] != nullptr && m_weapons[i]->Is_Show_Ammo_Pips()) {
            return m_weapons[i];
        }
    }

    return nullptr;
}

Weapon *WeaponSet::Find_Waypoint_Following_Capable_Weapon()
{
    for (int i = 0; i < WEAPONSLOT_COUNT; i++) {
        if (m_weapons[i] != nullptr && m_weapons[i]->Is_Capable_Of_Following_Waypoint()) {
            return m_weapons[i];
        }
    }

    return nullptr;
}

bool WeaponSet::Set_Weapon_Lock(WeaponSlotType wslot, WeaponLockType lock)
{
    if (lock != NOT_LOCKED) {
        if (m_weapons[wslot] != nullptr) {
            if (lock == LOCKED_LEVEL_2) {
                m_curWeapon = wslot;
                m_curWeaponLocked = LOCKED_LEVEL_2;
            } else if (lock == LOCKED_LEVEL_1 && m_curWeaponLocked != LOCKED_LEVEL_2) {
                m_curWeapon = wslot;
                m_curWeaponLocked = LOCKED_LEVEL_1;
            }

            return true;
        } else {
            captainslog_dbgassert(false, "setWeaponLock: weapon %d not found (missing an upgrade?)", wslot);
            return false;
        }
    } else {
        captainslog_dbgassert(false,
            "calling setWeaponLock with NOT_LOCKED, so I am doing nothing... did you mean to use releaseWeaponLock()?");
        return false;
    }
}

void WeaponSet::Release_Weapon_Lock(WeaponLockType lock)
{
    if (m_curWeaponLocked != NOT_LOCKED) {
        if (lock == LOCKED_LEVEL_2) {
            m_curWeaponLocked = NOT_LOCKED;
        } else if (lock == LOCKED_LEVEL_1) {
            if (m_curWeaponLocked == LOCKED_LEVEL_1) {
                m_curWeaponLocked = NOT_LOCKED;
            }
        } else {
            captainslog_dbgassert(false, "calling releaseWeaponLock with NOT_LOCKED makes no sense. why did you do this?");
        }
    }
}

void WeaponSet::Clear_Leech_Range_Mode_For_All_Weapons()
{
    for (int i = 0; i < WEAPONSLOT_COUNT; i++) {
        if (m_weapons[i] != nullptr) {
            m_weapons[i]->Set_Leech_Range_Active(false);
        }
    }
}

unsigned int WeaponSet::Get_Most_Percent_Ready_To_Fire_Any_Weapon() const
{
    unsigned int percent = 0;

    for (int i = 0; i < WEAPONSLOT_COUNT; i++) {
        if (m_weapons[i] != nullptr) {
            unsigned int fire = m_weapons[i]->Get_Percent_Ready_To_Fire() * 100.0f;

            if (fire > percent) {
                percent = fire;
            }

            if (percent >= 100) {
                return percent;
            }
        }
    }

    return percent;
}

Weapon *WeaponSet::Get_Weapon_In_Weapon_Slot(WeaponSlotType wslot) const
{
    return m_weapons[wslot];
}

bool Is_Attack_Type_1(AbleToAttackType type)
{
    return (type & ATTACK_TYPE_1) != 0;
}

CanAttackResult WeaponSet::Get_Able_To_Attack_Specific_Object(AbleToAttackType type,
    const Object *source_obj,
    const Object *victim_obj,
    CommandSourceType source,
    WeaponSlotType wslot) const
{
    if (source_obj == nullptr || victim_obj == nullptr || source_obj->Is_Effectively_Dead()
        || victim_obj->Is_Effectively_Dead() || source_obj->Is_Destroyed() || victim_obj->Is_Destroyed()
        || victim_obj == source_obj) {
        return ATTACK_RESULT_CANNOT_ATTACK;
    }

    bool same_player =
        source_obj->Get_Controlling_Player() == victim_obj->Get_Controlling_Player() && Is_Attack_Type_1(type);

    if (victim_obj->Get_Status(OBJECT_STATUS_MASKED)) {
        return ATTACK_RESULT_CANNOT_ATTACK;
    }

    if (victim_obj->Is_KindOf(KINDOF_UNATTACKABLE)) {
        return ATTACK_RESULT_CANNOT_ATTACK;
    }

    if (victim_obj->Get_Status(OBJECT_STATUS_NO_ATTACK_FROM_AI) && source == COMMANDSOURCE_AI) {
        return ATTACK_RESULT_CANNOT_ATTACK;
    }

    bool check_stealth = true;

    if (source_obj->Get_Status(OBJECT_STATUS_IGNORING_STEALTH) || same_player) {
        check_stealth = false;
    }

    if (Is_Attack_Type_1(type) && victim_obj->Is_KindOf(KINDOF_DISGUISER)
        && victim_obj->Get_Status(OBJECT_STATUS_DISGUISED)) {
        check_stealth = false;
    }

    if (!check_stealth || !victim_obj->Get_Status(OBJECT_STATUS_STEALTHED)
        || victim_obj->Get_Status(OBJECT_STATUS_DETECTED)) {
        goto l1;
    }

    if (!victim_obj->Is_KindOf(KINDOF_DISGUISER)) {
        return ATTACK_RESULT_CANNOT_ATTACK;
    }

    {
        StealthUpdate *update = victim_obj->Get_Stealth_Update();

        if (update != nullptr) {
            if (update->Has_Disguised_Template()) {
                Player *player = source_obj->Get_Controlling_Player();
                Player *player2 = g_thePlayerList->Get_Nth_Player(update->Get_Player_Index());

                if (player != nullptr) {
                    if (player2 != nullptr) {
                        if (player->Get_Relationship(player2->Get_Default_Team()) != ENEMIES) {
                            return ATTACK_RESULT_CANNOT_ATTACK;
                        }
                    }
                }
            }
        }
    }

l1:
    Relationship relationship = source_obj->Get_Relationship(victim_obj);

    if (relationship != ENEMIES && !Is_Attack_Type_1(type) && (!victim_obj->Is_KindOf(KINDOF_MINE) || relationship == ALLIES)
        && source == COMMANDSOURCE_PLAYER && (!victim_obj->Get_Script_Status(STATUS_TARGETABLE) || relationship == ALLIES)) {
        return ATTACK_RESULT_CANNOT_ATTACK;
    }

    const Object *containedby = victim_obj->Get_Contained_By();

    if (containedby != nullptr) {
        if (containedby->Get_Contain()->Is_Enclosing_Container_For(victim_obj)) {
            return ATTACK_RESULT_CANNOT_ATTACK;
        }
    }

    if (!Is_Attack_Type_1(type)) {
        ContainModuleInterface *contain = victim_obj->Get_Contain();

        if (contain != nullptr) {
            const Player *controlling_player =
                contain->Get_Apparent_Controlling_Player(source_obj->Get_Controlling_Player());

            if (controlling_player != nullptr) {
                if (source_obj->Get_Team()->Get_Relationship(controlling_player->Get_Default_Team()) != ENEMIES) {
                    if (source == COMMANDSOURCE_PLAYER
                        && (!victim_obj->Get_Script_Status(STATUS_TARGETABLE) || relationship == ALLIES)) {
                        return ATTACK_RESULT_CANNOT_ATTACK;
                    }
                }
            }
        }
    }

    return Get_Able_To_Use_Weapon_Against_Target(type, source_obj, victim_obj, victim_obj->Get_Position(), source, wslot);
}

int Get_Victim_Anti_Mask(const Object *victim)
{
    if (victim->Is_KindOf(KINDOF_SMALL_MISSILE)) {
        return ANTI_SMALL_MISSILE;
    }

    if (victim->Is_KindOf(KINDOF_BALLISTIC_MISSILE)) {
        return ANTI_BALLISTIC_MISSILE;
    }

    if (victim->Is_KindOf(KINDOF_PROJECTILE)) {
        return ANTI_PROJECTILE;
    }

    if (victim->Is_KindOf(KINDOF_MINE) || victim->Is_KindOf(KINDOF_DEMOTRAP)) {
        return ANTI_MINE | ANTI_GROUND;
    }

    if (!victim->Is_Airborne_Target()) {
        return ANTI_GROUND;
    }

    if (victim->Is_KindOf(KINDOF_VEHICLE)) {
        return ANTI_AIRBORNE_VEHICLE;
    }

    if (victim->Is_KindOf(KINDOF_INFANTRY)) {
        return ANTI_AIRBORNE_INFANTRY;
    }

    if (victim->Is_KindOf(KINDOF_PARACHUTE)) {
        return ANTI_PARACHUTE;
    }

    captainslog_dbgassert(victim->Is_KindOf(KINDOF_UNATTACKABLE),
        "Object %s is being targetted as airborne, but is not infantry, nor vehicle. Is this legit?",
        victim->Get_Template()->Get_Name().Str());

    return 0;
}

CanAttackResult WeaponSet::Get_Able_To_Use_Weapon_Against_Target(AbleToAttackType type,
    const Object *source_obj,
    const Object *victim_obj,
    const Coord3D *location,
    CommandSourceType source,
    WeaponSlotType wslot) const
{
    int mask;

    if (victim_obj != nullptr) {
        mask = Get_Victim_Anti_Mask(victim_obj);
        location = victim_obj->Get_Position();
    } else {
        mask = ANTI_GROUND;
    }

    bool is_in_range = false;
    const Object *containedby = source_obj->Get_Contained_By();
    bool weapon_in_range = false;
    bool has_weapon = false;

    for (int i = 0; i < WEAPONSLOT_COUNT; i++) {
        Weapon *weapon = m_weapons[m_curWeapon];

        if (weapon != nullptr) {
            has_weapon = true;

            if ((mask & m_totalAntiMask) != 0) {
                bool is_garrison = false;
                ContainModuleInterface *contain = containedby != nullptr ? containedby->Get_Contain() : nullptr;

                if (contain != nullptr && contain->Is_Garrisonable() && contain->Is_Enclosing_Container_For(source_obj)) {
                    Coord3D loc = *location;
                    Coord3D garrison_pos;

                    if (contain->Calc_Best_Garrison_Position(&garrison_pos, &loc)) {
                        is_in_range = weapon->Is_Source_Object_With_Goal_Position_Within_Attack_Range(
                            source_obj, &garrison_pos, victim_obj, &loc);
                        is_garrison = true;
                    }
                } else {
                    is_in_range = victim_obj != nullptr ? weapon->Is_Within_Attack_Range(source_obj, victim_obj) :
                                                          weapon->Is_Within_Attack_Range(source_obj, location);
                }

                if (is_in_range) {
                    weapon_in_range = true;
                    break;
                }
            }
        }
    }

    if ((source_obj->Is_KindOf(KINDOF_IMMOBILE) || source_obj->Is_KindOf(KINDOF_SPAWNS_ARE_THE_WEAPONS)
            || containedby != nullptr)
        && has_weapon && !weapon_in_range && type != ATTACK_TYPE_4) {
        return ATTACK_RESULT_UNREACHABLE;
    }

    CanAttackResult result = is_in_range ? ATTACK_RESULT_CAN_ATTACK : ATTACK_RESULT_OUT_OF_RANGE;

    if (Has_Any_Damage_Weapon()) {
        if ((mask & m_totalAntiMask) == 0) {
            return ATTACK_RESULT_UNREACHABLE;
        }

        if (victim_obj == nullptr) {
            return result;
        }

        if (!Is_Any_Within_Target_Pitch(source_obj, victim_obj)) {
            return ATTACK_RESULT_UNREACHABLE;
        }

        WeaponSlotType start_slot;
        WeaponSlotType end_slot;

        if (Is_Cur_Weapon_Locked()) {
            start_slot = m_curWeapon;
            end_slot = m_curWeapon;
        } else if (wslot == WEAPONSLOT_UNK) {
            start_slot = WEAPONSLOT_TERTIARY;
            end_slot = WEAPONSLOT_PRIMARY;
        } else {
            start_slot = wslot;
            end_slot = wslot;
        }

        for (int i = start_slot; i >= end_slot; i--) {
            Weapon *weapon2 = m_weapons[i];

            if (weapon2 != nullptr && weapon2->Estimate_Weapon_Damage(source_obj, victim_obj) != 0.0f
                && (weapon2->Get_Damage_Type() != DAMAGE_KILL_PILOT || !source_obj->Is_KindOf(KINDOF_HERO)
                    || m_curWeapon != WEAPONSLOT_PRIMARY || wslot != WEAPONSLOT_UNK)) {
                return result;
            }
        }
    }

    ContainModuleInterface *contain = source_obj->Get_Contain();

    if (contain != nullptr) {
        if (contain->Is_Passenger_Allowed_To_Fire(INVALID_OBJECT_ID)) {
            const std::list<Object *> *list = contain->Get_Contained_Items_List();

            if (list != nullptr) {
                for (auto it = list->begin(); it != list->end(); it++) {
                    Object *obj = *it;

                    if (obj->Is_Able_To_Attack()) {
                        CanAttackResult can =
                            obj->Get_Able_To_Use_Weapon_Against_Target(type, victim_obj, location, source, WEAPONSLOT_UNK);

                        if (can == ATTACK_RESULT_OUT_OF_RANGE || can == ATTACK_RESULT_CAN_ATTACK) {
                            return can;
                        }
                    }
                }
            }
        }
    }

    SpawnBehaviorInterface *behavior = source_obj->Get_Spawn_Behavior_Interface();

    if (behavior == nullptr
        || behavior->Get_Can_Any_Slaves_Use_Weapon_Against_Target(type, victim_obj, location, source)
            != ATTACK_RESULT_CAN_ATTACK) {
        return ATTACK_RESULT_UNREACHABLE;
    }

    if (source_obj->Is_KindOf(KINDOF_IMMOBILE) && source_obj->Is_KindOf(KINDOF_SPAWNS_ARE_THE_WEAPONS)
        && result == ATTACK_RESULT_OUT_OF_RANGE) {
        return ATTACK_RESULT_CAN_ATTACK;
    }

    return result;
}

bool WeaponSet::Choose_Best_Weapon_For_Target(
    const Object *source_obj, const Object *victim_obj, WeaponChoiceCriteria criteria, CommandSourceType source)
{
    if (Is_Cur_Weapon_Locked()) {
        return true;
    }

    if (victim_obj != nullptr) {
        bool use_free_weapon = false;
        bool use_busy_weapon = false;
        float free_weapon_max_range = 0.0f;
        float free_weapon_max_damage = 0.0f;
        float busy_weapon_max_range = 0.0f;
        float busy_weapon_max_damage = 0.0f;
        WeaponSlotType free_weapon = WEAPONSLOT_PRIMARY;
        WeaponSlotType busy_weapon = WEAPONSLOT_PRIMARY;

        for (int i = WEAPONSLOT_TERTIARY; i >= 0; i--) {
            if (m_weapons[i] != nullptr) {
                int mask = m_curWeaponTemplateSet->Get_Nth_Auto_Choose_Mask(static_cast<WeaponSlotType>(i));

                if (((1 << source) & mask) != 0 || (mask & 4) != 0) {
                    Weapon *weapon = m_weapons[i];

                    if (weapon != nullptr
                        && (weapon->Get_Status() != Weapon::OUT_OF_AMMO || weapon->Is_Auto_Reloads_Clip())) {
                        if ((Get_Victim_Anti_Mask(victim_obj) & weapon->Get_Anti_Mask()) != 0
                            && weapon->Is_Within_Target_Pitch(source_obj, victim_obj)) {
                            float damage = weapon->Estimate_Weapon_Damage(source_obj, victim_obj);
                            float range = weapon->Get_Attack_Range(source_obj);
                            bool can_attack = weapon->Get_Status() == Weapon::READY_TO_FIRE;
                            const AIUpdateInterface *update = source_obj->Get_AI_Update_Interface();

                            if (update != nullptr
                                && update->Is_Weapon_Slot_On_Turret_And_Aiming_At_Target(
                                    static_cast<WeaponSlotType>(i), victim_obj)) {
                                can_attack = false;
                            }

                            if (damage > 0.0f || weapon->Get_Damage_Type() == DAMAGE_UNRESISTABLE) {
                                BitFlags<KINDOF_COUNT> against =
                                    m_curWeaponTemplateSet->Get_Nth_Preferred_Against_Mask(static_cast<WeaponSlotType>(i));

                                if (against.Any() && victim_obj->Is_KindOf_Multi(against, KINDOFMASK_NONE)) {
                                    damage = 10000000000.0f;
                                    range = 10000000000.0f;
                                    can_attack = weapon->Get_Status() != Weapon::OUT_OF_AMMO;
                                }

                                if (criteria != PREFER_MOST_DAMAGE) {
                                    if (criteria == PREFER_LONGEST_RANGE) {
                                        if (can_attack) {
                                            if (range > free_weapon_max_range) {
                                                free_weapon_max_range = range;
                                                free_weapon = static_cast<WeaponSlotType>(i);
                                                use_free_weapon = true;
                                            }
                                        } else if (range > busy_weapon_max_range) {
                                            busy_weapon_max_range = range;
                                            busy_weapon = static_cast<WeaponSlotType>(i);
                                            use_busy_weapon = true;
                                        }
                                    }
                                } else if (can_attack) {
                                    if (damage >= free_weapon_max_damage) {
                                        free_weapon_max_damage = damage;
                                        free_weapon = static_cast<WeaponSlotType>(i);
                                        use_free_weapon = true;
                                    }
                                } else if (damage >= busy_weapon_max_damage) {
                                    busy_weapon_max_damage = damage;
                                    busy_weapon = static_cast<WeaponSlotType>(i);
                                    use_busy_weapon = true;
                                }
                            }
                        }
                    }
                }
            }
        }

        if (use_free_weapon) {
            m_curWeapon = free_weapon;
        } else if (use_busy_weapon) {
            m_curWeapon = busy_weapon;
            return true;
        } else {
            m_curWeapon = WEAPONSLOT_PRIMARY;
        }

        return use_free_weapon;
    } else {
        m_curWeapon = WEAPONSLOT_PRIMARY;
        return true;
    }
}
