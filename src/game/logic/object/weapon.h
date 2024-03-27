/**
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
#pragma once

#include "always.h"
#include "asciistring.h"
#include "audioeventrts.h"
#include "coord.h"
#include "gametype.h"
#include "mempoolobj.h"
#include "namekeygenerator.h"
#include "snapshot.h"
#include "weaponset.h"
#include <list>
#include <vector>

class FXList;
class INI;
class Object;
class ObjectCreationList;
class ParticleSystemTemplate;
class ThingTemplate;

enum AttackType
{
    ATTACK_TYPE_PER_SHOT,
    ATTACK_TYPE_PER_ATTACK,
    ATTACK_TYPE_PER_CLIP,
};

enum AutoReloadsClip
{
    AUTO_RELOADS_CLIP_YES,
    AUTO_RELOADS_CLIP_NO,
    AUTO_RELOADS_CLIP_RETURN_TO_BASE,
};

enum WeaponBonusConditionType
{
    WEAPONBONUSCONDITION_INVALID = -1,
    WEAPONBONUSCONDITION_GARRISONED,
    WEAPONBONUSCONDITION_HORDE,
    WEAPONBONUSCONDITION_CONTINUOUS_FIRE_MEAN,
    WEAPONBONUSCONDITION_CONTINUOUS_FIRE_FAST,
    WEAPONBONUSCONDITION_NATIONALISM,
    WEAPONBONUSCONDITION_PLAYER_UPGRADE,
    WEAPONBONUSCONDITION_DRONE_SPOTTING,
    WEAPONBONUSCONDITION_DEMORALIZED_OBSOLETE,
    WEAPONBONUSCONDITION_ENTHUSIASTIC,
    WEAPONBONUSCONDITION_VETERAN,
    WEAPONBONUSCONDITION_ELITE,
    WEAPONBONUSCONDITION_HERO,
    WEAPONBONUSCONDITION_BATTLEPLAN_BOMBARDMENT,
    WEAPONBONUSCONDITION_BATTLEPLAN_HOLDTHELINE,
    WEAPONBONUSCONDITION_BATTLEPLAN_SEARCHANDDESTROY,
    WEAPONBONUSCONDITION_SUBLIMINAL,
    WEAPONBONUSCONDITION_SOLO_HUMAN_EASY,
    WEAPONBONUSCONDITION_SOLO_HUMAN_NORMAL,
    WEAPONBONUSCONDITION_SOLO_HUMAN_HARD,
    WEAPONBONUSCONDITION_SOLO_AI_EASY,
    WEAPONBONUSCONDITION_SOLO_AI_NORMAL,
    WEAPONBONUSCONDITION_SOLO_AI_HARD,
    WEAPONBONUSCONDITION_TARGET_FAERIE_FIRE,
    WEAPONBONUSCONDITION_FANATICISM,
    WEAPONBONUSCONDITION_FRENZY_ONE,
    WEAPONBONUSCONDITION_FRENZY_TWO,
    WEAPONBONUSCONDITION_FRENZY_THREE,
    WEAPONBONUSCONDITION_COUNT,
};

enum WeaponCollideType
{
    COLLIDE_ALLIES = 1,
    COLLIDE_ENEMIES = 2,
    COLLIDE_STRUCTURES = 4,
    COLLIDE_SHRUBBERY = 8,
    COLLIDE_PROJECTILES = 0x10,
    COLLIDE_WALLS = 0x20,
    COLLIDE_SMALL_MISSILES = 0x40,
    COLLIDE_BALLISTIC_MISSILES = 0x80,
    COLLIDE_CONTROLLED_STRUCTURES = 0x100,
};

enum WeaponAntiType
{
    ANTI_AIRBORNE_VEHICLE = 1,
    ANTI_GROUND = 2,
    ANTI_PROJECTILE = 4,
    ANTI_SMALL_MISSILE = 8,
    ANTI_MINE = 0x10,
    ANTI_AIRBORNE_INFANTRY = 0x20,
    ANTI_BALLISTIC_MISSILE = 0x40,
    ANTI_PARACHUTE = 0x80,
};

enum WeaponAffectsType
{
    AFFECTS_SELF = 1,
    AFFECTS_ALLIES = 2,
    AFFECTS_ENEMIES = 4,
    AFFECTS_NEUTRALS = 8,
    AFFECTS_SUICIDE = 0x10,
    AFFECTS_NOT_SIMILAR = 0x20,
    AFFECTS_NOT_AIRBORNE = 0x40,
};

class WeaponBonus
{
public:
    enum Field
    {
        INVALID = -1,
        DAMAGE,
        RADIUS,
        RANGE,
        RATE_OF_FIRE,
        PRE_ATTACK,
        COUNT,
    };

    WeaponBonus() { Clear(); }
    void Clear()
    {
        for (int i = 0; i < COUNT; i++) {
            m_field[i] = 1.0f;
        }
    }
    float Get_Field(Field f) const { return m_field[f]; }
    void Set_Field(Field f, float bonus) { m_field[f] = bonus; }
    void Append_Bonuses(WeaponBonus &bonus)
    {
        for (int i = 0; i < COUNT; i++) {
            bonus.m_field[i] = m_field[i] - 1.0f + bonus.m_field[i];
        }
    }

private:
    float m_field[COUNT];
};

class WeaponBonusSet : public MemoryPoolObject
{
    IMPLEMENT_POOL(WeaponBonusSet);
    friend class GlobalData;

protected:
    virtual ~WeaponBonusSet() override {}

public:
    static void Parse_Weapon_Bonus_Set_Ptr(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Weapon_Bonus_Set(INI *ini, void *formal, void *store, const void *user_data);
    void Parse_Weapon_Bonus_Set(INI *ini);
    WeaponBonusSet() {}
    void Append_Bonuses(unsigned int flags, WeaponBonus &bonus)
    {
        if (flags) {
            for (int i = 0; i < WEAPONBONUSCONDITION_COUNT; i++) {
                if ((1 << i & flags) != 0) {
                    m_bonus[i].Append_Bonuses(bonus);
                }
            }
        }
    }

private:
    WeaponBonus m_bonus[WEAPONBONUSCONDITION_COUNT];
};

struct HistoricWeaponDamageInfo
{
    unsigned int frame;
    Coord3D location;
    HistoricWeaponDamageInfo(unsigned int f, const Coord3D &loc) : frame(f), location(loc) {}
};

class WeaponTemplate : public MemoryPoolObject
{
    IMPLEMENT_POOL(WeaponTemplate);

public:
    WeaponTemplate();

#ifdef GAME_DLL
    WeaponTemplate *Hook_Ctor() { return new (this) WeaponTemplate(); }
#endif

    virtual ~WeaponTemplate() override;

    void Reset();
    void Post_Process_Load();
    bool Is_Contact_Weapon() const;
    void Trim_Old_Historic_Damage() const;

    float Estimate_Weapon_Template_Damage(
        const Object *source_obj, const Object *victim_obj, const Coord3D *victim_pos, const WeaponBonus &bonus) const;

    bool Should_Projectile_Collide_With(
        const Object *source_obj, const Object *projectile_obj, const Object *collide_obj, ObjectID id) const;

    unsigned int Fire_Weapon_Template(const Object *source_obj,
        WeaponSlotType wslot,
        int specific_barrel_to_use,
        Object *victim_obj,
        const Coord3D *victim_pos,
        const WeaponBonus &bonus,
        bool is_projectile_detonation,
        bool ignore_ranges,
        Weapon *firing_weapon,
        ObjectID *projectile_id,
        bool do_damage) const;

    void Deal_Damage_Internal(ObjectID source_id,
        ObjectID victim_id,
        const Coord3D *pos,
        const WeaponBonus &bonus,
        bool is_projectile_detonation) const;

    float Get_Attack_Range(const WeaponBonus &bonus) const;
    float Get_Minimum_Attack_Range() const;
    float Get_Unmodified_Attack_Range() const;
    int Get_Delay_Between_Shots(const WeaponBonus &bonus) const;
    int Get_Clip_Reload_Time(const WeaponBonus &bonus) const;
    int Get_Pre_Attack_Delay(const WeaponBonus &bonus) const;
    float Get_Primary_Damage(const WeaponBonus &bonus) const;
    float Get_Primary_Damage_Radius(const WeaponBonus &bonus) const;
    float Get_Secondary_Damage(const WeaponBonus &bonus) const;
    float Get_Secondary_Damage_Radius(const WeaponBonus &bonus) const;

    float Get_Min_Target_Pitch() const { return m_minTargetPitch; }
    float Get_Max_Target_Pitch() const { return m_maxTargetPitch; }
    int Get_Shots_Per_Barrel() const { return m_shotsPerBarrel; }
    unsigned int Get_Suspend_FX_Delay() const { return m_suspendFXDelay; }
    Utf8String Get_Name() const { return m_name; }
    int Get_Anti_Mask() const { return m_antiMask; }
    DamageType Get_Damage_Type() const { return m_damageType; }
    DeathType Get_Death_Type() const { return m_deathType; }
    int Get_Collide_Mask() const { return m_collideMask; }
    float Get_Weapon_Speed() const { return m_weaponSpeed; }
    float Get_Weapon_Recoil() const { return m_weaponRecoil; }
    const ThingTemplate *Get_Projectile_Template() const { return m_projectileTemplate; }
    FXList *Get_Fire_FX(VeterancyLevel level) const { return m_fireFX[level]; }
    FXList *Get_Projectile_Detonate_FX(VeterancyLevel level) const { return m_projectileDetonateFX[level]; }
    ObjectCreationList *Get_Fire_OCL(VeterancyLevel level) const { return m_fireOCL[level]; }
    Utf8String Get_Laser_Name() const { return m_laserName; }
    float Get_Radius_Damage_Angle() const { return m_radiusDamageAngle; }
    ObjectStatusTypes Get_Damage_Status_Type() const { return m_damageStatusType; }
    int Get_Affects_Mask() const { return m_affectsMask; }
    NameKeyType Get_Name_Key() const { return m_nameKey; }
    const AudioEventRTS *Get_Fire_Sound() const { return &m_fireSound; }
    WeaponBonusSet *Get_Extra_Bonus() const { return m_extraBonus; }
    int Get_Clip_Size() const { return m_clipSize; }
    const std::vector<Coord2D> &Get_Scatter_Targets() const { return m_scatterTarget; }
    Utf8String Get_Projectile_Stream_Name() const { return m_projectileStreamName; }
    const Utf8String &Get_Laser_Bone_Name() const { return m_laserBoneName; }
    float Get_Request_Assist_Range() const { return m_requestAssistRange; }
    float Get_Scatter_Target_Scalar() const { return m_scatterTargetScalar; }
    float Get_Scatter_Radius() const { return m_scatterRadius; }
    AttackType Get_Pre_Attack_Type() const { return m_preAttackType; }
    float Get_Continue_Attack_Range() const { return m_continueAttackRange; }
    float Get_Aim_Delta() const { return m_aimDelta; }
    unsigned int Get_Auto_Reload_When_Idle() const { return m_autoReloadWhenIdle; }
    unsigned int Get_Continuous_Fire_Cost_Frames() const { return m_continuousFireCoastFrames; }
    unsigned int Get_Continuous_Fire_One_Shots_Needed() const { return m_continuousFireOneShotsNeeded; }
    unsigned int Get_Continuous_Fire_Two_Shots_Needed() const { return m_continuousFireTwoShotsNeeded; }
    unsigned int Get_Fire_Sound_Loop_Time() const { return m_fireSoundLoopTime; }

    ObjectCreationList *Get_Projectile_Detonation_OCL(VeterancyLevel level) const
    {
        return m_projectileDetonationOCL[level];
    }

    bool Is_Show_Ammo_Pips() const { return m_showAmmoPips; }
    bool Is_Capable_Of_Following_Waypoint() const { return m_capableOfFollowingWaypoint; }
    bool Is_Damage_Dealt_At_Self_Position() const { return m_damageDealtAtSelfPosition; }
    bool Is_Leech_Range_Weapon() const { return m_leechRangeWeapon; }
    bool Is_Auto_Reloads_Clip() const { return m_autoReloadsClip == AUTO_RELOADS_CLIP_YES; }
    bool Is_Play_FX_When_Stealthed() const { return m_playFXWhenStealthed; }
    bool Is_Override() const { return m_nextTemplate != nullptr; }

    void Friend_Set_Next_Template(WeaponTemplate *tmplate) { m_nextTemplate = tmplate; }
    WeaponTemplate *Friend_Clear_Next_Template()
    {
        WeaponTemplate *tmplate = m_nextTemplate;
        m_nextTemplate = nullptr;
        return tmplate;
    }

    static void Parse_Weapon_Bonus_Set(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Scatter_Target(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Shot_Delay(INI *ini, void *formal, void *store, const void *user_data);
    static FieldParse *Get_Field_Parse() { return s_fieldParseTable; }

private:
    WeaponTemplate *m_nextTemplate;
    Utf8String m_name;
    NameKeyType m_nameKey;
    Utf8String m_projectileStreamName;
    Utf8String m_laserName;
    Utf8String m_laserBoneName;
    float m_primaryDamage;
    float m_primaryDamageRadius;
    float m_secondaryDamage;
    float m_secondaryDamageRadius;
    float m_shockWaveAmount;
    float m_shockWaveRadius;
    float m_shockWaveTaperOff;
    float m_attackRange;
    float m_minimumAttackRange;
    float m_requestAssistRange;
    float m_aimDelta;
    float m_scatterRadius;
    float m_scatterTargetScalar;
    std::vector<Coord2D> m_scatterTarget;
    DamageType m_damageType;
    DeathType m_deathType;
    float m_weaponSpeed;
    float m_minWeaponSpeed;
    bool m_scaleWeaponSpeed;
    float m_weaponRecoil;
    float m_minTargetPitch;
    float m_maxTargetPitch;
    float m_radiusDamageAngle;
    Utf8String m_projectileObject;
    ThingTemplate *m_projectileTemplate;
    Utf8String m_fireOCLName[VETERANCY_COUNT];
    Utf8String m_projectileDetonationOCLName[VETERANCY_COUNT];
    ParticleSystemTemplate *m_projectileExhaust[VETERANCY_COUNT];
    ObjectCreationList *m_fireOCL[VETERANCY_COUNT];
    ObjectCreationList *m_projectileDetonationOCL[VETERANCY_COUNT];
    FXList *m_fireFX[VETERANCY_COUNT];
    FXList *m_projectileDetonateFX[VETERANCY_COUNT];
    AudioEventRTS m_fireSound;
    unsigned int m_fireSoundLoopTime;
    WeaponBonusSet *m_extraBonus;
    int m_clipSize;
    int m_clipReloadTime;
    int m_minDelayBetweenShots;
    int m_maxDelayBetweenShots;
    int m_continuousFireOneShotsNeeded;
    int m_continuousFireTwoShotsNeeded;
    unsigned int m_continuousFireCoastFrames;
    unsigned int m_autoReloadWhenIdle;
    int m_shotsPerBarrel;
    int m_antiMask;
    int m_affectsMask;
    int m_collideMask;
    bool m_damageDealtAtSelfPosition;
    AutoReloadsClip m_autoReloadsClip;
    AttackType m_preAttackType;
    unsigned int m_historicBonusTime;
    float m_historicBonusRadius;
    int m_historicBonusCount;
    WeaponTemplate *m_historicBonusWeapon;
    bool m_leechRangeWeapon;
    bool m_capableOfFollowingWaypoint;
    bool m_showAmmoPips;
    bool m_allowAttackGarrisonedBldgs;
    bool m_playFXWhenStealthed;
    unsigned int m_preAttackDelay;
    float m_continueAttackRange;
    float m_scatterRadiusVsInfantry;
    ObjectStatusTypes m_damageStatusType;
    unsigned int m_suspendFXDelay;
    bool m_missileCallsOnDie;
    mutable std::list<HistoricWeaponDamageInfo> m_historicDamage;

    static FieldParse s_fieldParseTable[];
    friend class WeaponStore;
};

struct AssistanceRequestData
{
    const Object *source_obj;
    Object *target_obj;
    float request_assist_range_sqr;
    AssistanceRequestData() : source_obj(nullptr), target_obj(nullptr), request_assist_range_sqr(0.0f) {}
};

class Weapon : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_POOL(Weapon);

public:
    enum WeaponStatus
    {
        READY_TO_FIRE,
        OUT_OF_AMMO,
        BETWEEN_FIRING_SHOTS,
        RELOADING_CLIP,
        PRE_ATTACK,
    };

    Weapon(const WeaponTemplate *tmpl, WeaponSlotType wslot);
    Weapon(const Weapon &that);

    virtual ~Weapon() override {}
    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    Weapon &operator=(const Weapon &that);

    bool Is_Within_Attack_Range(const Object *source_obj, const Object *target_obj) const;
    bool Is_Within_Attack_Range(const Object *source_obj, const Coord3D *target_pos) const;
    bool Is_Within_Target_Pitch(const Object *source_obj, const Object *target_obj) const;
    bool Is_Damage_Weapon() const;
    bool Is_Too_Close(const Object *source_obj, const Object *target_obj) const;
    bool Is_Too_Close(const Object *source_obj, const Coord3D *target_pos) const;
    bool Is_Clear_Firing_Line_Of_Sight_Terrain(const Object *source_obj, const Object *target_obj) const;
    bool Is_Clear_Firing_Line_Of_Sight_Terrain(const Object *source_obj, const Coord3D &target_pos) const;

    bool Is_Goal_Pos_Within_Attack_Range(
        const Object *source_obj, const Coord3D *goal_pos, const Object *target_obj, const Coord3D *target_pos) const;

    bool Is_Source_Object_With_Goal_Position_Within_Attack_Range(
        const Object *source_obj, const Coord3D *goal_pos, const Object *target_obj, const Coord3D *target_pos) const;

    bool Is_Clear_Goal_Firing_Line_Of_Sight_Terrain(
        const Object *source_obj, const Coord3D &goal_pos, const Object *target_obj) const;

    bool Is_Clear_Goal_Firing_Line_Of_Sight_Terrain(
        const Object *source_obj, const Coord3D &goal_pos, const Coord3D &target_pos) const;

    float Get_Attack_Range(const Object *source_obj) const;
    WeaponStatus Get_Status() const;
    float Get_Percent_Ready_To_Fire() const;
    int Get_Clip_Reload_Time(const Object *source_obj) const;
    float Get_Attack_Distance(const Object *source_obj, const Object *victim_obj, const Coord3D *victim_pos) const;
    float Get_Primary_Damage_Radius(const Object *source_obj) const;
    int Get_Pre_Attack_Delay(const Object *source_obj, const Object *target_obj) const;
    void Get_Firing_Line_Of_Sight_Origin(const Object *source_obj, Coord3D &pos) const;

    void On_Weapon_Bonus_Change(const Object *source_obj);
    void Load_Ammo_Now(const Object *source_obj);
    void Reload_Ammo(const Object *source_obj);
    void Compute_Bonus(const Object *source_obj, unsigned int flags, WeaponBonus &bonus) const;
    void Set_Clip_Percent_Full(float percent, bool b);
    void Rebuild_Scatter_Targets();
    void Reload_With_Bonus(const Object *source_obj, const WeaponBonus &bonus, bool load_instantly);
    float Estimate_Weapon_Damage(const Object *source_obj, const Object *victim_obj, const Coord3D *victim_pos);
    void Create_Laser(const Object *source_obj, const Object *victim_obj, const Coord3D *victim_pos);
    void Pre_Fire_Weapon(const Object *source_obj, const Object *victim_obj);
    bool Fire_Weapon(const Object *source_obj, Object *victim_obj, ObjectID *projectile_id);
    bool Fire_Weapon(const Object *source_obj, const Coord3D *victim_pos, ObjectID *projectile_id);
    Object *Force_Fire_Weapon(const Object *source_obj, const Coord3D *victim_pos);
    void Process_Request_Assistance(const Object *source_obj, Object *target_obj);
    void Transfer_Next_Shot_Stats_From(const Weapon &weapon);

    static void Position_Projectile_For_Launch(
        Object *projectile_obj, const Object *source_obj, WeaponSlotType wslot, int ammo_index);

    bool Compute_Approach_Target(const Object *source_obj,
        const Object *target_obj,
        const Coord3D *target_pos,
        float angle_offset,
        Coord3D &approach_target_pos) const;

    void New_Projectile_Fired(
        const Object *source_obj, const Object *projectile_obj, const Object *target_obj, const Coord3D *target_pos);

    bool Private_Fire_Weapon(const Object *source_obj,
        Object *victim_obj,
        const Coord3D *victim_pos,
        bool is_projectile_detonation,
        bool ignore_ranges,
        unsigned int bonus_condition,
        ObjectID *projectile_id,
        bool do_damage);

    bool Fire_Projectile_Detonation_Weapon(
        const Object *source_obj, Object *victim_obj, unsigned int bonus_condition, bool do_damage);

    bool Fire_Projectile_Detonation_Weapon(
        const Object *source_obj, const Coord3D *victim_pos, unsigned int bonus_condition, bool do_damage);

    static void Calc_Projectile_Launch_Position(
        const Object *source_obj, WeaponSlotType wslot, int ammo_index, Matrix3D &launch_transform, Coord3D &launch_pos);

    int Get_Anti_Mask() const { return m_template->Get_Anti_Mask(); }
    DamageType Get_Damage_Type() const { return m_template->Get_Damage_Type(); }
    unsigned int Get_Suspend_FX_Delay() const { return m_suspendFXDelay; }
    unsigned int Get_Next_Shot() const { return m_whenWeCanFireAgain; }
    float Get_Scatter_Target_Scalar() const { return m_template->Get_Scatter_Target_Scalar(); }
    float Get_Scatter_Radius() const { return m_template->Get_Scatter_Radius(); }
    unsigned int Get_Last_Reload_Started() const { return m_whenLastReloadStarted; }
    int Get_Clip_Size() const { return m_template->Get_Clip_Size(); }
    int Get_Last_Fire_Frame() const { return m_lastFireFrame; }
    unsigned int Get_When_Pre_Attack_Finished() const { return m_whenPreAttackFinished; }
    WeaponSlotType Get_Weapon_Slot() const { return m_wslot; }
    Utf8String Get_Name() const { return m_template->Get_Name(); }
    float Get_Continue_Attack_Range() const { return m_template->Get_Continue_Attack_Range(); }
    float Get_Aim_Delta() const { return m_template->Get_Aim_Delta(); }
    int Get_Max_Shot_Count() const { return m_maxShotCount; }
    unsigned int Get_Auto_Reload_When_Idle() const { return m_template->Get_Auto_Reload_When_Idle(); }
    unsigned int Get_Continuous_Fire_Cost_Frames() const { return m_template->Get_Continuous_Fire_Cost_Frames(); }
    unsigned int Get_Continuous_Fire_One_Shots_Needed() const { return m_template->Get_Continuous_Fire_One_Shots_Needed(); }
    unsigned int Get_Continuous_Fire_Two_Shots_Needed() const { return m_template->Get_Continuous_Fire_Two_Shots_Needed(); }
    unsigned int Get_Fire_Sound_Loop_Time() const { return m_template->Get_Fire_Sound_Loop_Time(); }
    const AudioEventRTS *Get_Fire_Sound() const { return m_template->Get_Fire_Sound(); }

    unsigned int Get_Ammo_In_Clip() const
    {
        if (Get_Status() == RELOADING_CLIP) {
            return 0;
        } else {
            return m_ammoInClip;
        }
    }

    bool Is_Pitch_Limited() const { return m_pitchLimited; }
    bool Is_Show_Ammo_Pips() const { return m_template->Is_Show_Ammo_Pips(); }
    bool Is_Capable_Of_Following_Waypoint() const { return m_template->Is_Capable_Of_Following_Waypoint(); }
    bool Is_Contact_Weapon() const { return m_template->Is_Contact_Weapon(); }
    bool Is_Auto_Reloads_Clip() const { return m_template->Is_Auto_Reloads_Clip(); }
    bool Is_Leech_Weapon_Range_Active() const { return m_leechWeaponRangeActive; }

    void Set_Leech_Range_Active(bool set) { m_leechWeaponRangeActive = set; }
    void Set_Next_Shot(unsigned int time) { m_whenWeCanFireAgain = time; }
    void Set_Status(WeaponStatus status) { m_status = status; }
    void Set_Pre_Attack_Finished(unsigned int set) { m_whenPreAttackFinished = set; }
    void Set_Max_Shot_Count(int count) { m_maxShotCount = count; }

    bool Has_Laser() const { return m_template->Get_Laser_Name().Is_Not_Empty(); }

    float Estimate_Weapon_Damage(const Object *source_obj, const Object *victim_obj)
    {
        return Estimate_Weapon_Damage(source_obj, victim_obj, nullptr);
    }

#ifdef GAME_DLL
    Weapon *Hook_Ctor(const WeaponTemplate *tmpl, WeaponSlotType wslot) { return new (this) Weapon(tmpl, wslot); }
#endif

private:
    const WeaponTemplate *m_template;
    WeaponSlotType m_wslot;
    mutable WeaponStatus m_status;
    unsigned int m_ammoInClip;
    unsigned int m_whenWeCanFireAgain;
    unsigned int m_whenPreAttackFinished;
    unsigned int m_whenLastReloadStarted;
    unsigned int m_lastFireFrame;
    unsigned int m_suspendFXDelay;
    ObjectID m_projectileStreamID;
    int m_maxShotCount;
    int m_curBarrel;
    int m_numShotsForCurBarrel;
    std::vector<int> m_scatterTargetIndexes;
    bool m_pitchLimited;
    bool m_leechWeaponRangeActive;
};

class WeaponStore : public SubsystemInterface
{
public:
    struct WeaponDelayedDamageInfo
    {
        const WeaponTemplate *m_delayedWeapon;
        Coord3D m_delayDamagePos;
        unsigned int m_delayDamageFrame;
        ObjectID m_delaySourceID;
        ObjectID m_delayIntendedVictimID;
        WeaponBonus m_bonus;
    };

#ifdef GAME_DLL
    WeaponStore *Hook_Ctor() { return new (this) WeaponStore(); }
#endif

    WeaponStore() {}

    virtual ~WeaponStore() override;
    virtual void Init() override {}
    virtual void PostProcessLoad() override;
    virtual void Reset() override;
    virtual void Update() override;

    void Create_And_Fire_Temp_Weapon(const WeaponTemplate *tmplate, const Object *source_obj, const Coord3D *victim_pos);
    const WeaponTemplate *Find_Weapon_Template(Utf8String name) const;
    WeaponTemplate *Find_Weapon_Template_Private(NameKeyType key) const;
    Weapon *Allocate_New_Weapon(const WeaponTemplate *tmpl, WeaponSlotType wslot) const;
    WeaponTemplate *New_Weapon_Template(Utf8String name);
    WeaponTemplate *New_Override(WeaponTemplate *tmplate);
    void Delete_All_Delayed_Damage();
    void Reset_Weapon_Templates();

    void Handle_Projectile_Detonation(const WeaponTemplate *tmplate,
        const Object *source_obj,
        const Coord3D *victim_pos,
        int bonus_condition,
        bool do_damage) const;

    void Set_Delayed_Damage(const WeaponTemplate *weapon,
        const Coord3D *pos,
        unsigned int which_frame,
        ObjectID source_id,
        ObjectID victim_id,
        const WeaponBonus &bonus);

    static void Parse_Weapon_Template(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Weapon_Template_Definition(INI *ini);

private:
    std::vector<WeaponTemplate *> m_weaponTemplateVector;
    std::list<WeaponDelayedDamageInfo> m_weaponDDI;
};

#ifdef GAME_DLL
extern WeaponStore *&g_theWeaponStore;
#else
extern WeaponStore *g_theWeaponStore;
#endif
