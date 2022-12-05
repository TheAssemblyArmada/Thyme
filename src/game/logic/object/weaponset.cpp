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
    m_curWeaponLocked(WEAPONSLOT_PRIMARY),
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
    WeaponSlotType slot, WeaponSetConditionType condition)
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

    ModelConditionFlagType flag = Lookup[condition][slot];

    if (flag != MODELCONDITION_INVALID) {
        flags.Set(flag, true);
    }

    if (condition != WSF_NONE) {
        flags.Set(Using[slot], true);
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

#if 0
void WeaponSet::Update_Weapon_Set(const Object *obj) {}

void WeaponSet::Weapon_Set_On_Weapon_Bonus_Change(const Object *obj) {}

bool WeaponSet::Is_Any_Within_Target_Pitch(const Object *obj, const Object *obj2) const {}

bool WeaponSet::Choose_Best_Weapon_For_Target(
    const Object *obj, const Object *obj2, WeaponChoiceCriteria criteria, CommandSourceType source)
{
}

void WeaponSet::Reload_All_Ammo(const Object *obj, bool now) {}

bool WeaponSet::Is_Out_Of_Ammo() const {}

const Weapon *WeaponSet::Find_Ammo_Pip_Showing_Weapon() const {}

Weapon *WeaponSet::Find_Waypoint_Following_Capable_Weapon() {}

unsigned int WeaponSet::Get_Most_Percent_Ready_To_Fire_Any_Weapon() const {}

CanAttackResult WeaponSet::Get_Able_To_Attack_Specific_Object(
    AbleToAttackType type, const Object *obj, const Object *obj2, CommandSourceType source, WeaponSlotType slot) const
{
}

CanAttackResult WeaponSet::Get_Able_To_Use_Weapon_Against_Target(AbleToAttackType type,
    const Object *obj,
    const Object *obj2,
    const Coord3D *location,
    CommandSourceType source,
    WeaponSlotType slot) const
{
}

bool WeaponSet::Set_Weapon_Lock(WeaponSlotType slot, WeaponLockType lock) {}

void WeaponSet::Release_Weapon_Lock(WeaponLockType lock) {}

void WeaponSet::Clear_Leech_Range_Mode_For_All_Weapons() {}
#endif

Weapon *WeaponSet::Get_Weapon_In_Weapon_Slot(WeaponSlotType slot) const
{
    return m_weapons[slot];
}
