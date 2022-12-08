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

#if 0
bool WeaponSet::Choose_Best_Weapon_For_Target(
    const Object *source_obj, const Object *victim_obj, WeaponChoiceCriteria criteria, CommandSourceType source)
{
}

CanAttackResult WeaponSet::Get_Able_To_Attack_Specific_Object(
    AbleToAttackType type, const Object *source_obj, const Object *victim_obj, CommandSourceType source, WeaponSlotType wslot) const
{
}

CanAttackResult WeaponSet::Get_Able_To_Use_Weapon_Against_Target(AbleToAttackType type,
    const Object *source_obj,
    const Object *obj2,
    const Coord3D *location,
    CommandSourceType source,
    WeaponSlotType wslot) const
{
}
#endif
