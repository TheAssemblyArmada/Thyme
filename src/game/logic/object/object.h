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
#pragma once

#include "always.h"
#include "armortemplateset.h"
#include "bitflags.h"
#include "disabledtypes.h"
#include "gametype.h"
#include "geometry.h"
#include "partitionmanager.h"
#include "scriptparam.h"
#include "snapshot.h"
#include "thing.h"
#include "weapon.h"
#include "weaponset.h"
#include "weapontemplateset.h"

class AIGroup;
class AIUpdateInterface;
class BehaviorModule;
class BodyModuleInterface;
class CommandButton;
class ContainModuleInterface;
class CountermeasuresBehaviorInterface;
class DamageInfo;
class DamageInfoInput;
class DamageModule;
class DieModule;
class DockUpdateInterface;
class ExitInterface;
class ExperienceTracker;
class FiringTracker;
class Module;
class ObjectDefectionHelper;
class ObjectRepulsorHelper;
class ObjectSMCHelper;
class ObjectWeaponStatusHelper;
class PartitionData;
class PhysicsBehavior;
class Player;
class PolygonTrigger;
class ProductionUpdateInterface;
class ProjectileUpdateInterface;
class RadarObject;
class SightingInfo;
class SpawnBehaviorInterface;
class SpecialAbilityUpdate;
class SpecialPowerCompletionDie;
class SpecialPowerModuleInterface;
class SpecialPowerTemplate;
class SpecialPowerUpdateInterface;
class StatusDamageHelper;
class StealthUpdate;
class SubdualDamageHelper;
class Team;
class TempWeaponBonusHelper;
class UpdateModule;
class UpgradeTemplate;
class Waypoint;

enum FormationID : int32_t
{
    INVALID_FORMATION_ID,
};

struct TTriggerInfo
{
    PolygonTrigger *polygon_trigger;
    bool entered;
    bool exited;
    bool inside;
};

enum WhichTurretType
{
    TURRET_INVALID = -1,
    TURRET_MAIN = 0,
    TURRET_ALT,
    MAX_TURRETS,
};

enum PrivateStatus
{
    STATUS_EFFECTIVELY_DEAD = 1 << 0,
    STATUS_UNDETECTED_DEFECTOR = 1 << 1,
    STATUS_CAPTURED = 1 << 2,
    STATUS_OUTSIDE_MAP = 1 << 3,
};

enum ObjectScriptStatusBit
{
    STATUS_ENABLED = 1 << 0,
    STATUS_POWERED = 1 << 1,
    STATUS_UNSELLABLE = 1 << 2,
    STATUS_STEALTH = 1 << 3,
    STATUS_TARGETABLE = 1 << 4,
};

enum SpecialPowerType
{
    SPECIAL_INVALID,
    SPECIAL_DAISY_CUTTER,
    SPECIAL_PARADROP_AMERICA,
    SPECIAL_CARPET_BOMB,
    SPECIAL_CLUSTER_MINES,
    SPECIAL_EMP_PULSE,
    SPECIAL_NAPALM_STRIKE,
    SPECIAL_CASH_HACK,
    SPECIAL_NEUTRON_MISSILE,
    SPECIAL_SPY_SATELLITE,
    SPECIAL_DEFECTOR,
    SPECIAL_TERROR_CELL,
    SPECIAL_AMBUSH,
    SPECIAL_BLACK_MARKET_NUKE,
    SPECIAL_ANTHRAX_BOMB,
    SPECIAL_SCUD_STORM,
    SPECIAL_DEMORALIZE_OBSOLETE,
    SPECIAL_CRATE_DROP,
    SPECIAL_A10_THUNDERBOLT_STRIKE,
    SPECIAL_DETONATE_DIRTY_NUKE,
    SPECIAL_ARTILLERY_BARRAGE,
    SPECIAL_MISSILE_DEFENDER_LASER_GUIDED_MISSILE,
    SPECIAL_REMOTE_CHARGES,
    SPECIAL_TIMED_CHARGES,
    SPECIAL_HELIX_NAPALM_BOMB,
    SPECIAL_HACKER_DISABLE_BUILDING,
    SPECIAL_TANKHUNTER_TNT_ATTACK,
    SPECIAL_BLACKLOTUS_CAPTURE_BUILDING,
    SPECIAL_BLACKLOTUS_DISABLE_VEHICLE_HACK,
    SPECIAL_BLACKLOTUS_STEAL_CASH_HACK,
    SPECIAL_INFANTRY_CAPTURE_BUILDING,
    SPECIAL_RADAR_VAN_SCAN,
    SPECIAL_SPY_DRONE,
    SPECIAL_DISGUISE_AS_VEHICLE,
    SPECIAL_BOOBY_TRAP,
    SPECIAL_REPAIR_VEHICLES,
    SPECIAL_PARTICLE_UPLINK_CANNON,
    SPECIAL_CASH_BOUNTY,
    SPECIAL_CHANGE_BATTLE_PLANS,
    SPECIAL_CIA_INTELLIGENCE,
    SPECIAL_CLEANUP_AREA,
    SPECIAL_LAUNCH_BAIKONUR_ROCKET,
    SPECIAL_SPECTRE_GUNSHIP,
    SPECIAL_GPS_SCRAMBLER,
    SPECIAL_FRENZY,
    SPECIAL_SNEAK_ATTACK,
    SPECIAL_CHINA_CARPET_BOMB,
    EARLY_SPECIAL_CHINA_CARPET_BOMB,
    SPECIAL_LEAFLET_DROP,
    EARLY_SPECIAL_LEAFLET_DROP,
    EARLY_SPECIAL_FRENZY,
    SPECIAL_COMMUNICATIONS_DOWNLOAD,
    EARLY_SPECIAL_REPAIR_VEHICLES,
    SPECIAL_TANK_PARADROP,
    SUPW_SPECIAL_PARTICLE_UPLINK_CANNON,
    AIRF_SPECIAL_DAISY_CUTTER,
    NUKE_SPECIAL_CLUSTER_MINES,
    NUKE_SPECIAL_NEUTRON_MISSILE,
    AIRF_SPECIAL_A10_THUNDERBOLT_STRIKE,
    AIRF_SPECIAL_SPECTRE_GUNSHIP,
    INFA_SPECIAL_PARADROP_AMERICA,
    SLTH_SPECIAL_GPS_SCRAMBLER,
    AIRF_SPECIAL_CARPET_BOMB,
    SUPR_SPECIAL_CRUISE_MISSILE,
    LAZR_SPECIAL_PARTICLE_UPLINK_CANNON,
    SUPW_SPECIAL_NEUTRON_MISSILE,
    SPECIAL_BATTLESHIP_BOMBARDMENT,
    SPECIAL_POWER_COUNT,
};

enum CrushSquishTestType
{
    TEST_TYPE_0 = 0,
    TEST_TYPE_1,
    TEST_TYPE_2,
};

class Object : public Thing, public SnapShot
{
    IMPLEMENT_NAMED_POOL(Object, ObjectPool)

public:
    enum
    {
        DISABLED_MANUALLY = 0xff,
    };

    enum
    {
        MAX_TRIGGER_AREA_INFOS = 5
    };
    Object(const ThingTemplate *tt, BitFlags<OBJECT_STATUS_COUNT> status_bits, Team *team);
    virtual ~Object() override;

    virtual float Calculate_Height_Above_Terrain() const override;
    virtual Object *As_Object_Meth() override { return this; }
    virtual const Object *As_Object_Meth() const override { return this; }

    virtual void React_To_Transform_Change(const Matrix3D *tm, const Coord3D *pos, float angle) override;
    virtual void React_To_Turret(WhichTurretType turret, float angle, float pitch);

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    ObjectID Get_ID() const { return m_id; }
    bool Get_Status(ObjectStatusTypes status) const { return m_status.Test(status); }
    float Get_Construction_Percent() const { return m_constructionPercent; }
    AIUpdateInterface *Get_AI_Update_Interface() { return m_ai; }
    const AIUpdateInterface *Get_AI_Update_Interface() const { return m_ai; }
    PathfindLayerEnum Get_Layer() const { return m_layer; }
    const GeometryInfo &Get_Geometry_Info() const { return m_geometryInfo; }
    Drawable *Get_Drawable() const { return m_drawable; }
    Utf8String Get_Name() const { return m_name; }
    const PhysicsBehavior *Get_Physics() const { return m_physics; }
    PhysicsBehavior *Get_Physics() { return m_physics; }
    ContainModuleInterface *Get_Contain() const { return m_contain; }
    const Object *Get_Contained_By() const { return m_containedBy; }
    Object *Get_Contained_By() { return m_containedBy; }
    BehaviorModule **Get_All_Modules() const { return m_allModules; }
    ObjectID Get_Producer_ID() const { return m_producerID; }
    unsigned int Get_Weapon_Bonus_Condition() const { return m_weaponBonusCondition; }
    const Weapon *Get_Weapon_In_Weapon_Slot(WeaponSlotType type) const
    {
        return m_weaponSet.Get_Weapon_In_Weapon_Slot(type);
    }
    const ExperienceTracker *Get_Experience_Tracker() const { return m_experienceTracker; }
    ExperienceTracker *Get_Experience_Tracker() { return m_experienceTracker; }
    const BitFlags<WEAPONSET_COUNT> &Get_Weapon_Set_Flags() const { return m_curWeaponSetFlags; }
    bool Test_Weapon_Set_Flag(WeaponSetType type) const { return m_curWeaponSetFlags.Test(type); }
    const Team *Get_Team() const { return m_team; }
    Team *Get_Team() { return m_team; }
    StealthUpdate *Get_Stealth_Update() const { return m_stealth; }
    bool Get_Script_Status(ObjectScriptStatusBit status) const { return (m_scriptStatus & status) != 0; }
    Object *Get_Next_Object() { return m_next; }
    const BitFlags<OBJECT_STATUS_COUNT> &Get_Status_Bits() const { return m_status; }
    BodyModuleInterface *Get_Body_Module() const { return m_body; }
    bool Get_Disabled_State(DisabledType type) const { return m_disabledStates.Test(type); }
    const BitFlags<DISABLED_TYPE_COUNT> &Get_Disabled_State_Bits() const { return m_disabledStates; }
    bool Get_Receiving_Difficulty_Bonus() { return m_receivingDifficultyBonus; }
    BitFlags<128> Get_Object_Upgrade_Mask() const { return m_objectUpgradesCompleted; }
    bool Get_Apply_Battle_Plan_Bonuses() const { return m_applyBattlePlanBonuses; }
    PathfindLayerEnum Get_Destination_Layer() const { return m_destinationLayer; }
    void Get_Formation_Offset(Coord2D *c) { *c = m_formationOffset; }
    bool Get_Single_Use_Command() const { return m_singleUseCommand; }
    unsigned int Get_Occlusion_Delay_Frame() const { return m_occlusionDelayFrame; }
    FormationID Get_Formation_ID() const { return m_formationID; }
    PartitionData *Get_Partition_Data() const { return m_partitionData; }
    unsigned int Get_Contained_By_Frame() const { return m_containedByFrame; }

    void Clear_Status(BitFlags<OBJECT_STATUS_COUNT> bits) { return Set_Status(bits, false); }
    void Clear_Script_Status(ObjectScriptStatusBit bit) { Set_Script_Status(bit, false); }

    BehaviorModule *Find_Module(NameKeyType type) const;
    DamageModule *Find_Damage_Module(NameKeyType type) const;
    UpdateModule *Find_Update_Module(NameKeyType type) const;

    bool Is_Effectively_Dead() const { return (m_privateStatus & STATUS_EFFECTIVELY_DEAD) != 0; }
    bool Is_Undetected_Defector() const { return (m_privateStatus & STATUS_UNDETECTED_DEFECTOR) != 0; }
    bool Is_Airborne_Target() const { return m_status.Test(OBJECT_STATUS_AIRBORNE_TARGET); }
    bool Is_Share_Weapon_Reload_Time() const { return m_weaponSet.Is_Share_Weapon_Reload_Time(); }
    bool Is_Destroyed() const { return m_status.Test(OBJECT_STATUS_DESTROYED); }
    bool Is_Outside_Map() const { return (m_privateStatus & STATUS_OUTSIDE_MAP) != 0; }
    bool Is_Captured() const { return (m_privateStatus & STATUS_CAPTURED) != 0; }
    bool Is_Disabled() const { return m_disabledStates.Any(); }
    bool Is_Contained() const { return m_containedBy != nullptr; }
    bool Is_Weapon_Locked() const { return m_weaponSet.Is_Cur_Weapon_Locked(); }

    bool Has_Custom_Indicator_Color() const { return m_customIndicatorColor != 0; }

    void Set_Formation_Offset(Coord2D *c) { m_formationOffset = *c; }
    void Set_Formation_ID(FormationID id) { m_formationID = id; }
    void Set_Single_Use_Command() { m_singleUseCommand = true; }
    void Set_Name(const Utf8String &name) { m_name = name; }
    void Set_Partition_Data(PartitionData *data) { m_partitionData = data; }
    void Set_Radar_Data(RadarObject *data) { m_radarData = data; }
    void Set_Construction_Percent(float percent) { m_constructionPercent = percent; }
    bool Set_Weapon_Lock(WeaponSlotType wslot, WeaponLockType lock) { return m_weaponSet.Set_Weapon_Lock(wslot, lock); }

    RadarObject *Friend_Get_Radar_Data() { return m_radarData; }
    void Delete() { Delete_Instance(); }

    void Release_Weapon_Lock(WeaponLockType type) { m_weaponSet.Release_Weapon_Lock(type); }

    bool Test_Weapon_Bonus_Condition(WeaponBonusConditionType type) const
    {
        return ((1 << type) & m_weaponBonusCondition) != 0;
    }

    int Get_Transport_Slot_Count() const;
    unsigned char Get_Crusher_Level() const;
    unsigned char Get_Crushable_Level() const;
    unsigned int Get_Most_Percent_Ready_To_Fire_Any_Weapon() const;
    float Get_Largest_Weapon_Range() const;
    const Weapon *Get_Current_Weapon(WeaponSlotType *wslot) const;
    Weapon *Get_Current_Weapon(WeaponSlotType *wslot);
    bool Get_Ammo_Pip_Showing_Info(int &clip_size, int &ammo_in_clip) const;
    CanAttackResult Get_Able_To_Attack_Specific_Object(
        AbleToAttackType type, const Object *obj, CommandSourceType source, WeaponSlotType slot) const;
    CanAttackResult Get_Able_To_Use_Weapon_Against_Target(
        AbleToAttackType type, const Object *obj, const Coord3D *pos, CommandSourceType source, WeaponSlotType slot) const;
    unsigned int Get_Last_Shot_Fired_Frame() const;
    ObjectID Get_Last_Victim_ID() const;
    Relationship Get_Relationship(const Object *that) const;
    Player *Get_Controlling_Player() const;
    int Get_Indicator_Color() const;
    int Get_Night_Indicator_Color() const;
    ObjectShroudStatus Get_Shrouded_Status(int index) const;
    ObjectID Get_Sole_Healing_Benefactor() const;
    unsigned int Get_Disabled_Until(DisabledType type) const;
    ExitInterface *Get_Object_Exit_Interface() const;
    VeterancyLevel Get_Veterancy_Level() const;
    void Get_Health_Box_Position(Coord3D &pos) const;
    bool Get_Health_Box_Dimensions(float &width, float &height) const;
    float Get_Vision_Range() const;
    float Get_Shroud_Clearing_Range() const;
    float Get_Shroud_Range() const;
    SpecialPowerModuleInterface *Get_Special_Power_Module(const SpecialPowerTemplate *t) const;
    ProductionUpdateInterface *Get_Production_Update_Interface();
    DockUpdateInterface *Get_Dock_Update_Interface();
    SpawnBehaviorInterface *Get_Spawn_Behavior_Interface() const;
    ProjectileUpdateInterface *Get_Projectile_Update_Interface() const;
    int Get_Num_Consecutive_Shots_Fired_At_Target(const Object *target) const;
    bool Get_Single_Logical_Bone_Position(const char *bone, Coord3D *pos, Matrix3D *tm) const;
    bool Get_Single_Logical_Bone_Position_On_Turret(
        WhichTurretType type, const char *bone, Coord3D *pos, Matrix3D *tm) const;
    int Get_Multi_Logical_Bone_Position(const char *bone, int max_count, Coord3D *pos, Matrix3D *tm, bool world_space) const;
    const Utf8String &Get_Command_Set_String() const;
    RadarPriorityType Get_Radar_Priority() const;
    AIGroup *Get_Group();
    float Get_Carrier_Deck_Height() const;
    CountermeasuresBehaviorInterface *Get_Countermeasures_Behavior_Interface();
    const CountermeasuresBehaviorInterface *Get_Countermeasures_Behavior_Interface() const;

    Weapon *Find_Waypoint_Following_Capable_Weapon();
    SpecialPowerModuleInterface *Find_Special_Power_Module_Interface(SpecialPowerType type) const;
    SpecialPowerModuleInterface *Find_Any_Shortcut_Special_Power_Module_Interface() const;
    SpecialPowerUpdateInterface *Find_Special_Power_With_Overridable_Destination_Active(SpecialPowerType type) const;
    SpecialPowerUpdateInterface *Find_Special_Power_With_Overridable_Destination(SpecialPowerType type) const;
    SpecialAbilityUpdate *Find_Special_Ability_Update(SpecialPowerType type) const;
    SpecialPowerCompletionDie *Find_Special_Power_Completion_Die() const;

    void Set_Geometry_Info(const GeometryInfo &geom);
    void Set_Geometry_Info_Z(float setz);
    void Set_Team(Team *team);
    void Set_Temporary_Team(Team *team);
    void Set_Or_Restore_Team(Team *team, bool b);
    void Set_Status(BitFlags<OBJECT_STATUS_COUNT> bits, bool set);
    void Set_Script_Status(ObjectScriptStatusBit bit, bool set);
    void Set_Armor_Set_Flag(ArmorSetType type);
    void Set_Firing_Condition_For_Current_Weapon() const;
    void Set_Model_Condition_State(ModelConditionFlagType a);
    void Set_Model_Condition_Flags(const BitFlags<MODELCONDITION_COUNT> &set);
    void Set_Special_Model_Condition_State(ModelConditionFlagType type, unsigned int i);
    void Set_Producer(const Object *obj);
    void Set_Builder(const Object *obj);
    void Set_Custom_Indicator_Color(int color);
    void Set_Effectively_Dead(bool dead);
    void Set_Captured(bool captured);
    void Set_Receiving_Difficulty_Bonus(bool bonus);
    void Set_Disabled(DisabledType type);
    void Set_Disabled_Until(DisabledType type, unsigned int frame);
    void Set_Trigger_Area_Flags_For_Change_In_Position();
    void Set_Layer(PathfindLayerEnum layer);
    void Set_Destination_Layer(PathfindLayerEnum layer);
    void Set_ID(ObjectID id);
    void Set_Selectable(bool selectable);
    void Set_Weapon_Set_Flag(WeaponSetType wst);
    void Set_Weapon_Bonus_Condition(WeaponBonusConditionType bonus);
    void Set_Vision_Range(float range);
    void Set_Shroud_Clearing_Range(float range);
    void Set_Shroud_Range(float range);
    void Set_Vision_Spied(bool vision_spied, int player_index);

    bool Is_Out_Of_Ammo() const;
    bool Is_Locally_Controlled() const;
    bool Is_Neutral_Controlled() const;
    bool Is_Structure() const;
    bool Is_Faction_Structure() const;
    bool Is_Non_Faction_Structure() const;
    bool Is_Hero() const;
    bool Is_Salvage_Crate() const;
    bool Is_Inside(const PolygonTrigger *trigger) const;
    bool Is_In_List(Object **list) const; // Returns true if this object is in any list, or head of argument list.
    bool Is_Mobile() const;
    bool Is_Selectable() const;
    bool Is_Mass_Selectable() const;
    bool Is_Able_To_Attack() const;
    bool Is_Using_Airborne_Locomotor() const;

    void Clear_Armor_Set_Flag(ArmorSetType type);
    void Clear_Model_Condition_State(ModelConditionFlagType a);
    void Clear_And_Set_Model_Condition_State(ModelConditionFlagType clr, ModelConditionFlagType set);
    void Clear_Model_Condition_Flags(const BitFlags<MODELCONDITION_COUNT> &clr);
    void Clear_And_Set_Model_Condition_Flags(
        const BitFlags<MODELCONDITION_COUNT> &clr, const BitFlags<MODELCONDITION_COUNT> &set);
    void Clear_Special_Model_Condition_States();
    bool Clear_Disabled(DisabledType type);
    void Clear_Weapon_Set_Flag(WeaponSetType wst);
    void Clear_Weapon_Bonus_Condition(WeaponBonusConditionType bonus);
    void Clear_Leech_Range_Mode_For_All_Weapons();

    void Do_Status_Damage(ObjectStatusTypes status, float damage);
    void Do_Temp_Weapon_Bonus(WeaponBonusConditionType type, unsigned int frame);
    void Do_Special_Power(const SpecialPowerTemplate *special_power_template, unsigned int options, bool force_usable);
    void Do_Special_Power_At_Object(
        const SpecialPowerTemplate *special_power_template, Object *obj, unsigned int options, bool force_usable);
    void Do_Special_Power_At_Location(const SpecialPowerTemplate *special_power_template,
        const Coord3D *loc,
        float f,
        unsigned int options,
        bool force_usable);
    void Do_Special_Power_Using_Waypoints(
        const SpecialPowerTemplate *special_power_template, const Waypoint *wp, unsigned int options, bool force_usable);
    void Do_Command_Button(const CommandButton *button, CommandSourceType type);
    void Do_Command_Button_At_Object(const CommandButton *button, Object *obj, CommandSourceType type);
    void Do_Command_Button_At_Position(const CommandButton *button, const Coord3D *pos, CommandSourceType type);
    void Do_Command_Button_Using_Waypoints(const CommandButton *button, const Waypoint *wp, CommandSourceType type);

    bool Did_Enter_Or_Exit() const;
    bool Did_Enter(const PolygonTrigger *trigger) const;
    bool Did_Exit(const PolygonTrigger *trigger) const;

    void Fire_Current_Weapon(const Coord3D *pos);
    void Fire_Current_Weapon(Object *target);
    void Pre_Fire_Current_Weapon(const Object *target);

    void On_Contained_By(Object *contained);
    void On_Removed_From(Object *removed);
    void On_Destroy();
    void On_Collide(Object *other, const Coord3D *loc, const Coord3D *normal);
    void On_Veterancy_Level_Changed(VeterancyLevel old_level, VeterancyLevel new_level, bool play_sound);
    void On_Disabled_Edge(bool edge);
    void On_Capture(Player *o, Player *n);
    void On_Die(DamageInfo *damage);
    void On_Partition_Cell_Change();

    bool Has_Any_Weapon() const;
    bool Has_Any_Damage_Weapon() const;
    bool Has_Weapon_To_Deal_Damage_Type(DamageType type) const;
    bool Has_Special_Power(SpecialPowerType spt) const;
    bool Has_Any_Special_Power() const;
    bool Has_Upgrade(const UpgradeTemplate *upgrade) const;
    bool Has_Countermeasures() const;

    void Friend_Set_Undetected_Defector(bool set);
    void Friend_Prepare_For_Map_Boundary_Adjust();
    void Friend_Notify_Of_New_Map_Boundary();
    void Friend_Bind_To_Drawable(Drawable *drawable);
    void Friend_Adjust_Power_For_Player(bool power);

    void Prepend_To_List(Object **list);
    void Remove_From_List(Object **list);

    void Handle_Shroud();
    void Handle_Value_Map();
    void Handle_Threat_Map();

    void Add_Value();
    void Remove_Value();
    void Add_Threat();
    void Remove_Threat();

    void Look();
    void Unlook();
    void Shroud();
    void Unshroud();

    void Attempt_Damage(DamageInfo *info);
    void Attempt_Healing(float amount, const Object *obj);
    bool Attempt_Healing_From_Sole_Benefactor(float amount, const Object *obj, unsigned int frame);

    bool Affected_By_Upgrade(const UpgradeTemplate *upgrade) const;
    void Update_Upgrade_Modules();
    void Force_Refresh_Sub_Object_Upgrade_Status();
    void Give_Upgrade(const UpgradeTemplate *upgrade);
    void Remove_Upgrade(const UpgradeTemplate *upgrade);
    bool Can_Produce_Upgrade(const UpgradeTemplate *upgrade);

    void Adjust_Model_Condition_For_Weapon_Status();

    void Report_Missile_For_Countermeasures(Object *obj);
    ObjectID Calculate_Countermeasure_To_Divert_To(const Object &obj);

    void Enter_Group(AIGroup *group);
    void Leave_Group();

    void Restore_Original_Team();
    void Defect(Team *team, unsigned int timer);

    void Reload_All_Ammo(bool now);
    bool Choose_Best_Weapon_For_Target(const Object *target, WeaponChoiceCriteria criteria, CommandSourceType source);
    void Notify_Firing_Tracker_Shot_Fired(const Weapon *weapon, ObjectID id);

    float Estimate_Damage(DamageInfoInput &info) const;
    void Kill(DamageType damage, DeathType death);
    void Score_The_Kill(const Object *victim);
    void Notify_Subdual_Damage(float damage);
    bool Can_Crush_Or_Squish(Object *obj, CrushSquishTestType type);
    void Heal_Completely();
    void Go_Invulnerable(unsigned int timer);

    bool Check_And_Detonate_Booby_Trap(Object *obj);
    void Check_Disabled_Status();

    void Init_Object();
    void Topple(const Coord3D *dir, float speed, int options);
    bool Test_Armor_Set_Flag(ArmorSetType type) const;
    void Remove_Custom_Indicator_Color();
    void Pause_All_Special_Powers(bool pause);
    void Update_Trigger_Area_Flags();
    void Calc_Natural_Rally_Point(Coord2D *pt);
    void Mask_Object(bool mask);
    void Update_Obj_Values_From_Map_Properties(Dict *properties);
    void Handle_Partition_Cell_Maintenance();

    bool DLink_Is_In_List_Team_Member_List(Object *const *head) const
    {
        captainslog_dbgassert(((uintptr_t)head & 1) == 0 && ((uintptr_t)m_dlink_TeamMemberList.m_next & 1) == 0
                && ((uintptr_t)m_dlink_TeamMemberList.m_prev & 1) == 0,
            "bogus ptrs");
        return *head == this || m_dlink_TeamMemberList.m_prev != nullptr || m_dlink_TeamMemberList.m_next != nullptr;
    }

    void DLink_Prepend_To_Team_Member_List(Object **head)
    {
        captainslog_dbgassert(!DLink_Is_In_List_Team_Member_List(head), "already in listTeamMemberList");
        captainslog_dbgassert(((uintptr_t)head & 1) == 0 && ((uintptr_t)m_dlink_TeamMemberList.m_next & 1) == 0
                && ((uintptr_t)m_dlink_TeamMemberList.m_prev & 1) == 0,
            "bogus ptrs");
        m_dlink_TeamMemberList.m_next = *head;

        if (*head != nullptr) {
            (*head)->m_dlink_TeamMemberList.m_prev = this;
        }

        *head = this;
        captainslog_dbgassert(((uintptr_t)head & 1) == 0 && ((uintptr_t)m_dlink_TeamMemberList.m_next & 1) == 0
                && ((uintptr_t)m_dlink_TeamMemberList.m_prev & 1) == 0,
            "bogus ptrs");
    }

    void DLink_Remove_From_Team_Member_List(Object **head)
    {
        captainslog_dbgassert(DLink_Is_In_List_Team_Member_List(head), "not in listTeamMemberList");
        captainslog_dbgassert(((uintptr_t)head & 1) == 0 && ((uintptr_t)m_dlink_TeamMemberList.m_next & 1) == 0
                && ((uintptr_t)m_dlink_TeamMemberList.m_prev & 1) == 0,
            "bogus ptrs");

        if (m_dlink_TeamMemberList.m_next != nullptr) {
            m_dlink_TeamMemberList.m_next->m_dlink_TeamMemberList.m_prev = m_dlink_TeamMemberList.m_prev;
        }

        if (m_dlink_TeamMemberList.m_prev != nullptr) {
            m_dlink_TeamMemberList.m_prev->m_dlink_TeamMemberList.m_next = m_dlink_TeamMemberList.m_next;
        } else {
            *head = m_dlink_TeamMemberList.m_next;
        }

        m_dlink_TeamMemberList.m_prev = nullptr;
        m_dlink_TeamMemberList.m_next = nullptr;
        captainslog_dbgassert(((uintptr_t)head & 1) == 0 && ((uintptr_t)m_dlink_TeamMemberList.m_next & 1) == 0
                && ((uintptr_t)m_dlink_TeamMemberList.m_prev & 1) == 0,
            "bogus ptrs");
    }

    Object *Dlink_Next_Team_Member_List() const { return m_dlink_TeamMemberList.m_next; }

private:
    struct DLINK_TeamMemberList
    {
        Object *m_prev;
        Object *m_next;
        DLINK_TeamMemberList() : m_prev(nullptr), m_next(nullptr) {}
        ~DLINK_TeamMemberList()
        {
            captainslog_dbgassert(!m_prev && !m_next, "destroying dlink still in a list TeamMemberList");
        }
    };

    ObjectID m_id;
    ObjectID m_producerID;
    ObjectID m_builderID;
    Drawable *m_drawable;
    Utf8String m_name;
    Object *m_next;
    Object *m_prev;
    BitFlags<OBJECT_STATUS_COUNT> m_status;
    GeometryInfo m_geometryInfo;
    AIGroup *m_aiGroup;
    SightingInfo *m_friendlyLookSighting;
    SightingInfo *m_allLookSighting;
    unsigned int m_spiedOnByPlayer[16];
    unsigned short m_spiedOnPlayers;
    SightingInfo *m_shroudSighting;
    SightingInfo *m_threatSighting;
    SightingInfo *m_valueSighting;
    float m_visionRange;
    float m_shroudClearingRange;
    float m_shroudRange;
    BitFlags<DISABLED_TYPE_COUNT> m_disabledStates;
    unsigned int m_disabledStateFrames[DISABLED_TYPE_COUNT];
    unsigned int m_specialModelConditionSleepFrame;
    ObjectRepulsorHelper *m_objectRepulsorHelper;
    ObjectSMCHelper *m_objectSMCHelper;
    ObjectWeaponStatusHelper *m_objectWeaponStatusHelper;
    ObjectDefectionHelper *m_objectDefectionHelper;
    StatusDamageHelper *m_statusDamageHelper;
    SubdualDamageHelper *m_subdualDamageHelper;
    TempWeaponBonusHelper *m_tempWeaponBonusHelper;
    FiringTracker *m_firingTracker;
    BehaviorModule **m_allModules;
    ContainModuleInterface *m_contain;
    BodyModuleInterface *m_body;
    StealthUpdate *m_stealth;
    AIUpdateInterface *m_ai;
    PhysicsBehavior *m_physics;
    PartitionData *m_partitionData;
    RadarObject *m_radarData;
    ExperienceTracker *m_experienceTracker;
    Object *m_containedBy;
    ObjectID m_containedByID;
    unsigned int m_containedByFrame;
    float m_constructionPercent;
    BitFlags<128> m_objectUpgradesCompleted;
    Team *m_team;
    Utf8String m_originalTeamName;
    int m_customIndicatorColor;
    Coord3D m_healthBoxOffset;
    DLINK_TeamMemberList m_dlink_TeamMemberList;
    WeaponSet m_weaponSet;
    BitFlags<WEAPONSET_COUNT> m_curWeaponSetFlags;
    unsigned int m_weaponBonusCondition;
    char m_weaponSetConditions[WEAPONSLOT_COUNT];
    BitFlags<SPECIAL_POWER_COUNT> m_specialPowers;
    ObjectID m_soleHealingBenefactor;
    unsigned int m_soleHealingEndFrame;
    TTriggerInfo m_triggerInfo[MAX_TRIGGER_AREA_INFOS];
    unsigned int m_enteredOrExited;
    ICoord3D m_iPos;
    PathfindLayerEnum m_layer;
    PathfindLayerEnum m_destinationLayer;
    FormationID m_formationID;
    Coord2D m_formationOffset;
    Utf8String m_commandSetStringOverride;
    unsigned int m_occlusionDelayFrame;
    bool m_isSelectable;
    bool m_applyBattlePlanBonuses;
#ifdef GAME_DEBUG_STRUCTS
    bool m_hasDiedAlready;
#endif
    unsigned char m_scriptStatus;
    unsigned char m_privateStatus;
    signed char m_numTriggerAreasActive;
    bool m_singleUseCommand;
    bool m_receivingDifficultyBonus;
};

extern BitFlags<OBJECT_STATUS_COUNT> OBJECT_STATUS_MASK_NONE;
