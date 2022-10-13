/**
 * @file
 *
 * @author Jonathan Wilson
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
#pragma once

#include "always.h"
#include "bitflags.h"
#include "gametype.h"
#include "snapshot.h"

class Coord3D;
class Object;
class Weapon;
class WeaponTemplateSet;

enum DamageType
{
    DAMAGE_EXPLOSION,
    DAMAGE_CRUSH,
    DAMAGE_ARMOR_PIERCING,
    DAMAGE_SMALL_ARMS,
    DAMAGE_GATTLING,
    DAMAGE_RADIATION,
    DAMAGE_FLAME,
    DAMAGE_LASER,
    DAMAGE_SNIPER,
    DAMAGE_POISON,
    DAMAGE_HEALING,
    DAMAGE_UNRESISTABLE,
    DAMAGE_WATER,
    DAMAGE_DEPLOY,
    DAMAGE_SURRENDER,
    DAMAGE_HACK,
    DAMAGE_KILL_PILOT,
    DAMAGE_PENALTY,
    DAMAGE_FALLING,
    DAMAGE_MELEE,
    DAMAGE_DISARM,
    DAMAGE_HAZARD_CLEANUP,
    DAMAGE_PARTICLE_BEAM,
    DAMAGE_TOPPLING,
    DAMAGE_INFANTRY_MISSILE,
    DAMAGE_AURORA_BOMB,
    DAMAGE_LAND_MINE,
    DAMAGE_JET_MISSILES,
    DAMAGE_STEALTHJET_MISSILES,
    DAMAGE_MOLOTOV_COCKTAIL,
    DAMAGE_COMANCHE_VULCAN,
    DAMAGE_SUBDUAL_MISSILE,
    DAMAGE_SUBDUAL_VEHICLE,
    DAMAGE_SUBDUAL_BUILDING,
    DAMAGE_SUBDUAL_UNRESISTABLE,
    DAMAGE_MICROWAVE,
    DAMAGE_KILL_GARRISONED,
    DAMAGE_STATUS,
    DAMAGE_NUM_TYPES,
};

enum WeaponSetConditionType
{
    WSF_NONE,
    WSF_FIRING,
    WSF_BETWEEN,
    WSF_RELOADING,
    WSF_PREATTACK,
    WSF_COUNT,
};

enum WeaponChoiceCriteria
{
    PREFER_MOST_DAMAGE,
    PREFER_LONGEST_RANGE,
};

enum WeaponLockType
{
    NOT_LOCKED,
    LOCKED_LEVEL_1,
    LOCKED_LEVEL_2,
};

enum CanAttackResult
{
    ATTACK_RESULT_0 = 0,
    ATTACK_RESULT_1,
    ATTACK_RESULT_2,
    ATTACK_RESULT_3,
};

enum AbleToAttackType
{
    ATTACK_TYPE_0 = 0,
    ATTACK_TYPE_1,
    ATTACK_TYPE_2,
    ATTACK_TYPE_3,
    ATTACK_TYPE_4,
};

class WeaponSet : public SnapShot
{
public:
    WeaponSet();
    ~WeaponSet();

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    void Update_Weapon_Set(const Object *obj);
    BitFlags<MODELCONDITION_COUNT> Get_Model_Condition_For_Weapon_Slot(
        WeaponSlotType slot, WeaponSetConditionType condition);
    void Weapon_Set_On_Weapon_Bonus_Change(const Object *obj);
    bool Is_Any_Within_Target_Pitch(const Object *obj, const Object *obj2) const;
    bool Choose_Best_Weapon_For_Target(
        const Object *obj, const Object *obj2, WeaponChoiceCriteria criteria, CommandSourceType source);
    void Reload_All_Ammo(const Object *obj, bool now);
    bool Is_Out_Of_Ammo() const;
    const Weapon *Find_Ammo_Pip_Showing_Weapon() const;
    Weapon *Find_Waypoint_Following_Capable_Weapon();
    unsigned int Get_Most_Percent_Ready_To_Fire_Any_Weapon() const;
    CanAttackResult Get_Able_To_Attack_Specific_Object(
        AbleToAttackType type, const Object *obj, const Object *obj2, CommandSourceType source, WeaponSlotType slot) const;
    CanAttackResult Get_Able_To_Use_Weapon_Against_Target(AbleToAttackType type,
        const Object *obj,
        const Object *obj2,
        const Coord3D *location,
        CommandSourceType source,
        WeaponSlotType slot) const;
    bool Set_Weapon_Lock(WeaponSlotType slot, WeaponLockType lock);
    void Release_Weapon_Lock(WeaponLockType lock);
    Weapon *Get_Weapon_In_Weapon_Slot(WeaponSlotType slot) const;
    void Clear_Leech_Range_Mode_For_All_Weapons();
    bool Is_Shared_Reload_Time() const;

    bool Is_Cur_Weapon_Locked() const { return m_curWeaponLocked != NOT_LOCKED; }
    bool Has_Any_Weapon() const { return m_filledWeaponSlotMask != 0; }
    bool Has_Weapon_To_Deal_Damage_Type(DamageType type) { return m_damageTypes.Test(type); }
    WeaponSlotType Get_Cur_Weapon_Slot() const { return m_curWeapon; }
    const Weapon *Get_Cur_Weapon() const { return m_weapons[m_curWeapon]; }
    Weapon *Get_Cur_Weapon() { return m_weapons[m_curWeapon]; }

private:
    WeaponTemplateSet *m_curWeaponTemplateSet;
    Weapon *m_weapons[WEAPONSLOT_COUNT];
    WeaponSlotType m_curWeapon;
    int m_curWeaponLocked;
    unsigned int m_filledWeaponSlotMask;
    int m_totalAntiMask;
    BitFlags<DAMAGE_NUM_TYPES> m_damageTypes;
    bool m_pitchLimited;
    bool m_damageWeapon;
};
