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
#include "aiupdate.h"
#include "behaviormodule.h"
#include "damage.h"
#include "drawable.h"
#include "fxlist.h"
#include "gamelogic.h"
#include "globaldata.h"
#include "ini.h"
#include "object.h"
#include "objectcreationlist.h"
#include "opencontain.h"
#include "parkingplacebehavior.h"
#include "particlesystemplate.h"
#include "partitionmanager.h"
#include "player.h"
#include "simpleobjectiterator.h"
#include "specialpowercompletiondie.h"
#include "terrainlogic.h"
#include "thingfactory.h"
#include "weaponset.h"

BitFlags<OBJECT_STATUS_COUNT> OBJECT_STATUS_MASK_NONE;

void Do_FX_Pos(FXList const *list,
    const Coord3D *primary,
    const Matrix3D *primary_mtx,
    float primary_speed,
    const Coord3D *secondary,
    float radius);

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

void WeaponBonusSet::Parse_Weapon_Bonus_Set_Ptr(INI *ini, void *formal, void *store, const void *user_data)
{
    WeaponBonusSet *wbs = *static_cast<WeaponBonusSet **>(store);
    wbs->Parse_Weapon_Bonus_Set(ini);
}

void WeaponBonusSet::Parse_Weapon_Bonus_Set(INI *ini, void *formal, void *store, const void *user_data)
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

void WeaponStore::Create_And_Fire_Temp_Weapon(
    const WeaponTemplate *tmplate, const Object *source_obj, const Coord3D *victim_pos)
{
#ifdef GAME_DLL
    Call_Method<void, WeaponStore, const WeaponTemplate *, const Object *, const Coord3D *>(
        PICK_ADDRESS(0x004C50A0, 0x006D6661), this, tmplate, source_obj, victim_pos);
#endif
}

void WeaponStore::Set_Delayed_Damage(const WeaponTemplate *weapon,
    const Coord3D *pos,
    unsigned int which_frame,
    ObjectID source_id,
    ObjectID victim_id,
    const WeaponBonus &bonus)
{
    WeaponDelayedDamageInfo info;
    info.m_delayedWeapon = weapon;
    info.m_delayDamagePos = *pos;
    info.m_delayDamageFrame = which_frame;
    info.m_delaySourceID = source_id;
    info.m_delayIntendedVictimID = victim_id;
    info.m_bonus = bonus;
    m_weaponDDI.push_back(info);
}

void WeaponStore::Parse_Weapon_Template(INI *ini, void *formal, void *store, const void *user_data)
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

bool Weapon::Is_Within_Attack_Range(const Object *source_obj, const Object *victim_obj) const
{
#ifdef GAME_DLL
    return Call_Method<bool, const Weapon, const Object *, const Object *>(
        PICK_ADDRESS(0x004C72A0, 0x006D7E7F), this, source_obj, victim_obj);
#else
    return false;
#endif
}

bool Weapon::Is_Within_Attack_Range(const Object *source_obj, const Coord3D *victim_obj) const
{
#ifdef GAME_DLL
    return Call_Method<bool, const Weapon, const Object *, const Coord3D *>(
        PICK_ADDRESS(0x004C7170, 0x006D7DF0), this, source_obj, victim_obj);
#else
    return false;
#endif
}

float Weapon::Get_Attack_Range(const Object *source_obj) const
{
#ifdef GAME_DLL
    return Call_Method<float, const Weapon, const Object *>(PICK_ADDRESS(0x004C77A0, 0x006D843A), this, source_obj);
#else
    return 0;
#endif
}

void Weapon::On_Weapon_Bonus_Change(const Object *source_obj)
{
#ifdef GAME_DLL
    Call_Method<void, Weapon, const Object *>(PICK_ADDRESS(0x004C68E0, 0x006D76F8), this, source_obj);
#endif
}

bool Weapon::Is_Within_Target_Pitch(const Object *source_obj, const Object *victim_obj) const
{
#ifdef GAME_DLL
    return Call_Method<bool, const Weapon, const Object *, const Object *>(
        PICK_ADDRESS(0x004C85F0, 0x006D91B7), this, source_obj, victim_obj);
#else
    return true;
#endif
}

void Weapon::Load_Ammo_Now(const Object *source_obj)
{
#ifdef GAME_DLL
    Call_Method<void, Weapon, const Object *>(PICK_ADDRESS(0x004C6170, 0x006D7405), this, source_obj);
#endif
}

bool Weapon::Is_Damage_Weapon() const
{
#ifdef GAME_DLL
    return Call_Method<bool, const Weapon>(PICK_ADDRESS(0x004C87F0, 0x006D931D), this);
#else
    return true;
#endif
}

void Weapon::Reload_Ammo(const Object *source_obj)
{
#ifdef GAME_DLL
    Call_Method<void, Weapon, const Object *>(PICK_ADDRESS(0x004C62A0, 0x006D7440), this, source_obj);
#endif
}

Weapon::WeaponStatus Weapon::Get_Status() const
{
#ifdef GAME_DLL
    return Call_Method<Weapon::WeaponStatus, const Weapon>(PICK_ADDRESS(0x004C85B0, 0x006D915A), this);
#else
    return READY_TO_FIRE;
#endif
}

float Weapon::Get_Percent_Ready_To_Fire() const
{
#ifdef GAME_DLL
    return Call_Method<float, const Weapon>(PICK_ADDRESS(0x004C76C0, 0x006D8259), this);
#else
    return 0.0f;
#endif
}

float Weapon::Estimate_Weapon_Damage(const Object *source_obj, const Object *victim_obj, const Coord3D *victim_pos)
{
#ifdef GAME_DLL
    return Call_Method<float, Weapon, const Object *, const Object *, const Coord3D *>(
        PICK_ADDRESS(0x004C7970, 0x006D84C3), this, source_obj, victim_obj, victim_pos);
#else
    return 0.0f;
#endif
}

bool Weapon::Is_Source_Object_With_Goal_Position_Within_Attack_Range(
    const Object *source_obj, const Coord3D *goal_pos, const Object *target_obj, const Coord3D *target_pos) const
{
#ifdef GAME_DLL
    return Call_Method<bool, const Weapon, const Object *, const Coord3D *, const Object *, const Coord3D *>(
        PICK_ADDRESS(0x004C7010, 0x006D7D2B), this, source_obj, goal_pos, target_obj, target_pos);
#else
    return false;
#endif
}

void Weapon::New_Projectile_Fired(
    const Object *source_obj, const Object *projectile_obj, const Object *target_obj, const Coord3D *target_pos)
{
#ifdef GAME_DLL
    Call_Method<void, Weapon, const Object *, const Object *, const Object *, const Coord3D *>(
        PICK_ADDRESS(0x004C7B00, 0x006D8541), this, source_obj, projectile_obj, target_obj, target_pos);
#endif
}

void Weapon::Create_Laser(const Object *source_obj, const Object *victim_obj, const Coord3D *victim_pos)
{
#ifdef GAME_DLL
    Call_Method<void, Weapon, const Object *, const Object *, const Coord3D *>(
        PICK_ADDRESS(0x004C7C70, 0x006D8700), this, source_obj, victim_obj, victim_pos);
#endif
}

const char *s_theDeathTypeNames[] = {
    "NORMAL",
    "NONE",
    "CRUSHED",
    "BURNED",
    "EXPLODED",
    "POISONED",
    "TOPPLED",
    "FLOODED",
    "SUICIDED",
    "LASERED",
    "DETONATED",
    "SPLATTED,",
    "POISONED_BETA",
    "EXTRA_2",
    "EXTRA_3",
    "EXTRA_4",
    "EXTRA_5",
    "EXTRA_6",
    "EXTRA_7",
    "EXTRA_8",
    "POISONED_GAMMA",
    nullptr,
};

const char *s_theAutoReloadsClipNames[] = {
    "YES",
    "NO",
    "RETURN_TO_BASE",
    nullptr,
};

const char *s_theAttackTypeNames[] = {
    "PER_SHOT",
    "PER_ATTACK",
    "PER_CLIP",
    nullptr,
};

const char *s_theRadiusDamageAffectsNames[] = {
    "SELF",
    "ALLIES",
    "ENEMIES",
    "NEUTRALS",
    "SUICIDE",
    "NOT_SIMILAR",
    "NOT_AIRBORNE",
    nullptr,
};

const char *s_theProjectileCollidesWithNames[] = {
    "ALLIES",
    "ENEMIES",
    "STRUCTURES",
    "SHRUBBERY",
    "PROJECTILES",
    "WALLS",
    "SMALL_MISSILES",
    "BALLISTIC_MISSILES",
    "CONTROLLED_STRUCTURES",
    nullptr,
};

void Parse_All_Vet_Levels_ASCII_String(INI *ini, void *formal, void *store, const void *user_data)
{
    Utf8String OCL_str = ini->Get_Next_Ascii_String();
    Utf8String *OCL_vet_array = static_cast<Utf8String *>(store);

    for (auto i = 0; i < VETERANCY_COUNT; ++i) {
        OCL_vet_array[i] = OCL_str;
    }
}

void Parse_Per_Vet_Level_ASCII_String(INI *ini, void *formal, void *store, const void *user_data)
{
    int vet_level = ini->Scan_IndexList(ini->Get_Next_Token(), g_veterancyNames);
    Utf8String *OCL_vet_array = static_cast<Utf8String *>(store);
    OCL_vet_array[vet_level] = ini->Get_Next_Ascii_String();
}

void Parse_All_Vet_Levels_FXList(INI *ini, void *formal, void *store, const void *user_data)
{
    FXList **lists = static_cast<FXList **>(store);
    FXList *list = nullptr;
    FXList::Parse(ini, nullptr, &list, nullptr);

    for (int i = 0; i < VETERANCY_COUNT; i++) {
        lists[i] = list;
    }
}

void Parse_Per_Vet_Level_FXList(INI *ini, void *formal, void *store, const void *user_data)
{
    FXList **lists = static_cast<FXList **>(store);
    int index = INI::Scan_IndexList(ini->Get_Next_Token(), g_veterancyNames);
    FXList *list = nullptr;
    FXList::Parse(ini, nullptr, &list, nullptr);
    lists[index] = list;
}

void Parse_All_Vet_Levels_Particle_System(INI *ini, void *formal, void *store, const void *user_data)
{
    ParticleSystemTemplate *particle_template = nullptr;
    ParticleSystemTemplate::Parse(ini, nullptr, &particle_template, nullptr);
    ParticleSystemTemplate **particle_syst_vet_array = static_cast<ParticleSystemTemplate **>(store);

    for (int i = 0; i < VETERANCY_COUNT; ++i) {
        particle_syst_vet_array[i] = particle_template;
    }
}

void Parse_Per_Vet_Level_Particle_System(INI *ini, void *formal, void *store, const void *user_data)
{
    int vet_level = ini->Scan_IndexList(ini->Get_Next_Token(), g_veterancyNames);
    ParticleSystemTemplate **particle_syst_vet_array = static_cast<ParticleSystemTemplate **>(store);
    ParticleSystemTemplate *particle_template = nullptr;
    ParticleSystemTemplate::Parse(ini, nullptr, &particle_template, nullptr);
    particle_syst_vet_array[vet_level] = particle_template;
}

// clang-format off
FieldParse WeaponTemplate::s_fieldParseTable[] = {
     { "ProjectileStreamName", &INI::Parse_AsciiString, nullptr, offsetof(WeaponTemplate, m_projectileStreamName) },
     { "LaserName", &INI::Parse_AsciiString, nullptr, offsetof(WeaponTemplate, m_laserName) },
     { "LaserBoneName", &INI::Parse_AsciiString, nullptr, offsetof(WeaponTemplate, m_laserBoneName) },
     { "PrimaryDamage", &INI::Parse_Real, nullptr, offsetof(WeaponTemplate, m_primaryDamage) },
     { "PrimaryDamageRadius", &INI::Parse_Real, nullptr, offsetof(WeaponTemplate, m_primaryDamageRadius) },
     { "SecondaryDamage", &INI::Parse_Real, nullptr, offsetof(WeaponTemplate, m_secondaryDamage) },
     { "SecondaryDamageRadius", &INI::Parse_Real, nullptr, offsetof(WeaponTemplate, m_secondaryDamageRadius) },
     { "ShockWaveAmount", &INI::Parse_Real, nullptr, offsetof(WeaponTemplate, m_shockWaveAmount) },
     { "ShockWaveRadius", &INI::Parse_Real, nullptr, offsetof(WeaponTemplate, m_shockWaveRadius) },
     { "ShockWaveTaperOff", &INI::Parse_Real, nullptr, offsetof(WeaponTemplate, m_shockWaveTaperOff) },
     { "AttackRange", &INI::Parse_Real, nullptr, offsetof(WeaponTemplate, m_attackRange) },
     { "MinimumAttackRange", &INI::Parse_Real, nullptr, offsetof(WeaponTemplate, m_minimumAttackRange) },
     { "RequestAssistRange", &INI::Parse_Real, nullptr, offsetof(WeaponTemplate, m_requestAssistRange) },
     { "AcceptableAimDelta", &INI::Parse_Angle_Real, nullptr, offsetof(WeaponTemplate, m_aimDelta) },
     { "ScatterRadius", &INI::Parse_Real, nullptr, offsetof(WeaponTemplate, m_scatterRadius) },
     { "ScatterTargetScalar", &INI::Parse_Real, nullptr, offsetof(WeaponTemplate, m_scatterTargetScalar) },
     { "ScatterTarget", &WeaponTemplate::Parse_Scatter_Target, nullptr, offsetof(WeaponTemplate, m_scatterTarget) },
     { "DamageType", &BitFlags<DAMAGE_NUM_TYPES>::Parse_Single_Bit_From_INI, nullptr, offsetof(WeaponTemplate, m_damageType) },
     { "DeathType", &INI::Parse_Index_List, s_theDeathTypeNames, offsetof(WeaponTemplate, m_deathType) },
     { "WeaponSpeed", &INI::Parse_Velocity_Real, nullptr, offsetof(WeaponTemplate, m_weaponSpeed) },
     { "MinWeaponSpeed", &INI::Parse_Velocity_Real, nullptr, offsetof(WeaponTemplate, m_minWeaponSpeed) },
     { "ScaleWeaponSpeed", &INI::Parse_Bool, nullptr, offsetof(WeaponTemplate, m_scaleWeaponSpeed) },
     { "WeaponRecoil", &INI::Parse_Angle_Real, nullptr, offsetof(WeaponTemplate, m_weaponRecoil) },
     { "MinTargetPitch", &INI::Parse_Angle_Real, nullptr, offsetof(WeaponTemplate, m_minTargetPitch) },
     { "MaxTargetPitch", &INI::Parse_Angle_Real, nullptr, offsetof(WeaponTemplate, m_maxTargetPitch) },
     { "RadiusDamageAngle", &INI::Parse_Angle_Real, nullptr, offsetof(WeaponTemplate, m_radiusDamageAngle) },
     { "ProjectileObject", &INI::Parse_AsciiString, nullptr, offsetof(WeaponTemplate, m_projectileObject) },
     { "FireOCL", &Parse_All_Vet_Levels_ASCII_String, nullptr, offsetof(WeaponTemplate, m_fireOCL) },
     { "VeterancyFireOCL", &Parse_Per_Vet_Level_ASCII_String, nullptr, offsetof(WeaponTemplate, m_fireOCL) },
     { "ProjectileDetonationOCL", &Parse_All_Vet_Levels_ASCII_String, nullptr, offsetof(WeaponTemplate, m_projectileDetonationOCL) },
     { "VeterancyProjectileDetonationOCL", &Parse_Per_Vet_Level_ASCII_String, nullptr, offsetof(WeaponTemplate, m_projectileDetonationOCL) },
     { "ProjectileExhaust", &Parse_All_Vet_Levels_Particle_System, nullptr, offsetof(WeaponTemplate, m_projectileExhaust) },
     { "VeterancyProjectileExhaust", &Parse_Per_Vet_Level_Particle_System, nullptr, offsetof(WeaponTemplate, m_projectileExhaust) },
     { "FireFX", &Parse_All_Vet_Levels_FXList, nullptr, offsetof(WeaponTemplate, m_fireFX) },
     { "VeterancyFireFX", &Parse_Per_Vet_Level_FXList, nullptr, offsetof(WeaponTemplate, m_fireFX) },
     { "ProjectileDetonationFX", &Parse_All_Vet_Levels_FXList, nullptr, offsetof(WeaponTemplate, m_projectileDetonateFX) },
     { "VeterancyProjectileDetonationFX", &Parse_Per_Vet_Level_FXList, nullptr, offsetof(WeaponTemplate, m_projectileDetonateFX) },
     { "FireSound", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(WeaponTemplate, m_fireSound) },
     { "FireSoundLoopTime", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(WeaponTemplate, m_fireSoundLoopTime) },
     { "WeaponBonus", &WeaponTemplate::Parse_Weapon_Bonus_Set, nullptr, 0 },
     { "ClipSize", &INI::Parse_Int, nullptr, offsetof(WeaponTemplate, m_clipSize) },
     { "ClipReloadTime", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(WeaponTemplate, m_clipReloadTime) },
     { "DelayBetweenShots", &WeaponTemplate::Parse_Shot_Delay, nullptr, 0 },
     { "ContinuousFireOne", &INI::Parse_Int, nullptr, offsetof(WeaponTemplate, m_continuousFireOneShotsNeeded) },
     { "ContinuousFireTwo", &INI::Parse_Int, nullptr, offsetof(WeaponTemplate, m_continuousFireTwoShotsNeeded) },
     { "ContinuousFireCoast", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(WeaponTemplate, m_continuousFireCoastFrames) },
     { "AutoReloadWhenIdle", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(WeaponTemplate, m_autoReloadWhenIdle) },
     { "ShotsPerBarrel", &INI::Parse_Int, nullptr, offsetof(WeaponTemplate, m_shotsPerBarrel) },
     { "AntiAirborneVehicle", &INI::Parse_Bit_In_Int32, reinterpret_cast<void*>(ANTI_AIRBORNE_VEHICLE), offsetof(WeaponTemplate, m_antiMask) },
     { "AntiGround", &INI::Parse_Bit_In_Int32, reinterpret_cast<void*>(ANTI_GROUND), offsetof(WeaponTemplate, m_antiMask) },
     { "AntiProjectile", &INI::Parse_Bit_In_Int32, reinterpret_cast<void*>(ANTI_PROJECTILE), offsetof(WeaponTemplate, m_antiMask) },
     { "AntiSmallMissile", &INI::Parse_Bit_In_Int32, reinterpret_cast<void*>(ANTI_SMALL_MISSILE), offsetof(WeaponTemplate, m_antiMask) },
     { "AntiMine", &INI::Parse_Bit_In_Int32, reinterpret_cast<void*>(ANTI_MINE), offsetof(WeaponTemplate, m_antiMask) },
     { "AntiAirborneInfantry", &INI::Parse_Bit_In_Int32, reinterpret_cast<void*>(ANTI_AIRBORNE_INFANTRY), offsetof(WeaponTemplate, m_antiMask) },
     { "AntiBallisticMissile", &INI::Parse_Bit_In_Int32, reinterpret_cast<void*>(ANTI_BALLISTIC_MISSILE), offsetof(WeaponTemplate, m_antiMask) },
     { "AntiParachute", &INI::Parse_Bit_In_Int32, reinterpret_cast<void*>(ANTI_PARACHUTE), offsetof(WeaponTemplate, m_antiMask) },
     { "RadiusDamageAffects", &INI::Parse_Bitstring32, s_theRadiusDamageAffectsNames, offsetof(WeaponTemplate, m_affectsMask) },
     { "ProjectileCollidesWith", &INI::Parse_Bitstring32, s_theProjectileCollidesWithNames, offsetof(WeaponTemplate, m_collideMask) },
     { "DamageDealtAtSelfPosition", &INI::Parse_Bool, nullptr, offsetof(WeaponTemplate, m_damageDealtAtSelfPosition) },
     { "AutoReloadsClip", &INI::Parse_Index_List, s_theAutoReloadsClipNames, offsetof(WeaponTemplate, m_autoReloadsClip) },
     { "PreAttackType", &INI::Parse_Index_List, s_theAttackTypeNames, offsetof(WeaponTemplate, m_preAttackType) },
     { "HistoricBonusTime", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(WeaponTemplate, m_historicBonusTime) },
     { "HistoricBonusRadius", &INI::Parse_Real, nullptr, offsetof(WeaponTemplate, m_historicBonusRadius) },
     { "HistoricBonusCount", &INI::Parse_Int, nullptr, offsetof(WeaponTemplate, m_historicBonusCount) },
     { "HistoricBonusWeapon", &WeaponStore::Parse_Weapon_Template, nullptr, offsetof(WeaponTemplate, m_historicBonusWeapon) },
     { "LeechRangeWeapon", &INI::Parse_Bool, nullptr, offsetof(WeaponTemplate, m_leechRangeWeapon) },
     { "CapableOfFollowingWaypoints", &INI::Parse_Bool, nullptr, offsetof(WeaponTemplate, m_capableOfFollowingWaypoint) },
     { "ShowsAmmoPips", &INI::Parse_Bool, nullptr, offsetof(WeaponTemplate, m_showAmmoPips) },
     { "AllowAttackGarrisonedBldgs", &INI::Parse_Bool, nullptr, offsetof(WeaponTemplate, m_allowAttackGarrisonedBldgs) },
     { "PlayFXWhenStealthed", &INI::Parse_Bool, nullptr, offsetof(WeaponTemplate, m_playFXWhenStealthed) },
     { "PreAttackDelay", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(WeaponTemplate, m_preAttackDelay) },
     { "ContinueAttackRange", &INI::Parse_Real, nullptr, offsetof(WeaponTemplate, m_continueAttackRange) },
     { "ScatterRadiusVsInfantry", &INI::Parse_Real, nullptr, offsetof(WeaponTemplate, m_scatterRadiusVsInfantry) },
     { "DamageStatusType", &BitFlags<OBJECT_STATUS_COUNT>::Parse_Single_Bit_From_INI, nullptr, offsetof(WeaponTemplate, m_damageStatusType) },
     { "SuspendFXDelay", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(WeaponTemplate, m_suspendFXDelay) },
     { "MissileCallsOnDie", &INI::Parse_Bool, nullptr, offsetof(WeaponTemplate, m_missileCallsOnDie) },
     { nullptr, nullptr, nullptr, 0 },
};
// clang-format on

WeaponTemplate::WeaponTemplate() :
    m_nextTemplate(nullptr),
    m_name("NoNameWeapon"),
    m_nameKey(NAMEKEY_INVALID),
    m_primaryDamage(0.0f),
    m_primaryDamageRadius(0.0f),
    m_secondaryDamage(0.0f),
    m_secondaryDamageRadius(0.0f),
    m_shockWaveAmount(0.0f),
    m_shockWaveRadius(0.0f),
    m_shockWaveTaperOff(0.0f),
    m_attackRange(0.0f),
    m_minimumAttackRange(0.0f),
    m_requestAssistRange(0.0f),
    m_aimDelta(0.0f),
    m_scatterRadius(0.0f),
    m_scatterTargetScalar(0.0f),
    m_damageType(DAMAGE_EXPLOSION),
    m_deathType(DEATH_NORMAL),
    m_weaponSpeed(999999.0f),
    m_minWeaponSpeed(999999.0f),
    m_scaleWeaponSpeed(false),
    m_weaponRecoil(0.0f),
    m_minTargetPitch(DEG_TO_RADF(-180.0f)),
    m_maxTargetPitch(DEG_TO_RADF(180.0f)),
    m_radiusDamageAngle(DEG_TO_RADF(180.0f)),
    m_projectileTemplate(nullptr),
    m_fireSoundLoopTime(0),
    m_extraBonus(nullptr),
    m_clipSize(0),
    m_clipReloadTime(0),
    m_minDelayBetweenShots(0),
    m_maxDelayBetweenShots(0),
    m_continuousFireOneShotsNeeded(0x7FFFFFFF),
    m_continuousFireTwoShotsNeeded(0x7FFFFFFF),
    m_continuousFireCoastFrames(0),
    m_autoReloadWhenIdle(0),
    m_shotsPerBarrel(1),
    m_antiMask(ANTI_GROUND),
    m_affectsMask(AFFECTS_NEUTRALS | AFFECTS_ENEMIES | AFFECTS_ALLIES),
    m_collideMask(COLLIDE_STRUCTURES),
    m_damageDealtAtSelfPosition(false),
    m_autoReloadsClip(AUTO_RELOADS_CLIP_YES),
    m_preAttackType(ATTACK_TYPE_PER_SHOT),
    m_historicBonusTime(0),
    m_historicBonusRadius(0.0f),
    m_historicBonusCount(0),
    m_historicBonusWeapon(nullptr),
    m_leechRangeWeapon(false),
    m_capableOfFollowingWaypoint(false),
    m_showAmmoPips(false),
    m_allowAttackGarrisonedBldgs(false),
    m_playFXWhenStealthed(false),
    m_preAttackDelay(0),
    m_continueAttackRange(0.0f),
    m_scatterRadiusVsInfantry(0.0f),
    m_damageStatusType(OBJECT_STATUS_NONE),
    m_suspendFXDelay(0),
    m_missileCallsOnDie(false)
{
    m_projectileObject.Clear();

    for (int i = 0; i < VETERANCY_COUNT; i++) {
        m_fireOCLName[i].Clear();
        m_projectileDetonationOCLName[i].Clear();
        m_projectileExhaust[i] = nullptr;
        m_fireOCL[i] = nullptr;
        m_projectileDetonationOCL[i] = nullptr;
        m_fireFX[i] = nullptr;
        m_projectileDetonateFX[i] = nullptr;
    }

    m_projectileStreamName.Clear();
    m_laserName.Clear();
    m_laserBoneName.Clear();
}

WeaponTemplate::~WeaponTemplate()
{
    if (m_nextTemplate != nullptr) {
        m_nextTemplate->Delete_Instance();
    }

    if (m_extraBonus != nullptr) {
        m_extraBonus->Delete_Instance();
    }
}

void WeaponTemplate::Reset()
{
    m_historicDamage.clear();
}

void WeaponTemplate::Parse_Weapon_Bonus_Set(INI *ini, void *formal, void *store, const void *user_data)
{
    WeaponTemplate *tmplate = static_cast<WeaponTemplate *>(formal);

    if (tmplate->m_extraBonus == nullptr) {
        tmplate->m_extraBonus = new WeaponBonusSet();
    }

    tmplate->m_extraBonus->Parse_Weapon_Bonus_Set(ini);
}

void WeaponTemplate::Parse_Scatter_Target(INI *ini, void *formal, void *store, const void *user_data)
{
    WeaponTemplate *tmplate = static_cast<WeaponTemplate *>(formal);
    Coord2D target;
    INI::Parse_Coord2D(ini, nullptr, &target, nullptr);
    tmplate->m_scatterTarget.push_back(target);
}

const float _DURATION_MULT = 0.03f;

void WeaponTemplate::Parse_Shot_Delay(INI *ini, void *formal, void *store, const void *user_data)
{
    WeaponTemplate *tmplate = static_cast<WeaponTemplate *>(formal);
    const char *str = ini->Get_Next_Token_Or_Null(ini->Get_Seps_Colon());

    if (strcasecmp(str, "Min") == 0) {
        tmplate->m_minDelayBetweenShots = INI::Scan_Int(ini->Get_Next_Token(ini->Get_Seps_Colon()));
        const char *str2 = ini->Get_Next_Token_Or_Null(ini->Get_Seps_Colon());

        if (strcasecmp(str2, "Max") == 0) {
            tmplate->m_maxDelayBetweenShots = INI::Scan_Int(ini->Get_Next_Token(ini->Get_Seps_Colon()));
        } else {
            tmplate->m_maxDelayBetweenShots = tmplate->m_minDelayBetweenShots;
        }
    } else {
        tmplate->m_minDelayBetweenShots = INI::Scan_Int(str);
        tmplate->m_maxDelayBetweenShots = tmplate->m_minDelayBetweenShots;
    }

    tmplate->m_minDelayBetweenShots = GameMath::Ceil(tmplate->m_minDelayBetweenShots * _DURATION_MULT);
    tmplate->m_maxDelayBetweenShots = GameMath::Ceil(tmplate->m_maxDelayBetweenShots * _DURATION_MULT);
}

void WeaponTemplate::Post_Process_Load()
{
    if (g_theThingFactory != nullptr) {
        if (m_projectileObject.Is_Empty()) {
            m_projectileTemplate = nullptr;
        } else {
            m_projectileTemplate = g_theThingFactory->Find_Template(m_projectileObject, true);
            captainslog_dbgassert(m_projectileTemplate != nullptr, "projectile %s not found!", m_projectileObject.Str());
        }

        for (int i = 0; i < VETERANCY_COUNT; i++) {
            if (m_fireOCLName[i].Is_Empty()) {
                m_fireOCL[i] = nullptr;
            } else {
                m_fireOCL[i] = g_theObjectCreationListStore->Find_Object_Creation_List(m_fireOCLName[i].Str());
                captainslog_dbgassert(m_fireOCL[i] != nullptr, "OCL %s not found in a weapon!", m_fireOCLName[i].Str());
            }

            if (m_projectileDetonationOCLName[i].Is_Empty()) {
                m_projectileDetonationOCL[i] = nullptr;
            } else {
                m_projectileDetonationOCL[i] =
                    g_theObjectCreationListStore->Find_Object_Creation_List(m_projectileDetonationOCLName[i].Str());
                captainslog_dbgassert(m_projectileDetonationOCL[i] != nullptr,
                    "OCL %s not found in a weapon!",
                    m_projectileDetonationOCLName[i].Str());
            }
        }
    } else {
        captainslog_dbgassert(false, "you must call this after TheThingFactory is inited");
    }
}

float WeaponTemplate::Get_Attack_Range(const WeaponBonus &bonus) const
{
    float range = bonus.Get_Field(WeaponBonus::RANGE) * m_attackRange - 2.5f;

    if (range < 0.0f) {
        return 0.0f;
    }

    return range;
}

float WeaponTemplate::Get_Minimum_Attack_Range() const
{
    float range = m_minimumAttackRange - 2.5f;

    if (range < 0.0f) {
        return 0.0f;
    }

    return range;
}

float WeaponTemplate::Get_Unmodified_Attack_Range() const
{
    return m_attackRange;
}

int WeaponTemplate::Get_Delay_Between_Shots(const WeaponBonus &bonus) const
{
    float delay;

    if (m_minDelayBetweenShots == m_maxDelayBetweenShots) {
        delay = m_minDelayBetweenShots;
    } else {
        delay = Get_Logic_Random_Value(m_minDelayBetweenShots, m_maxDelayBetweenShots);
    }

    return GameMath::Fast_To_Int_Floor(delay / bonus.Get_Field(WeaponBonus::RATE_OF_FIRE));
}

int WeaponTemplate::Get_Clip_Reload_Time(const WeaponBonus &bonus) const
{
    return GameMath::Fast_To_Int_Floor(m_clipReloadTime / bonus.Get_Field(WeaponBonus::RATE_OF_FIRE));
}

int WeaponTemplate::Get_Pre_Attack_Delay(const WeaponBonus &bonus) const
{
    return bonus.Get_Field(WeaponBonus::PRE_ATTACK) * m_preAttackDelay;
}

float WeaponTemplate::Get_Primary_Damage(const WeaponBonus &bonus) const
{
    return bonus.Get_Field(WeaponBonus::DAMAGE) * m_primaryDamage;
}

float WeaponTemplate::Get_Primary_Damage_Radius(const WeaponBonus &bonus) const
{
    return bonus.Get_Field(WeaponBonus::RADIUS) * m_primaryDamageRadius;
}

float WeaponTemplate::Get_Secondary_Damage(const WeaponBonus &bonus) const
{
    return bonus.Get_Field(WeaponBonus::DAMAGE) * m_secondaryDamage;
}

float WeaponTemplate::Get_Secondary_Damage_Radius(const WeaponBonus &bonus) const
{
    return bonus.Get_Field(WeaponBonus::RADIUS) * m_secondaryDamageRadius;
}

bool WeaponTemplate::Is_Contact_Weapon() const
{
    return m_attackRange - 2.5f < 10.0f;
}

void WeaponTemplate::Trim_Old_Historic_Damage() const
{
    unsigned int frame = g_theGameLogic->Get_Frame() - g_theWriteableGlobalData->m_historicDamageLimit;

    while (m_historicDamage.size() != 0 && m_historicDamage.front().frame <= frame) {
        m_historicDamage.pop_front();
    }
}

bool WeaponTemplate::Should_Projectile_Collide_With(
    const Object *source_obj, const Object *projectile_obj, const Object *collide_obj, ObjectID id) const
{
    if (projectile_obj == nullptr || collide_obj == nullptr) {
        return false;
    }

    if (id == collide_obj->Get_ID()) {
        return true;
    }

    if (source_obj != nullptr) {
        if (source_obj == collide_obj) {
            return false;
        }

        if (source_obj->Get_Contained_By() == collide_obj) {
            return false;
        }
    }

    if ((Get_Damage_Type() == DAMAGE_FLAME || Get_Damage_Type() == DAMAGE_PARTICLE_BEAM)
        && collide_obj->Get_Status(OBJECT_STATUS_BURNED)) {
        return false;
    }

    if (collide_obj->Is_KindOf(KINDOF_FS_AIRFIELD)) {
        for (BehaviorModule **i = collide_obj->Get_All_Modules(); *i != nullptr; i++) {
            ParkingPlaceBehaviorInterface *park = (*i)->Get_Parking_Place_Behavior_Interface();

            if (park != nullptr && park->Has_Reserved_Space(id)) {
                return false;
            }
        }
    }

    const AIUpdateInterface *update = collide_obj->Get_AI_Update_Interface();

    if (update != nullptr && update->Get_Sneaky_Targeting_Offset(nullptr)) {
        return false;
    }

    int mask = 0;
    Relationship r = projectile_obj->Get_Relationship(collide_obj);

    if (r == ALLIES) {
        mask = COLLIDE_ALLIES;
    } else if (r == ENEMIES) {
        mask = COLLIDE_ENEMIES;
    }

    if (collide_obj->Is_KindOf(KINDOF_STRUCTURE)) {
        if (collide_obj->Get_Controlling_Player() == projectile_obj->Get_Controlling_Player()) {
            mask |= COLLIDE_CONTROLLED_STRUCTURES;
        } else {
            mask |= COLLIDE_STRUCTURES;
        }
    }

    if (collide_obj->Is_KindOf(KINDOF_SHRUBBERY)) {
        mask |= COLLIDE_SHRUBBERY;
    }

    if (collide_obj->Is_KindOf(KINDOF_PROJECTILE)) {
        mask |= COLLIDE_PROJECTILES;
    }

    if (collide_obj->Get_Template()->Get_Fence_Width() > 0.0f) {
        mask |= COLLIDE_WALLS;
    }

    if (collide_obj->Is_KindOf(KINDOF_SMALL_MISSILE)) {
        mask |= COLLIDE_SMALL_MISSILES;
    }

    if (collide_obj->Is_KindOf(KINDOF_BALLISTIC_MISSILE)) {
        mask |= COLLIDE_BALLISTIC_MISSILES;
    }

    return (mask & Get_Collide_Mask()) != 0;
}

float WeaponTemplate::Estimate_Weapon_Template_Damage(
    const Object *source_obj, const Object *victim_obj, const Coord3D *victim_pos, const WeaponBonus &bonus) const
{
    if (source_obj == nullptr || (victim_obj == nullptr && victim_pos == nullptr)) {
        captainslog_dbgassert(false, "bad args to estimate");
        return 0.0f;
    }

    DamageType damage = Get_Damage_Type();
    DeathType death = Get_Death_Type();

    if (victim_obj != nullptr && victim_obj->Is_KindOf(KINDOF_SHRUBBERY)) {
        if (death == DEATH_BURNED) {
            return 1.0f;
        } else {
            return 0.0f;
        }
    }

    if (victim_obj->Is_KindOf(KINDOF_STRUCTURE) && damage == DAMAGE_SNIPER) {
        if (victim_obj->Get_Contain() != nullptr) {
            ContainModuleInterface *contain = victim_obj->Get_Contain();

            if (contain->Get_Contain_Count() == 0) {
                return 0.0f;
            }
        }
    }

    if (damage == DAMAGE_SURRENDER || m_allowAttackGarrisonedBldgs) {
        ContainModuleInterface *contain = victim_obj->Get_Contain();

        if (contain != nullptr) {
            if (contain->Get_Contain_Count() != 0 && contain->Is_Garrisonable()
                && !contain->Is_Immune_To_Clear_Building_Attacks()) {
                return 1.0f;
            }
        }
    }

    if (victim_obj != nullptr) {
        if (damage == DAMAGE_DISARM) {
            if (victim_obj->Is_KindOf(KINDOF_MINE) || victim_obj->Is_KindOf(KINDOF_BOOBY_TRAP)
                || victim_obj->Is_KindOf(KINDOF_DEMOTRAP)) {
                return 1.0f;
            } else {
                return 0.0f;
            }
        }

        if (damage == DAMAGE_DEPLOY && !victim_obj->Is_Airborne_Target()) {
            return 1.0f;
        }
    }

    if (victim_pos == nullptr) {
        victim_pos = victim_obj->Get_Position();
    }

    float primary = Get_Primary_Damage(bonus);

    if (victim_obj == nullptr) {
        return primary;
    }

    DamageInfoInput info;
    info.m_damageType = damage;
    info.m_deathType = death;
    info.m_sourceID = source_obj->Get_ID();
    info.m_amount = primary;
    return victim_obj->Estimate_Damage(info);
}

unsigned int WeaponTemplate::Fire_Weapon_Template(const Object *source_obj,
    WeaponSlotType wslot,
    int specific_barrel_to_use,
    Object *victim_obj,
    const Coord3D *victim_pos,
    const WeaponBonus &bonus,
    bool is_projectile_detonation,
    bool ignore_ranges,
    Weapon *firing_weapon,
    ObjectID *projectile_id,
    bool do_damage) const
{
    Utf8String str;

#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData->m_extraLogging) {
        if (victim_obj != nullptr) {
            str.Format("%s", victim_obj->Get_Template()->Get_Name().Str());
        } else if (victim_pos != nullptr) {
            str.Format("%d,%d,%d", (int)victim_pos->x, (int)victim_pos->y, (int)victim_pos->z);
        }

        captainslog_debug("%d - WeaponTemplate::fireWeaponTemplate() begin - %s attacking %s - ",
            (int)g_theGameLogic->Get_Frame(),
            source_obj->Get_Template()->Get_Name().Str(),
            str.Str());
    }
#endif

    captainslog_dbgassert(specific_barrel_to_use >= 0, "specific_barrel_to_use should no longer be -1");

    if (source_obj == nullptr || (victim_obj == nullptr && victim_pos == nullptr)) {
#ifdef GAME_DEBUG_STRUCTS
        if (g_theWriteableGlobalData->m_extraLogging) {
            captainslog_debug("FAIL 1 (sourceObj %d == nullptr || (victimObj %d == nullptr && victimPos %d == nullptr)",
                source_obj != nullptr,
                victim_obj != nullptr,
                victim_pos != nullptr);
        }
#endif
        return 0;
    }

    // assert disabled due to stock issue with ScorpionTankGunFXWeapon
    // captainslog_dbgassert(m_primaryDamage > 0.0f, "You can't really shoot a zero damage weapon at an Object.");
    ObjectID source_id = source_obj->Get_ID();
    const Coord3D *source_pos = source_obj->Get_Position();
    ObjectID victim_id;
    Coord3D new_pos;
    float dist_sqr;

    if (victim_obj != nullptr) {
        if (source_obj == victim_obj) {
            captainslog_debug("*** firing weapon at self -- is this really what you want?");
        }

        victim_pos = victim_obj->Get_Position();
        victim_id = victim_obj->Get_ID();
        const AIUpdateInterface *update = victim_obj->Get_AI_Update_Interface();
        Coord3D offset;

        if (update != nullptr && update->Get_Sneaky_Targeting_Offset(&offset)) {
            new_pos = *victim_pos;
            new_pos += offset;
            victim_pos = &new_pos;
            victim_obj = nullptr;
            victim_id = OBJECT_UNK;
            dist_sqr = g_thePartitionManager->Get_Distance_Squared(source_obj, &new_pos, FROM_BOUNDINGSPHERE_2D, nullptr);
        } else if (victim_obj->Is_KindOf(KINDOF_BRIDGE)) {
            TBridgeAttackInfo info;
            g_theTerrainLogic->Get_Bridge_Attack_Points(victim_obj, &info);
            dist_sqr = g_thePartitionManager->Get_Distance_Squared(
                source_obj, &info.m_attackPoint1, FROM_BOUNDINGSPHERE_2D, nullptr);
            victim_pos = &info.m_attackPoint1;
            float dist2_sqr = g_thePartitionManager->Get_Distance_Squared(
                source_obj, &info.m_attackPoint2, FROM_BOUNDINGSPHERE_2D, nullptr);

            if (dist_sqr > dist2_sqr) {
                dist_sqr = dist2_sqr;
                victim_pos = &info.m_attackPoint2;
            }
        } else {
            dist_sqr = g_thePartitionManager->Get_Distance_Squared(source_obj, victim_obj, FROM_BOUNDINGSPHERE_2D, nullptr);
        }
    } else {
        victim_id = OBJECT_UNK;
        dist_sqr = g_thePartitionManager->Get_Distance_Squared(source_obj, victim_pos, FROM_BOUNDINGSPHERE_2D, nullptr);
    }

    if (!ignore_ranges && !Is_Leech_Range_Weapon()) {
        float attack_range_sqr = GameMath::Square(Get_Attack_Range(bonus));

        if (dist_sqr > attack_range_sqr) {
#ifdef GAME_DEBUG_STRUCTS
            if (g_theWriteableGlobalData->m_extraLogging) {
                captainslog_debug("FAIL 2 (dist_sqr %.2f > attack_range_sqr %.2f)", dist_sqr, attack_range_sqr);
            }
#endif

            return 0;
        }
    }

    if (!ignore_ranges) {
        float min_attack_range_sqr = GameMath::Square(Get_Minimum_Attack_Range());

        if (dist_sqr < min_attack_range_sqr && !is_projectile_detonation) {
            captainslog_dbgassert(min_attack_range_sqr * 0.8f < dist_sqr,
                "*** victim is closer than min attack range (%f vs %f) of this weapon -- why did we attempt to fire?",
                GameMath::Sqrt(dist_sqr),
                GameMath::Sqrt(min_attack_range_sqr));
#ifdef GAME_DEBUG_STRUCTS
            if (g_theWriteableGlobalData->m_extraLogging) {
                captainslog_debug("FAIL 3 (dist_sqr %.2f< min_attack_range_sqr %.2f - 0.5f && !is_projectile_detonation %d)",
                    dist_sqr,
                    min_attack_range_sqr,
                    0);
            }
#endif

            return 0;
        }
    }

    Coord3D secondary_pos;

    if (source_obj->Get_Drawable()) {
        if (victim_obj != nullptr) {
            victim_obj->Get_Geometry_Info().Get_Center_Position(*victim_obj->Get_Position(), secondary_pos);
        } else {
            secondary_pos.Set(victim_pos);
        }

        float recoil_amount = Get_Weapon_Recoil();
        float recoil_angle;

        if (recoil_amount == 0.0f) {
            recoil_angle = 0.0f;
        } else {
            recoil_angle = GameMath::Atan2(victim_pos->y - source_pos->y, victim_pos->x - source_pos->x);
        }

        VeterancyLevel veterancy = source_obj->Get_Veterancy_Level();
        FXList *fx;

        if (is_projectile_detonation) {
            fx = Get_Projectile_Detonate_FX(veterancy);
        } else {
            fx = Get_Fire_FX(veterancy);
        }

        if (g_theGameLogic->Get_Frame() < firing_weapon->Get_Suspend_FX_Delay()) {
            fx = nullptr;
        }

        bool done;

        if (source_obj->Is_Locally_Controlled() || !source_obj->Get_Status(OBJECT_STATUS_STEALTHED)
            || source_obj->Get_Status(OBJECT_STATUS_DETECTED) || source_obj->Get_Status(OBJECT_STATUS_DISGUISED)
            || source_obj->Is_KindOf(KINDOF_MINE) || Is_Play_FX_When_Stealthed()) {
            done = source_obj->Get_Drawable()->Handle_Weapon_Fire_FX(wslot,
                specific_barrel_to_use,
                fx,
                Get_Weapon_Speed(),
                recoil_amount,
                recoil_angle,
                &secondary_pos,
                Get_Primary_Damage_Radius(bonus));
        } else {
            done = true;
        }

        if (!done && fx != nullptr) {
            const Coord3D *pos2;

            if (Is_Contact_Weapon()) {
                pos2 = &secondary_pos;
            } else {
                pos2 = source_obj->Get_Drawable()->Get_Position();
            }

            Do_FX_Pos(fx,
                pos2,
                source_obj->Get_Drawable()->Get_Transform_Matrix(),
                Get_Weapon_Speed(),
                &secondary_pos,
                Get_Primary_Damage_Radius(bonus));
        }
    }

    VeterancyLevel veterancy = source_obj->Get_Veterancy_Level();
    ObjectCreationList *ocl;

    if (is_projectile_detonation) {
        ocl = Get_Projectile_Detonation_OCL(veterancy);
    } else {
        ocl = Get_Fire_OCL(veterancy);
    }

    if (ocl != nullptr) {
        ocl->Create(source_obj, nullptr, 0);
    }

    Coord3D pos = *victim_pos;
    float scatter = 0.0f;

    if (m_scatterRadius > 0.0f
        || (m_scatterRadiusVsInfantry > 0.0f && victim_obj != nullptr && victim_obj->Is_KindOf(KINDOF_INFANTRY))) {
        scatter = m_scatterRadius;
        PathfindLayerEnum layer = LAYER_GROUND;

        if (victim_obj != nullptr) {
            if (victim_obj->Is_KindOf(KINDOF_STRUCTURE)) {
                victim_obj->Get_Geometry_Info().Get_Center_Position(*victim_obj->Get_Position(), pos);
            }

            if (m_scatterRadiusVsInfantry > 0.0f && victim_obj->Is_KindOf(KINDOF_INFANTRY)) {
                scatter += m_scatterRadiusVsInfantry;
            }

            layer = victim_obj->Get_Layer();
        }

        scatter = Get_Logic_Random_Value_Real(0.0f, scatter);
        float angle = Get_Logic_Random_Value_Real(0.0f, DEG_TO_RADF(360.0f));
        Coord3D pos3;
        pos3.Zero();
        pos3.x = GameMath::Cos(angle) * scatter;
        pos3.y = GameMath::Sin(angle) * scatter;
        pos.x += pos3.x;
        pos.y += pos3.y;
        pos.z = g_theTerrainLogic->Get_Layer_Height(pos.x, pos.y, layer, nullptr, true);
    }

    if (Get_Projectile_Template() != nullptr && !is_projectile_detonation) {
        Player *player = source_obj->Get_Controlling_Player();

        Object *projectile_obj =
            g_theThingFactory->New_Object(Get_Projectile_Template(), player->Get_Default_Team(), OBJECT_STATUS_MASK_NONE);
        projectile_obj->Set_Producer(source_obj);

        if (player->Get_Total_Battle_Plan_Count() > 0) {
            player->Apply_Battle_Plan_Bonuses_For_Object(projectile_obj);
        }

        if (projectile_id != nullptr) {
            *projectile_id = projectile_obj->Get_ID();
        }

        SpecialPowerCompletionDie *source_die = source_obj->Find_Special_Power_Completion_Die();

        if (source_die != nullptr) {
            source_die->Notify_Script_Engine();
            SpecialPowerCompletionDie *projectile_die = projectile_obj->Find_Special_Power_Completion_Die();

            if (projectile_die != nullptr) {
                projectile_die->Set_Creator(OBJECT_UNK);
            }
        } else {
            SpecialPowerCompletionDie *projectile_die = projectile_obj->Find_Special_Power_Completion_Die();

            if (projectile_die != nullptr) {
                projectile_die->Set_Creator(source_obj->Get_ID());
            }
        }

        firing_weapon->New_Projectile_Fired(source_obj, projectile_obj, victim_obj, victim_pos);
        ProjectileUpdateInterface *projectile = nullptr;

        for (BehaviorModule **i = projectile_obj->Get_All_Modules(); *i != nullptr; i++) {
            projectile = (*i)->Get_Projectile_Update_Interface();

            if (projectile != nullptr) {
                break;
            }
        }

        if (projectile != nullptr) {
            if (scatter <= 0.0f) {
                projectile->Projectile_Launch_At_Object_Or_Position(victim_obj,
                    &pos,
                    source_obj,
                    wslot,
                    specific_barrel_to_use,
                    this,
                    m_projectileExhaust[source_obj->Get_Veterancy_Level()]);
            } else {
                projectile->Projectile_Launch_At_Object_Or_Position(nullptr,
                    &pos,
                    source_obj,
                    wslot,
                    specific_barrel_to_use,
                    this,
                    m_projectileExhaust[source_obj->Get_Veterancy_Level()]);
            }
        } else {
            projectile_obj->Set_Position(&pos);
        }

        if (projectile_obj->Is_KindOf(KINDOF_SMALL_MISSILE)) {
            if (victim_obj != nullptr) {
                if (victim_obj->Has_Countermeasures()) {
                    const AIUpdateInterface *update = victim_obj->Get_AI_Update_Interface();

                    if (update != nullptr) {
                        if (update->Get_Cur_Locomotor_Set() != LOCOMOTORSET_SUPERSONIC) {
                            victim_obj->Report_Missile_For_Countermeasures(projectile_obj);
                        }
                    }
                }
            }
        }

#ifdef GAME_DEBUG_STRUCTS
        if (g_theWriteableGlobalData->m_extraLogging) {
            captainslog_debug("DONE");
        }
#endif

        return 0;
    } else {
        Coord3D firing_vector = *victim_pos - *source_pos;
        float firing_duration = firing_vector.Length() / Get_Weapon_Speed();
        ObjectID target_id = !Is_Damage_Dealt_At_Self_Position() ? victim_id : OBJECT_UNK;

        if (firing_weapon->Has_Laser()) {
            if (Get_Primary_Damage_Radius(bonus) < scatter && Get_Secondary_Damage_Radius(bonus) < scatter) {
                target_id = OBJECT_UNK;
                firing_weapon->Create_Laser(source_obj, nullptr, &pos);
            } else {
                if (victim_obj != nullptr) {
                    pos.Set(victim_obj->Get_Position());
                }

                firing_weapon->Create_Laser(source_obj, victim_obj, &pos);
            }

            if (do_damage) {
                Deal_Damage_Internal(source_id, target_id, &pos, bonus, is_projectile_detonation);
            }

            return g_theGameLogic->Get_Frame();
        } else {
            const Coord3D *pos3;
            if (Is_Damage_Dealt_At_Self_Position()) {
                pos3 = source_pos;
            } else {
                pos3 = victim_pos;
            }

            if (firing_duration > 1.0f) {
                unsigned int frame = 0;

                if (g_theWeaponStore && do_damage) {
                    frame = GameMath::Fast_To_Int_Ceil(firing_duration) + g_theGameLogic->Get_Frame();
                    g_theWeaponStore->Set_Delayed_Damage(this, pos3, frame, source_id, target_id, bonus);
                }

#ifdef GAME_DEBUG_STRUCTS
                if (g_theWriteableGlobalData->m_extraLogging) {
                    captainslog_debug("EARLY 5 (delaying damage applied until frame %d)", (int)frame);
                }
#endif

                return frame;
            } else {
                if (do_damage) {
                    Deal_Damage_Internal(source_id, target_id, &pos, bonus, is_projectile_detonation);
                }

#ifdef GAME_DEBUG_STRUCTS
                if (g_theWriteableGlobalData->m_extraLogging) {
                    captainslog_debug("EARLY 4 (delayed damage applied now)");
                }
#endif

                return g_theGameLogic->Get_Frame();
            }
        }
    }
}

bool Is_2D_Dist_Squared_Less_Than(const Coord3D *pos, const Coord3D *pos2, float dist_sqr)
{
    return GameMath::Square(pos->x - pos2->x) + GameMath::Square(pos->y - pos2->y) <= dist_sqr;
}

void WeaponTemplate::Deal_Damage_Internal(ObjectID source_id,
    ObjectID victim_id,
    const Coord3D *pos,
    const WeaponBonus &bonus,
    bool is_projectile_detonation) const
{
    if (source_id != OBJECT_UNK && (victim_id != OBJECT_UNK || pos != nullptr)) {
        Object *source = g_theGameLogic->Find_Object_By_ID(source_id);
        Trim_Old_Historic_Damage();

        if (m_historicBonusCount > 0 && m_historicBonusWeapon != this) {
            float radius_sqr = m_historicBonusRadius * m_historicBonusRadius;
            int count = 0;
            unsigned int frame = g_theGameLogic->Get_Frame();
            unsigned int frame2 = frame - m_historicBonusTime;

            for (auto it = m_historicDamage.begin(); it != m_historicDamage.end(); it++) {
                if (it->frame >= frame2) {
                    if (Is_2D_Dist_Squared_Less_Than(pos, &it->location, radius_sqr)) {
                        count++;
                    }
                }
            }

            if (count < m_historicBonusCount - 1) {
                HistoricWeaponDamageInfo info(frame, *pos);
                m_historicDamage.push_back(info);
            } else {
                g_theWeaponStore->Create_And_Fire_Temp_Weapon(m_historicBonusWeapon, source, pos);
                m_historicDamage.clear();
            }
        }

        Object *victim;

        if (victim_id != OBJECT_UNK) {
            victim = g_theGameLogic->Find_Object_By_ID(victim_id);
        } else {
            victim = nullptr;
        }

        if (victim != nullptr) {
            pos = victim->Get_Position();
        }

        DamageType damage = Get_Damage_Type();
        DeathType death = Get_Death_Type();
        ObjectStatusTypes status = Get_Damage_Status_Type();

        if (Get_Projectile_Template() == nullptr || is_projectile_detonation) {
            float primary_radius = Get_Primary_Damage_Radius(bonus);
            float secondary_radius = Get_Secondary_Damage_Radius(bonus);
            float primary_damage = Get_Primary_Damage(bonus);
            float secondary_damage = Get_Secondary_Damage(bonus);
            int affects_mask = Get_Affects_Mask();

            captainslog_dbgassert(secondary_radius >= primary_radius || secondary_radius == 0.0f,
                "secondary radius should be >= primary radius (or zero)");

            float primary_radius_sqr = GameMath::Square(primary_radius);
            float max_radius = std::max(primary_radius, secondary_radius);
            SimpleObjectIterator *iter;
            float numeric;
            Object *target;

            if (max_radius <= 0.0f) {
                iter = nullptr;
                numeric = 0.0f;
                target = victim;
                if ((affects_mask & AFFECTS_SUICIDE) != 0) {
                    DamageInfo info;
                    info.m_in.m_damageType = damage;
                    info.m_in.m_deathType = death;
                    info.m_in.m_sourceID = source_id;
                    info.m_in.m_playerMask = 0;
                    info.m_in.m_damageStatusType = status;
                    info.m_in.m_amount = 999999.0f;
                    source->Attempt_Damage(&info);
                }
            } else {
                iter = g_thePartitionManager->Iterate_Objects_In_Range(
                    pos, max_radius, FROM_BOUNDINGSPHERE_3D, nullptr, ITER_FASTEST);
                target = iter->First_With_Numeric(&numeric);
            }

            MemoryPoolObjectHolder holder(iter);

            for (;;) {
                if (target == nullptr) {
                    return;
                }

                bool suicide = false;
                DamageInfo info;
                float angle;
                Coord3D source_to_target_vec;

                if (source != nullptr && target != victim) {
                    if ((affects_mask & AFFECTS_SUICIDE) != 0 && source == target) {
                        suicide = true;
                    } else {
                        if ((affects_mask & AFFECTS_SELF) == 0) {
                            if (source == target) {
                                goto next;
                            }

                            if (source->Get_Producer_ID() == target->Get_ID()) {
                                goto next;
                            }
                        }

                        if ((affects_mask & AFFECTS_NOT_SIMILAR) != 0) {
                            if (source->Get_Template()->Is_Equivalent_To(target->Get_Template())) {
                                if (source->Get_Relationship(target) == ALLIES) {
                                    goto next;
                                }
                            }
                        }

                        if ((affects_mask & AFFECTS_NOT_AIRBORNE) != 0 && target->Is_Significantly_Above_Terrain()) {
                            goto next;
                        }

                        Relationship relationship = source->Get_Relationship(target);
                        WeaponAffectsType affects;

                        if (relationship == ALLIES) {
                            affects = AFFECTS_ALLIES;
                        } else if (relationship == ENEMIES) {
                            affects = AFFECTS_ENEMIES;
                        } else {
                            affects = AFFECTS_NEUTRALS;
                        }

                        if ((affects & affects_mask) == 0) {
                            goto next;
                        }
                    }
                }

                info.m_in.m_damageType = damage;
                info.m_in.m_deathType = death;
                info.m_in.m_sourceID = source_id;
                info.m_in.m_playerMask = 0;
                info.m_in.m_damageStatusType = status;
                source_to_target_vec.Zero();

                if (target != nullptr && source != nullptr) {
                    source_to_target_vec.Set(target->Get_Position());
                    source_to_target_vec.Sub(source->Get_Position());
                }

                angle = Get_Radius_Damage_Angle();

                if ((angle >= DEG_TO_RADF(180.0f) || target != nullptr) && source != nullptr) {
                    Vector3 source_forward_dir;
                    source->Get_Transform_Matrix()->Get_X_Vector(&source_forward_dir);
                    Vector3 source_to_target_dir(source_to_target_vec.x, source_to_target_vec.y, source_to_target_vec.z);
                    source_forward_dir.Normalize();
                    source_to_target_dir.Normalize();

                    if (GameMath::Cos(angle) <= source_forward_dir * source_to_target_dir) {
                        info.m_in.m_shockWaveAmount = m_shockWaveAmount;

                        if (info.m_in.m_shockWaveAmount > 0.0f) {
                            if (GameMath::Fabs(source_to_target_vec.x) < GAMEMATH_EPSILON
                                && GameMath::Fabs(source_to_target_vec.y) < GAMEMATH_EPSILON
                                && GameMath::Fabs(source_to_target_vec.z) < GAMEMATH_EPSILON) {
                                source_to_target_vec.z = 1.0f;
                            }

                            info.m_in.m_shockWavePos = source_to_target_vec;
                            info.m_in.m_shockWaveRadius = m_shockWaveRadius;
                            info.m_in.m_shockWaveTaperOff = m_shockWaveTaperOff;
                        }

                        if (source != nullptr && source->Get_Controlling_Player() != nullptr) {
                            info.m_in.m_playerMask = source->Get_Controlling_Player()->Get_Player_Mask();
                        }

                        if (numeric > primary_radius_sqr) {
                            info.m_in.m_amount = secondary_damage;
                        } else {
                            info.m_in.m_amount = primary_damage;
                        }

                        if (suicide) {
                            info.m_in.m_amount = 999999.0f;
                        }

                        if (source != nullptr && source->Is_KindOf(KINDOF_PROJECTILE)) {
                            ProjectileUpdateInterface *projectile = nullptr;

                            for (BehaviorModule **i = target->Get_All_Modules(); *i != nullptr; i++) {
                                projectile = (*i)->Get_Projectile_Update_Interface();

                                if (projectile != nullptr) {
                                    info.m_in.m_sourceID = projectile->Projectile_Get_Launcher_ID();
                                    break;
                                }
                            }
                        }

                        target->Attempt_Damage(&info);
                    }
                }

            next:
                if (iter != nullptr) {
                    target = iter->Next_With_Numeric(&numeric);
                } else {
                    target = nullptr;
                }
            }
        }

        captainslog_dbgassert(false, "projectile weapons should never get Deal_Damage_Internal called directly");
    }
}
