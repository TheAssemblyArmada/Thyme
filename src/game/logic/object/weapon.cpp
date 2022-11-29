/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Weapon objects.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "weapon.h"
#include "gamelogic.h"
#include "ini.h"
#include "weaponset.h"

#ifndef GAME_DLL
WeaponStore *g_theWeaponStore = nullptr;
#endif

const char *TheWeaponBonusNames[] = {
    "GARRISONED",
    "HORDE",
    "CONTINUOUS_FIRE_MEAN",
    "CONTINUOUS_FIRE_FAST",
    "NATIONALISM",
    "PLAYER_UPGRADE",
    "DRONE_SPOTTING",
    "DEMORALIZED_OBSOLETE",
    "ENTHUSIASTIC",
    "VETERAN",
    "ELITE",
    "HERO",
    "BATTLEPLAN_BOMBARDMENT",
    "BATTLEPLAN_HOLDTHELINE",
    "BATTLEPLAN_SEARCHANDDESTROY",
    "SUBLIMINAL",
    "SOLO_HUMAN_EASY",
    "SOLO_HUMAN_NORMAL",
    "SOLO_HUMAN_HARD",
    "SOLO_AI_EASY",
    "SOLO_AI_NORMAL",
    "SOLO_AI_HARD",
    "TARGET_FAERIE_FIRE",
    "FANATICISM",
    "FRENZY_ONE",
    "FRENZY_TWO",
    "FRENZY_THREE",
    nullptr,
};

const char *TheWeaponBonusFieldNames[] = {
    "DAMAGE",
    "RADIUS",
    "RANGE",
    "RATE_OF_FIRE",
    "PRE_ATTACK",
    nullptr,
};

void WeaponBonusSet::Parse_Weapon_Bonus_Set_Ptr(INI *ini, void *formal, void *store, void const *user_data)
{
    WeaponBonusSet *wbs = *static_cast<WeaponBonusSet **>(store);
    wbs->Parse_Weapon_Bonus_Set(ini);
}

void WeaponBonusSet::Parse_Weapon_Bonus_Set(INI *ini, void *formal, void *store, void const *user_data)
{
    WeaponBonusSet *wbs = static_cast<WeaponBonusSet *>(store);
    wbs->Parse_Weapon_Bonus_Set(ini);
}

void WeaponBonusSet::Parse_Weapon_Bonus_Set(INI *ini)
{
    int set = INI::Scan_IndexList(ini->Get_Next_Token(), TheWeaponBonusNames);
    WeaponBonus::Field field = WeaponBonus::Field(INI::Scan_IndexList(ini->Get_Next_Token(), TheWeaponBonusFieldNames));
    m_bonus[set].Set_Field(field, INI::Scan_PercentToReal(ini->Get_Next_Token()));
}

const WeaponTemplate *WeaponStore::Find_Weapon_Template(Utf8String name) const
{
#ifdef GAME_DLL
    return Call_Method<const WeaponTemplate *, const WeaponStore, Utf8String>(
        PICK_ADDRESS(0x004C51B0, 0x006D6703), this, name);
#else
    return nullptr;
#endif
}

void WeaponStore::Create_And_Fire_Temp_Weapon(const WeaponTemplate *tmpl, const Object *obj, const Coord3D *pos)
{
#ifdef GAME_DLL
    Call_Method<void, WeaponStore, const WeaponTemplate *, const Object *, const Coord3D *>(
        PICK_ADDRESS(0x004C50A0, 0x006D6661), this, tmpl, obj, pos);
#endif
}

void WeaponStore::Parse_Weapon_Template(INI *ini, void *, void *store, const void *)
{
    const char *name = ini->Get_Next_Token();
    const WeaponTemplate *w = g_theWeaponStore->Find_Weapon_Template(name);
    captainslog_dbgassert(w != nullptr || strcasecmp(name, "None") == 0, "WeaponTemplate %s not found!", name);
    *static_cast<const WeaponTemplate **>(store) = w;
}

Weapon *WeaponStore::Allocate_New_Weapon(const WeaponTemplate *tmpl, WeaponSlotType wslot) const
{
    return new Weapon(tmpl, wslot);
}

Weapon::Weapon(const WeaponTemplate *tmpl, WeaponSlotType wslot) :
    m_template(tmpl),
    m_wslot(wslot),
    m_status(OUT_OF_AMMO),
    m_ammoInClip(0),
    m_whenWeCanFireAgain(0),
    m_whenPreAttackFinished(0),
    m_whenLastReloadStarted(0),
    m_lastFireFrame(0),
    m_projectileStreamID(OBJECT_UNK),
    m_maxShotCount(0x7FFFFFFF),
    m_curBarrel(0),
    m_leechWeaponRangeActive(false)
{
    m_pitchLimited = m_template->Get_Min_Target_Pitch() > -GAMEMATH_PI || m_template->Get_Max_Target_Pitch() < GAMEMATH_PI;
    m_numShotsForCurBarrel = m_template->Get_Shots_Per_Barrel();
    m_suspendFXDelay = m_template->Get_Suspend_FX_Delay() + g_theGameLogic->Get_Frame();
}

Weapon::Weapon(const Weapon &that) :
    m_template(that.m_template),
    m_wslot(that.m_wslot),
    m_status(OUT_OF_AMMO),
    m_ammoInClip(0),
    m_whenWeCanFireAgain(0),
    m_whenPreAttackFinished(0),
    m_whenLastReloadStarted(0),
    m_lastFireFrame(0),
    m_projectileStreamID(OBJECT_UNK),
    m_maxShotCount(0x7FFFFFFF),
    m_curBarrel(0),
    m_leechWeaponRangeActive(false)
{
    m_pitchLimited = m_template->Get_Min_Target_Pitch() > -GAMEMATH_PI || m_template->Get_Max_Target_Pitch() < GAMEMATH_PI;
    m_numShotsForCurBarrel = m_template->Get_Shots_Per_Barrel();
    m_suspendFXDelay = that.Get_Suspend_FX_Delay();
}

Weapon &Weapon::operator=(const Weapon &that)
{
    if (this != &that) {
        m_template = that.m_template;
        m_wslot = that.m_wslot;
        m_status = OUT_OF_AMMO;
        m_ammoInClip = 0;
        m_whenPreAttackFinished = 0;
        m_whenLastReloadStarted = 0;
        m_whenWeCanFireAgain = 0;
        m_leechWeaponRangeActive = 0;
        m_pitchLimited =
            m_template->Get_Min_Target_Pitch() > -GAMEMATH_PI || m_template->Get_Max_Target_Pitch() < GAMEMATH_PI;
        m_maxShotCount = 0x7FFFFFFF;
        m_curBarrel = 0;
        m_lastFireFrame = 0;
        m_suspendFXDelay = that.Get_Suspend_FX_Delay();
        m_numShotsForCurBarrel = m_template->Get_Shots_Per_Barrel();
        m_projectileStreamID = OBJECT_UNK;
    }

    return *this;
}

void Weapon::CRC_Snapshot(Xfer *xfer)
{
#ifdef GAME_DEBUG_STRUCTS
    Utf8String str1;
    Utf8String str2;
    // todo
#endif

    Utf8String name = m_template->Get_Name();
    xfer->xferAsciiString(&name);
    xfer->xferUser(&m_wslot, sizeof(m_wslot));

#ifdef GAME_DEBUG_STRUCTS
    // todo
#endif

    xfer->xferUnsignedInt(&m_ammoInClip);

#ifdef GAME_DEBUG_STRUCTS
    // todo
#endif

    xfer->xferUnsignedInt(&m_whenWeCanFireAgain);

#ifdef GAME_DEBUG_STRUCTS
    // todo
#endif

    xfer->xferUnsignedInt(&m_whenPreAttackFinished);

#ifdef GAME_DEBUG_STRUCTS
    // todo
#endif

    xfer->xferUnsignedInt(&m_whenLastReloadStarted);

#ifdef GAME_DEBUG_STRUCTS
    // todo
#endif

    xfer->xferUnsignedInt(&m_lastFireFrame);

#ifdef GAME_DEBUG_STRUCTS
    // todo
#endif

    xfer->xferObjectID(&m_projectileStreamID);

#ifdef GAME_DEBUG_STRUCTS
    // todo
#endif

    ObjectID laser = OBJECT_UNK;
    xfer->xferObjectID(&laser);

#ifdef GAME_DEBUG_STRUCTS
    // todo
#endif

    xfer->xferInt(&m_maxShotCount);

#ifdef GAME_DEBUG_STRUCTS
    // todo
#endif

    xfer->xferInt(&m_curBarrel);

#ifdef GAME_DEBUG_STRUCTS
    // todo
#endif

    xfer->xferInt(&m_numShotsForCurBarrel);

#ifdef GAME_DEBUG_STRUCTS
    // todo
#endif

    unsigned short scatter_count = static_cast<unsigned short>(m_scatterTargetIndexes.size());
    xfer->xferUnsignedShort(&scatter_count);

#ifdef GAME_DEBUG_STRUCTS
    // todo
#endif

    for (auto i = m_scatterTargetIndexes.begin(); i != m_scatterTargetIndexes.end(); i++) {
        int index = *i;
        xfer->xferInt(&index);

#ifdef GAME_DEBUG_STRUCTS
        // todo
#endif
    }

    xfer->xferBool(&m_pitchLimited);

#ifdef GAME_DEBUG_STRUCTS
    // todo
#endif

    xfer->xferBool(&m_leechWeaponRangeActive);

#ifdef GAME_DEBUG_STRUCTS
    // todo
#endif
}

void Weapon::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 3;
    xfer->xferVersion(&version, 3);

    if (version >= 2) {
        Utf8String name;
        name = m_template->Get_Name();
        xfer->xferAsciiString(&name);

        if (xfer->Get_Mode() == XFER_LOAD) {
            m_template = g_theWeaponStore->Find_Weapon_Template(name);

            if (m_template == nullptr) {
                throw CODE_06;
            }
        }
    }

    xfer->xferUser(&m_wslot, sizeof(m_wslot));
    xfer->xferUser(&m_status, sizeof(m_status));
    xfer->xferUnsignedInt(&m_ammoInClip);
    xfer->xferUnsignedInt(&m_whenWeCanFireAgain);
    xfer->xferUnsignedInt(&m_whenPreAttackFinished);
    xfer->xferUnsignedInt(&m_whenLastReloadStarted);
    xfer->xferUnsignedInt(&m_lastFireFrame);

    if (version < 3) {
        m_suspendFXDelay = 0;
    } else {
        xfer->xferUnsignedInt(&m_suspendFXDelay);
    }

    xfer->xferObjectID(&m_projectileStreamID);
    ObjectID laser = OBJECT_UNK;
    xfer->xferObjectID(&laser);
    xfer->xferInt(&m_maxShotCount);
    xfer->xferInt(&m_curBarrel);
    xfer->xferInt(&m_numShotsForCurBarrel);

    unsigned short size = static_cast<unsigned short>(m_scatterTargetIndexes.size());
    xfer->xferUnsignedShort(&size);

    if (xfer->Get_Mode() == XFER_SAVE) {
        for (auto i = m_scatterTargetIndexes.begin(); i != m_scatterTargetIndexes.end(); i++) {
            int index = *i;
            xfer->xferInt(&index);
        }
    } else {
        m_scatterTargetIndexes.clear();
        for (unsigned short i = 0; i < size; i++) {
            int index;
            xfer->xferInt(&index);
            m_scatterTargetIndexes.push_back(index);
        }
    }

    xfer->xferBool(&m_pitchLimited);
    xfer->xferBool(&m_leechWeaponRangeActive);
}

void Weapon::Load_Post_Process()
{
    if (m_projectileStreamID != OBJECT_UNK) {
        if (g_theGameLogic->Find_Object_By_ID(m_projectileStreamID) == nullptr) {
            m_projectileStreamID = OBJECT_UNK;
        }
    }
}

bool Weapon::Is_Within_Attack_Range(const Object *source, const Object *target) const
{
#ifdef GAME_DLL
    return Call_Method<bool, const Weapon, const Object *, const Object *>(
        PICK_ADDRESS(0x004C72A0, 0x006D7E7F), this, source, target);
#else
    return false;
#endif
}

bool Weapon::Is_Within_Attack_Range(const Object *source, const Coord3D *target) const
{
#ifdef GAME_DLL
    return Call_Method<bool, const Weapon, const Object *, const Coord3D *>(
        PICK_ADDRESS(0x004C7170, 0x006D7DF0), this, source, target);
#else
    return false;
#endif
}

float Weapon::Get_Attack_Range(const Object *source) const
{
#ifdef GAME_DLL
    return Call_Method<float, const Weapon, const Object *>(PICK_ADDRESS(0x004C77A0, 0x006D843A), this, source);
#else
    return 0;
#endif
}
