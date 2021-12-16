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
#include "audiomanager.h"
#include "gamelogic.h"
#include "ini.h"
#include "particlesystemplate.h"
#include "thingfactory.h"
#include <algorithm>

#ifndef GAME_DLL
WeaponStore *g_theWeaponStore = nullptr;
#endif

static void Parse_All_Vet_Levels_ASCII_String(INI *ini, void *formal, void *store, const void *user_data);
static void Parse_Per_Vet_Level_ASCII_String(INI *ini, void *formal, void *store, const void *user_data);
static void Parse_All_Vet_Levels_FXList(INI *ini, void *formal, void *store, const void *user_data);
static void Parse_Per_Vet_Level_FXList(INI *ini, void *formal, void *store, const void *user_data);
static void Parse_All_Vet_Levels_Particle_System(INI *ini, void *formal, void *store, const void *user_data);
static void Parse_Per_Vet_Level_Particle_System(INI *ini, void *formal, void *store, const void *user_data);

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

constexpr const char *TheDeathTypeNames[] = {
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

constexpr const char *TheAutoReloadsClipNames[] = {
    "YES",
    "NO",
    "RETURN_TO_BASE",
    nullptr,
};

constexpr const char *TheAttackTypeNames[] = {
    "PER_SHOT",
    "PER_ATTACK",
    "PER_CLIP",
    nullptr,
};

constexpr const char *TheRadiusDamageAffectsNames[] = {
    "SELF",
    "ALLIES",
    "ENEMIES",
    "NEUTRALS",
    "SUICIDE",
    "NOT_SIMILAR",
    "NOT_AIRBORNE",
    nullptr,
};

constexpr const char *TheProjectileCollidesWithNames[] = {
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

template<>
const char *BitFlags<DAMAGE_NUM_TYPES>::s_bitNamesList[DAMAGE_NUM_TYPES + 1] = {
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

// clang-format off
FieldParse WeaponTemplate::s_fieldParseTable[] = {
    { "ProjectileStreamName",               &INI::Parse_AsciiString,                        nullptr,                        offsetof(WeaponTemplate, m_projectileStreamName) },
    { "LaserName",                          &INI::Parse_AsciiString,                        nullptr,                        offsetof(WeaponTemplate, m_laserName) },
    { "LaserBoneName",                      &INI::Parse_AsciiString,                        nullptr,                        offsetof(WeaponTemplate, m_laserBoneName) },
    { "PrimaryDamage",                      &INI::Parse_Real,                               nullptr,                        offsetof(WeaponTemplate, m_primaryDamage) },
    { "PrimaryDamageRadius",                &INI::Parse_Real,                               nullptr,                        offsetof(WeaponTemplate, m_primaryDamageRadius) },
    { "SecondaryDamage",                    &INI::Parse_Real,                               nullptr,                        offsetof(WeaponTemplate, m_secondaryDamage) },
    { "SecondaryDamageRadius",              &INI::Parse_Real,                               nullptr,                        offsetof(WeaponTemplate, m_secondaryDamageRadius) },
    { "ShockWaveAmount",                    &INI::Parse_Real,                               nullptr,                        offsetof(WeaponTemplate, m_shockWaveAmount) },
    { "ShockWaveRadius",                    &INI::Parse_Real,                               nullptr,                        offsetof(WeaponTemplate, m_shockWaveRadius) },
    { "ShockWaveTaperOff",                  &INI::Parse_Real,                               nullptr,                        offsetof(WeaponTemplate, m_shockWaveTaperOff) },
    { "AttackRange",                        &INI::Parse_Real,                               nullptr,                        offsetof(WeaponTemplate, m_attackRange) },
    { "MinimumAttackRange",                 &INI::Parse_Real,                               nullptr,                        offsetof(WeaponTemplate, m_minimumAttackRange) },
    { "RequestAssistRange",                 &INI::Parse_Real,                               nullptr,                        offsetof(WeaponTemplate, m_requestAssistRange) },
    { "AcceptableAimDelta",                 &INI::Parse_Angle_Real,                         nullptr,                        offsetof(WeaponTemplate, m_aimDelta) },
    { "ScatterRadius",                      &INI::Parse_Real,                               nullptr,                        offsetof(WeaponTemplate, m_scatterRadius) },
    { "ScatterTargetScalar",                &INI::Parse_Real,                               nullptr,                        offsetof(WeaponTemplate, m_scatterTargetScalar) },
    { "ScatterTarget",                      &WeaponTemplate::Parse_Scatter_Target,          nullptr,                        offsetof(WeaponTemplate, m_scatterTarget) },
    { "DamageType",                         &BitFlags<DAMAGE_NUM_TYPES>::Parse_Single_Bit_From_INI, nullptr,                offsetof(WeaponTemplate, m_damageType) },
    { "DeathType",                          &INI::Parse_Index_List,                         TheDeathTypeNames,              offsetof(WeaponTemplate, m_deathType) },
    { "WeaponSpeed",                        &INI::Parse_Velocity_Real,                      nullptr,                        offsetof(WeaponTemplate, m_weaponSpeed) },
    { "MinWeaponSpeed",                     &INI::Parse_Velocity_Real,                      nullptr,                        offsetof(WeaponTemplate, m_minWeaponSpeed) },
    { "ScaleWeaponSpeed",                   &INI::Parse_Bool,                               nullptr,                        offsetof(WeaponTemplate, m_scaleWeaponSpeed) },
    { "WeaponRecoil",                       &INI::Parse_Angle_Real,                         nullptr,                        offsetof(WeaponTemplate, m_weaponRecoil) },
    { "MinTargetPitch",                     &INI::Parse_Angle_Real,                         nullptr,                        offsetof(WeaponTemplate, m_minTargetPitch) },
    { "MaxTargetPitch",                     &INI::Parse_Angle_Real,                         nullptr,                        offsetof(WeaponTemplate, m_maxTargetPitch) },
    { "RadiusDamageAngle",                  &INI::Parse_Angle_Real,                         nullptr,                        offsetof(WeaponTemplate, m_radiusDamageAngle) },
    { "ProjectileObject",                   &INI::Parse_AsciiString,                        nullptr,                        offsetof(WeaponTemplate, m_projectileObject) },
    { "FireOCL",                            &Parse_All_Vet_Levels_ASCII_String,             nullptr,                        offsetof(WeaponTemplate, m_fireOCL) },
    { "VeterancyFireOCL",                   &Parse_Per_Vet_Level_ASCII_String,              nullptr,                        offsetof(WeaponTemplate, m_fireOCL) },
    { "ProjectileDetonationOCL",            &Parse_All_Vet_Levels_ASCII_String,             nullptr,                        offsetof(WeaponTemplate, m_projectileDetonationOCL) },
    { "VeterancyProjectileDetonationOCL",   &Parse_Per_Vet_Level_ASCII_String,              nullptr,                        offsetof(WeaponTemplate, m_projectileDetonationOCL) },
    { "ProjectileExhaust",                  &Parse_All_Vet_Levels_Particle_System,          nullptr,                        offsetof(WeaponTemplate, m_projectileExhaust) },
    { "VeterancyProjectileExhaust",         &Parse_Per_Vet_Level_Particle_System,           nullptr,                        offsetof(WeaponTemplate, m_projectileExhaust) },
    { "FireFX",                             &Parse_All_Vet_Levels_FXList,                   nullptr,                        offsetof(WeaponTemplate, m_fireFX) },
    { "VeterancyFireFX",                    &Parse_Per_Vet_Level_FXList,                    nullptr,                        offsetof(WeaponTemplate, m_fireFX) },
    { "ProjectileDetonationFX",             &Parse_All_Vet_Levels_FXList,                   nullptr,                        offsetof(WeaponTemplate, m_projectileDetonateFX) },
    { "VeterancyProjectileDetonationFX",    &Parse_Per_Vet_Level_FXList,                    nullptr,                        offsetof(WeaponTemplate, m_projectileDetonateFX) },
    { "FireSound",                          &INI::Parse_Audio_Event_RTS,                    nullptr,                        offsetof(WeaponTemplate, m_fireSound) },
    { "FireSoundLoopTime",                  &INI::Parse_Duration_Unsigned_Int,              nullptr,                        offsetof(WeaponTemplate, m_fireSoundLoopTime) },
    { "WeaponBonus",                        &WeaponTemplate::Parse_Weapon_Bonus_Set,        nullptr,                        0 },
    { "ClipSize",                           &INI::Parse_Int,                                nullptr,                        offsetof(WeaponTemplate, m_clipSize) },
    { "ClipReloadTime",                     &INI::Parse_Duration_Unsigned_Int,              nullptr,                        offsetof(WeaponTemplate, m_clipReloadTime) },
    { "DelayBetweenShots",                  &WeaponTemplate::Parse_Shot_Delay,              nullptr,                        0 },
    { "ContinuousFireOne",                  &INI::Parse_Int,                                nullptr,                        offsetof(WeaponTemplate, m_continuousFireOneShotsNeeded) },
    { "ContinuousFireTwo",                  &INI::Parse_Int,                                nullptr,                        offsetof(WeaponTemplate, m_continuousFireTwoShotsNeeded) },
    { "ContinuousFireCoast",                &INI::Parse_Duration_Unsigned_Int,              nullptr,                        offsetof(WeaponTemplate, m_continuousFireCoastFrames) },
    { "AutoReloadWhenIdle",                 &INI::Parse_Duration_Unsigned_Int,              nullptr,                        offsetof(WeaponTemplate, m_autoReloadWhenIdle) },
    { "ShotsPerBarrel",                     &INI::Parse_Int,                                nullptr,                        offsetof(WeaponTemplate, m_shotsPerBarrel) },
    { "AntiAirborneVehicle",                &INI::Parse_Bit_In_Int32,                       reinterpret_cast<void*>(1 << ANTI_WEAPON_AIRBORNE_VEHICLE),     offsetof(WeaponTemplate, m_antiMask) },
    { "AntiGround",                         &INI::Parse_Bit_In_Int32,                       reinterpret_cast<void*>(1 << ANTI_WEAPON_GROUND),               offsetof(WeaponTemplate, m_antiMask) },
    { "AntiProjectile",                     &INI::Parse_Bit_In_Int32,                       reinterpret_cast<void*>(1 << ANTI_WEAPON_PROJECTILE),           offsetof(WeaponTemplate, m_antiMask) },
    { "AntiSmallMissile",                   &INI::Parse_Bit_In_Int32,                       reinterpret_cast<void*>(1 << ANTI_WEAPON_SMALL_MISSILE),        offsetof(WeaponTemplate, m_antiMask) },
    { "AntiMine",                           &INI::Parse_Bit_In_Int32,                       reinterpret_cast<void*>(1 << ANTI_WEAPON_MINE),                 offsetof(WeaponTemplate, m_antiMask) },
    { "AntiAirborneInfantry",               &INI::Parse_Bit_In_Int32,                       reinterpret_cast<void*>(1 << ANTI_WEAPON_AIRBORNE_INFANTRY),    offsetof(WeaponTemplate, m_antiMask) },
    { "AntiBallisticMissile",               &INI::Parse_Bit_In_Int32,                       reinterpret_cast<void*>(1 << ANTI_WEAPON_BALLISTIC_MISSILE),    offsetof(WeaponTemplate, m_antiMask) },
    { "AntiParachute",                      &INI::Parse_Bit_In_Int32,                       reinterpret_cast<void*>(1 << ANTI_WEAPON_PARACHUTE),            offsetof(WeaponTemplate, m_antiMask) },
    { "RadiusDamageAffects",                &INI::Parse_Bitstring32,                        TheRadiusDamageAffectsNames,    offsetof(WeaponTemplate, m_affectsMask) },
    { "ProjectileCollidesWith",             &INI::Parse_Bitstring32,                        TheProjectileCollidesWithNames, offsetof(WeaponTemplate, m_collideMask) },
    { "DamageDealtAtSelfPosition",          &INI::Parse_Bool,                               nullptr,                        offsetof(WeaponTemplate, m_damageDealtAtSelfPosition) },
    { "AutoReloadsClip",                    &INI::Parse_Index_List,                         TheAutoReloadsClipNames,        offsetof(WeaponTemplate, m_autoReloadsClip) },
    { "PreAttackType",                      &INI::Parse_Index_List,                         TheAttackTypeNames,             offsetof(WeaponTemplate, m_preAttackType) },
    { "HistoricBonusTime",                  &INI::Parse_Duration_Unsigned_Int,              nullptr,                        offsetof(WeaponTemplate, m_historicBonusTime) },
    { "HistoricBonusRadius",                &INI::Parse_Real,                               nullptr,                        offsetof(WeaponTemplate, m_historicBonusRadius) },
    { "HistoricBonusCount",                 &INI::Parse_Int,                                nullptr,                        offsetof(WeaponTemplate, m_historicBonusCount) },
    { "HistoricBonusWeapon",                &Parse_Weapon_Template,                         nullptr,                        offsetof(WeaponTemplate, m_historicBonusWeapon) },
    { "LeechRangeWeapon",                   &INI::Parse_Bool,                               nullptr,                        offsetof(WeaponTemplate, m_leechRangeWeapon) },
    { "CapableOfFollowingWaypoints",        &INI::Parse_Bool,                               nullptr,                        offsetof(WeaponTemplate, m_capableOfFollowingWaypoint) },
    { "ShowsAmmoPips",                      &INI::Parse_Bool,                               nullptr,                        offsetof(WeaponTemplate, m_showAmmoPips) },
    { "AllowAttackGarrisonedBldgs",         &INI::Parse_Bool,                               nullptr,                        offsetof(WeaponTemplate, m_allowAttackGarrisonedBldgs) },
    { "PlayFXWhenStealthed",                &INI::Parse_Bool,                               nullptr,                        offsetof(WeaponTemplate, m_playFXWhenStealthed) },
    { "PreAttackDelay",                     &INI::Parse_Duration_Unsigned_Int,              nullptr,                        offsetof(WeaponTemplate, m_preAttackDelay) },
    { "ContinueAttackRange",                &INI::Parse_Real,                               nullptr,                        offsetof(WeaponTemplate, m_continueAttackRange) },
    { "ScatterRadiusVsInfantry",            &INI::Parse_Real,                               nullptr,                        offsetof(WeaponTemplate, m_scatterRadiusVsInfantry) },
    { "DamageStatusType",                   &BitFlags<OBJECT_STATUS_COUNT>::Parse_Single_Bit_From_INI, nullptr,             offsetof(WeaponTemplate, m_damageStatusType) },
    { "SuspendFXDelay",                     &INI::Parse_Duration_Unsigned_Int,              nullptr,                        offsetof(WeaponTemplate, m_suspendFXDelay) },
    { "MissileCallsOnDie",                  &INI::Parse_Bool,                               nullptr,                        offsetof(WeaponTemplate, m_missileCallsOnDie) },
    { nullptr,                              nullptr,                                        nullptr,                        0 },
};
// clang-format on

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

// zh: 0x004C4260 wb: 0x006DC370
void WeaponTemplate::Deal_Damage_Internal(
    uint32_t sourceID, uint32_t victimID, Coord3D *pos, WeaponBonus &bonus, bool isProjectileDetonation)
{
#ifdef GAME_DLL
    Call_Method<void, WeaponTemplate, uint32_t, uint32_t, Coord3D *, WeaponBonus &, bool>(
        PICK_ADDRESS(0x004C4260, 0x006DC370), this, sourceID, victimID, pos, bonus, isProjectileDetonation);
#endif
}

// zh: 0x004C33B0 wb: 0x006D40BD
void WeaponTemplate::PostProcessLoad()
{
#ifdef GAME_DLL
    Call_Method<void, WeaponTemplate>(PICK_ADDRESS(0x004C33B0, 0x006D40BD), this);
#endif
}

// zh: 0x004C3240 wb: 0x006D3F37
void WeaponTemplate::Parse_Scatter_Target(INI *ini, void *formal, void *store, void const *user_data)
{
    Coord2D coord2d;
    ini->Parse_Coord2D(ini, nullptr, &coord2d, nullptr);
    static_cast<WeaponTemplate *>(formal)->m_scatterTarget.push_back(coord2d);
}

// zh: 0x004C3160 wb: 0x006D3E9E
void WeaponTemplate::Parse_Weapon_Bonus_Set(INI *ini, void *formal, void *store, void const *user_data)
{
    // TODO
#ifdef GAME_DLL
    Call_Function<void, INI *, void *, void *, void const *>(
        PICK_ADDRESS(0x004C3160, 0x006D3E9E), ini, formal, store, user_data);
#endif
}

// zh: 0x004C32B0 wb: 0x006D3F78
void WeaponTemplate::Parse_Shot_Delay(INI *ini, void *formal, void *store, void const *user_data)
{
    // TODO
#ifdef GAME_DLL
    Call_Function<void, INI *, void *, void *, void const *>(
        PICK_ADDRESS(0x004C32B0, 0x006D3F78), ini, formal, store, user_data);
#endif
}

void WeaponTemplate::Set_Name(const Utf8String &newName)
{
    m_name = newName;
    m_nameKey = g_theNameKeyGenerator->Name_To_Key(newName);
}

// zh: 0x004C54C0 wb: 0x006DB1A0
WeaponTemplate &WeaponTemplate::operator=(WeaponTemplate &rhs)
{
    m_nextTemplate = rhs.m_nextTemplate;
    m_name = rhs.m_name;
    m_nameKey = rhs.m_nameKey;
    m_projectileStreamName = rhs.m_projectileStreamName;
    m_laserName = rhs.m_laserName;
    m_laserBoneName = rhs.m_laserBoneName;
    m_primaryDamage = rhs.m_primaryDamage;
    m_primaryDamageRadius = rhs.m_primaryDamageRadius;
    m_secondaryDamage = rhs.m_secondaryDamage;
    m_secondaryDamageRadius = rhs.m_secondaryDamageRadius;
    m_shockWaveAmount = rhs.m_shockWaveAmount;
    m_shockWaveRadius = rhs.m_shockWaveRadius;
    m_shockWaveTaperOff = rhs.m_shockWaveTaperOff;
    m_attackRange = rhs.m_attackRange;
    m_minimumAttackRange = rhs.m_minimumAttackRange;
    m_requestAssistRange = rhs.m_requestAssistRange;
    m_aimDelta = rhs.m_aimDelta;
    m_scatterRadius = rhs.m_scatterRadius;
    m_scatterTargetScalar = rhs.m_scatterTargetScalar;
    m_scatterTarget = rhs.m_scatterTarget;
    m_damageType = rhs.m_damageType;
    m_deathType = rhs.m_deathType;
    m_weaponSpeed = rhs.m_weaponSpeed;
    m_minWeaponSpeed = rhs.m_minWeaponSpeed;
    m_scaleWeaponSpeed = rhs.m_scaleWeaponSpeed;
    m_weaponRecoil = rhs.m_weaponRecoil;
    m_minTargetPitch = rhs.m_minTargetPitch;
    m_maxTargetPitch = rhs.m_maxTargetPitch;
    m_radiusDamageAngle = rhs.m_radiusDamageAngle;
    m_projectileObject = rhs.m_projectileObject;
    unk = rhs.unk;
    for (auto i = 0; i < VETERANCY_COUNT; ++i) {
        m_fireOCL[i] = rhs.m_fireOCL[i];
        m_projectileDetonationOCL[i] = rhs.m_projectileDetonationOCL[i];
        m_projectileExhaust[i] = rhs.m_projectileExhaust[i];
        m_projectileName[i] = rhs.m_projectileName[i];
        m_projectileTmpl[i] = rhs.m_projectileTmpl[i];
        m_fireFX[i] = rhs.m_fireFX[i];
        m_projectileDetonateFX[i] = rhs.m_projectileDetonateFX[i];
    }
    m_fireSound = rhs.m_fireSound;
    m_fireSoundLoopTime = rhs.m_fireSoundLoopTime;
    m_extraBonus = rhs.m_extraBonus;
    m_clipSize = rhs.m_clipSize;
    m_clipReloadTime = rhs.m_clipReloadTime;
    m_minDelayBetweenShots = rhs.m_minDelayBetweenShots;
    m_maxDelayBetweenShots = rhs.m_maxDelayBetweenShots;
    m_continuousFireOneShotsNeeded = rhs.m_continuousFireOneShotsNeeded;
    m_continuousFireTwoShotsNeeded = rhs.m_continuousFireTwoShotsNeeded;
    m_continuousFireCoastFrames = rhs.m_continuousFireCoastFrames;
    m_autoReloadWhenIdle = rhs.m_autoReloadWhenIdle;
    m_shotsPerBarrel = rhs.m_shotsPerBarrel;
    m_antiMask = rhs.m_antiMask;
    m_affectsMask = rhs.m_affectsMask;
    m_collideMask = rhs.m_collideMask;
    m_damageDealtAtSelfPosition = rhs.m_damageDealtAtSelfPosition;
    m_autoReloadsClip = rhs.m_autoReloadsClip;
    m_preAttackType = rhs.m_preAttackType;
    m_historicBonusTime = rhs.m_historicBonusTime;
    m_historicBonusRadius = rhs.m_historicBonusRadius;
    m_historicBonusCount = rhs.m_historicBonusCount;
    m_historicBonusWeapon = rhs.m_historicBonusWeapon;
    m_leechRangeWeapon = rhs.m_leechRangeWeapon;
    m_capableOfFollowingWaypoint = rhs.m_capableOfFollowingWaypoint;
    m_showAmmoPips = rhs.m_showAmmoPips;
    m_allowAttackGarrisonedBldgs = rhs.m_allowAttackGarrisonedBldgs;
    m_playFXWhenStealthed = rhs.m_playFXWhenStealthed;
    m_preAttackDelay = rhs.m_preAttackDelay;
    m_continueAttackRange = rhs.m_continueAttackRange;
    m_scatterRadiusVsInfantry = rhs.m_scatterRadiusVsInfantry;
    m_damageStatusType = rhs.m_damageStatusType;
    m_suspendFXDelay = rhs.m_suspendFXDelay;
    m_missileCallsOnDie = rhs.m_missileCallsOnDie;
    m_historicDamage = rhs.m_historicDamage;

    return *this;
}

// zh: 0x0041CC40 wb: 0x007A39AE
void WeaponTemplate::Parse_Weapon_Template(INI *ini, void *formal, void *store, const void *user_data)
{
    Utf8String weapon_str = ini->Get_Next_Token();
    auto *weapon_template = g_theWeaponStore->Find_Weapon_Template(weapon_str);
    captainslog_dbgassert(
        weapon_template != nullptr || weapon_str.Is_None(), "WeaponTemplate %s not found!\n", weapon_str.Str());
    *static_cast<WeaponTemplate **>(store) = weapon_template;
}

// zh: 0x004C4E40 wb: 0x006D654E
WeaponStore::~WeaponStore()
{
    Delete_All_Delayed_Damage();
    for (auto *weapon_template : m_weaponTemplateVector) {
        if (weapon_template != nullptr) {
            weapon_template->Delete_Instance();
        }
    }
}

// zh: 0x004C5BF0 wb: 0x006D6BEE
void WeaponStore::PostProcessLoad()
{
    if (g_theThingFactory == nullptr) {
        captainslog_error("You must call this after TheThingFactory");
        return;
    }

    for (auto *weapon_template : m_weaponTemplateVector) {
        weapon_template->PostProcessLoad();
    }
}

// zh: 0x004C5B10 wb: 0x006D6B0C
void WeaponStore::Reset()
{
    for (auto *weapon_template : m_weaponTemplateVector) {
        if (weapon_template->Has_Next_Template()) {
            auto *next_template = weapon_template->Clear_Next_Template();
            next_template->Delete_Instance();
        }
    }

    Delete_All_Delayed_Damage();
    Reset_Weapon_Templates();
}

// zh: 0x004C5AB0 wb: 0x006D69D3
void WeaponStore::Update()
{
    // Find any delayed damage that should now be applied and apply it.
    for (auto DDI_iter = m_weaponDDI.begin(); DDI_iter != m_weaponDDI.end();) {
        // Have cur_iter point at the current item and DDI point at the next item
        // Two iterators required so that items can be correctly erased.
        auto cur_iter = DDI_iter++;

        auto &delayed_damage = *cur_iter;
        if (delayed_damage.m_delayDamageFrame > g_theGameLogic->Get_Frame()) {
            continue;
        }

        delayed_damage.m_delayedWeapon->Deal_Damage_Internal(delayed_damage.m_delaySourceID,
            delayed_damage.m_delayIntendedVictimID,
            &delayed_damage.m_delayDamagePos,
            delayed_damage.m_bonus,
            false);

        m_weaponDDI.erase(cur_iter);
    }
}

// wb: 0x006D6ABF
void WeaponStore::Reset_Weapon_Templates()
{
    for (auto *weapon_template : m_weaponTemplateVector) {
        weapon_template->Reset();
    }
}

WeaponTemplate *WeaponStore::Find_Weapon_Template_Private(NameKeyType key)
{
    auto res = std::find_if(m_weaponTemplateVector.begin(),
        m_weaponTemplateVector.end(),
        [&](WeaponTemplate *weapon_template) { return weapon_template->Get_Name_Key() == key; });

    if (res == m_weaponTemplateVector.end()) {
        return nullptr;
    }
    return *res;
}

// zh: 0x004C52D0 wb: 0x006D684B
WeaponTemplate *WeaponStore::New_Weapon_Template(Utf8String name)
{
    if (name.Is_Empty()) {
        return nullptr;
    }

    auto *new_weapon_template = NEW_POOL_OBJ(WeaponTemplate);
    new_weapon_template->Set_Name(name);
    m_weaponTemplateVector.push_back(new_weapon_template);

    return new_weapon_template;
}

// wb: 0x006D693A
WeaponTemplate *WeaponStore::New_Override(WeaponTemplate *weapon_template)
{
    if (weapon_template == nullptr) {
        return nullptr;
    }

    auto *new_weapon_template = NEW_POOL_OBJ(WeaponTemplate);
    *new_weapon_template = *weapon_template;
    new_weapon_template->Set_Next(weapon_template);

    return new_weapon_template;
}

// zh: 0x004C51B0 wb: 0x006D6703
WeaponTemplate *WeaponStore::Find_Weapon_Template(Utf8String name)
{
    if (name.Is_None()) {
        return nullptr;
    }

    const auto key = g_theNameKeyGenerator->Name_To_Key(name);
    auto *weapon_template = Find_Weapon_Template_Private(key);
    captainslog_dbgassert(weapon_template != nullptr, "Weapon %s not found!\n", name.Str());

    return weapon_template;
}

// zh: 0x004C2A10 wb: 0x006D36E7
static void Parse_All_Vet_Levels_ASCII_String(INI *ini, void *formal, void *store, const void *user_data)
{
    auto OCL_str = ini->Get_Next_Ascii_String();
    auto *OCL_vet_array = static_cast<Utf8String *>(store);
    for (auto i = 0; i < VETERANCY_COUNT; ++i) {
        OCL_vet_array[i] = OCL_str;
    }
}

// zh: 0x004C2950 wb: 0x006D3660
static void Parse_Per_Vet_Level_ASCII_String(INI *ini, void *formal, void *store, const void *user_data)
{
    const auto vet_level = ini->Scan_IndexList(ini->Get_Next_Token(), g_veterancyNames);
    auto *OCL_vet_array = static_cast<Utf8String *>(store);
    OCL_vet_array[vet_level] = ini->Get_Next_Ascii_String();
}

// zh: 0x004C2B20 wb: 0x006D37B6
static void Parse_All_Vet_Levels_FXList(INI *ini, void *formal, void *store, const void *user_data)
{
    // TODO requires FXList
#ifdef GAME_DLL
    Call_Function<void, INI *, void *, void *, void const *>(
        PICK_ADDRESS(0x004C2B20, 0x006D37B6), ini, formal, store, user_data);
#endif
}

// zh: 0x004C2AD0 wb: 0x006D3764
static void Parse_Per_Vet_Level_FXList(INI *ini, void *formal, void *store, const void *user_data)
{
    // TODO requires FXList
#ifdef GAME_DLL
    Call_Function<void, INI *, void *, void *, void const *>(
        PICK_ADDRESS(0x004C2AD0, 0x006D3764), ini, formal, store, user_data);
#endif
}

// zh: 0x004C2BB0 wb: 0x006D3859
static void Parse_All_Vet_Levels_Particle_System(INI *ini, void *formal, void *store, const void *user_data)
{
    ParticleSystemTemplate *particle_template = nullptr;
    ParticleSystemTemplate::Parse(ini, nullptr, &particle_template, nullptr);

    auto *particle_syst_vet_array = static_cast<ParticleSystemTemplate **>(store);
    for (auto i = 0; i < VETERANCY_COUNT; ++i) {
        particle_syst_vet_array[i] = particle_template;
    }
}

// zh: 0x004C2B60 wb: 0x006D3807
static void Parse_Per_Vet_Level_Particle_System(INI *ini, void *formal, void *store, const void *user_data)
{
    const auto vet_level = ini->Scan_IndexList(ini->Get_Next_Token(), g_veterancyNames);

    auto *particle_syst_vet_array = static_cast<ParticleSystemTemplate **>(store);

    ParticleSystemTemplate *particle_template = nullptr;
    ParticleSystemTemplate::Parse(ini, nullptr, &particle_template, nullptr);

    particle_syst_vet_array[vet_level] = particle_template;
}

// zh: 0x004C5C30 wb: 0x006D6C7E
void WeaponStore::Parse_Weapon_Template_Definition(INI *ini)
{
    auto *weapon_store = g_theWeaponStore;
    auto *template_name = ini->Get_Next_Token();

    // This duplicates Find_Weapon_Template but allows for nullptr returns
    const auto key = g_theNameKeyGenerator->Name_To_Key(template_name);
    auto *weapon_template = weapon_store->Find_Weapon_Template_Private(key);

    if (weapon_template == nullptr) {
        weapon_template = weapon_store->New_Weapon_Template(template_name);
    } else {
        if (ini->Get_Load_Type() == INILoadType::INI_LOAD_CREATE_OVERRIDES) {
            weapon_template = weapon_store->New_Override(weapon_template);
        } else {
            captainslog_debug("Weapon '%s' already exists, but OVERRIDE not specified!", template_name);
            return;
        }
    }
    ini->Init_From_INI(weapon_template, WeaponTemplate::Get_Weapon_Template_Field_Parse_Table());

    if (weapon_template->Get_Projectile_Name().Is_Not_None()) {
        weapon_template->Clear_Projectile_Name();
    }

    auto &fs_name = weapon_template->Get_Fire_Sound().Get_Event_Name();
    if (fs_name.Is_Not_Empty() && !fs_name.Compare_No_Case("NoSound")) {
        captainslog_dbgassert(g_theAudio->Is_Valid_Audio_Event(&weapon_template->Get_Fire_Sound()),
            "Invalid FireSound %s in Weapon '%s'",
            fs_name.Str(),
            weapon_template->Get_Name().Str());
    }
}
