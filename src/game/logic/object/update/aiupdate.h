/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief AI update
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
#include "aistates.h"
#include "damage.h"
#include "locomotor.h"
#include "object.h"
#include "updatemodule.h"

class INI;
class DozerAIInterface;
class WorkerAIInterface;
class JetAIUpdate;
class AIStateMachine;
class Path;
class TurretAIData;
class TurretAI;
class AttackPriorityInfo;

class HackInternetAIInterface
{
public:
    virtual bool Is_Hacking() const = 0;
    virtual bool Is_Hacking_Packing_Or_Unpacking() const = 0;
};

enum AICommandType
{
    AICMD_MOVE_TO_POSITION,
    AICMD_MOVE_TO_OBJECT,
    AICMD_TIGHTEN_TO_POSITION,
    AICMD_MOVE_TO_POSITION_AND_EVACUATE,
    AICMD_MOVE_TO_POSITION_AND_EVACUATE_AND_EXIT,
    AICMD_IDLE,
    AICMD_FOLLOW_WAYPOINT_PATH,
    AICMD_FOLLOW_WAYPOINT_PATH_AS_TEAM,
    AICMD_UNK8,
    AICMD_FOLLOW_PATH,
    AICMD_FOLLOW_EXITPRODUCTION_PATH,
    AICMD_ATTACK_OBJECT,
    AICMD_FORCE_ATTACK_OBJECT,
    AICMD_ATTACK_TEAM,
    AICMD_ATTACK_POSITION,
    AICMD_ATTACKMOVE_TO_POSITION,
    AICMD_ATTACKFOLLOW_WAYPOINT_PATH,
    AICMD_ATTACKFOLLOW_WAYPOINT_PATH_AS_TEAM,
    AICMD_HUNT,
    AICMD_REPAIR,
    AICMD_RESUME_CONSTRUCTION,
    AICMD_GET_HEALED,
    AICMD_GET_REPAIRED,
    AICMD_ENTER,
    AICMD_DOCK,
    AICMD_EXIT,
    AICMD_EVACUATE,
    AICMD_EXECUTE_RAILED_TRANSPORT,
    AICMD_GO_PRONE,
    AICMD_GUARD_POSITION,
    AICMD_GUARD_OBJECT,
    AICMD_GUARD_AREA,
    AICMD_DEPLOY_ASSAULT_RETURN,
    AICMD_ATTACK_AREA,
    AICMD_HACK_INTERNET,
    AICMD_FACE_OBJECT,
    AICMD_FACE_POSITION,
    AICMD_RAPPEL_INTO,
    AICMD_COMBAT_DROP,
    AICMD_COMMAND_BUTTON_POSITION,
    AICMD_COMMAND_BUTTON_OBJECT,
    AICMD_COMMAND_BUTTON,
    AICMD_WANDER,
    AICMD_WANDER_IN_PLACE,
    AICMD_PANIC,
    AICMD_BUSY,
    AICMD_FOLLOW_WAYPOINT_PATH_EXACT,
    AICMD_FOLLOW_WAYPOINT_PATH_AS_TEAM_EXACT,
    AICMD_MOVE_AWAY_FROM_UNIT,
    AICMD_FOLLOW_WAYPATH_APPEND,
    AICMD_UNK50,
    AICMD_GUARD_TUNNEL_NETWORK,
    AICMD_EVACUATE_INSTANTLY,
    AICMD_EXIT_INSTANTLY,
    AICMD_GUARD_RETALIATE,
    AICMD_NUM_COMMANDS,
};

struct AICommandParms
{
    AICommandType m_cmd;
    CommandSourceType m_cmdSource;
    Coord3D m_pos;
    Object *m_obj;
    Object *m_otherObj;
    const Team *m_team;
    std::vector<Coord3D> m_coords;
    const Waypoint *m_waypoint;
    const PolygonTrigger *m_polygon;
    int m_intValue;
    DamageInfo m_damage;
    CommandButton *m_commandButton;
    Path *m_path;
    AICommandParms(AICommandType cmd, CommandSourceType cmd_source);
    ~AICommandParms();
};

enum GuardTargetType
{
    TARGET_UNK
};

enum AIFreeToExitType
{
    FREE_TO_EXIT_UNK
};

enum GuardMode
{
    GUARD_MODE_UNK
};

enum GuardType
{
    GUARD_TYPE_UNK
};

enum AttitudeType
{
    AI_SLEEP = -2,
    AI_PASSIVE = -1,
    AI_NORMAL = 0,
    AI_ALERT = 1,
    AI_AGGRESSIVE = 2,
    AI_INVALID = 3,
};

enum LocomotorSetType
{
    LOCOMOTORSET_INVALID = -1,
    LOCOMOTORSET_NORMAL = 0,
    LOCOMOTORSET_NORMAL_UPGRADED,
    LOCOMOTORSET_FREEFALL,
    LOCOMOTORSET_WANDER,
    LOCOMOTORSET_PANIC,
    LOCOMOTORSET_TAXIING,
    LOCOMOTORSET_SUPERSONIC,
    LOCOMOTORSET_SLUGGISH,
    LOCOMOTORSET_COUNT,
};

enum MoodMatrixAction
{
    MM_ACTION_IDLE,
    MM_ACTION_MOVE,
    MM_ACTION_ATTACK,
    MM_ACTION_ATTACKMOVE,
};

class SupplyTruckAIInterface
{
public:
    virtual int Get_Number_Boxes() const = 0;
    virtual bool Lose_One_Box() = 0;
    virtual bool Gain_One_Box(int i) = 0;
    virtual bool Is_Available_For_Supplying() const = 0;
    virtual bool Is_Currently_Ferrying_Supplies() const = 0;
    virtual float Get_Warehouse_Scan_Distance() const = 0;
    virtual void Set_Force_Wanting_State(bool wanting) = 0;
    virtual bool Is_Forced_Into_Wanting_State() const = 0;
    virtual void Set_Force_Busy_State(bool busy) = 0;
    virtual bool Is_Forced_Into_Busy_State() const = 0;
    virtual ObjectID Get_Preferred_Dock_ID() const = 0;
    virtual unsigned int Get_Action_Delay_For_Dock(Object *dock) = 0;
    virtual int Get_Upgraded_Supply_Boost() const = 0;
};

class AssaultTransportAIInterface
{
public:
    virtual void Begin_Assault(const Object *object) const = 0;
};

class AICommandInterface
{
public:
    AICommandInterface() {}

    virtual void AI_Do_Command(const AICommandParms *params) = 0;

    void AI_Hunt(CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_HUNT, cmd_source);
        AI_Do_Command(&params);
    }

    void AI_Idle(CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_IDLE, cmd_source);
        AI_Do_Command(&params);
    }

    void AI_Enter(Object *obj, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_ENTER, cmd_source);
        params.m_obj = obj;
        AI_Do_Command(&params);
    }

    void AI_Evacuate(bool expose_stealth_units, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_EVACUATE, cmd_source);
        params.m_intValue = expose_stealth_units;
        AI_Do_Command(&params);
    }

    void AI_Move_To_Position(const Coord3D *pos, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_MOVE_TO_POSITION, cmd_source);
        params.m_pos = *pos;
        AI_Do_Command(&params);
    }

    void AI_Attack_Position(const Coord3D *pos, int max_shots_to_fire, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_ATTACK_POSITION, cmd_source);
        params.m_pos = *pos;
        params.m_intValue = max_shots_to_fire;
        AI_Do_Command(&params);
    }

    void AI_Hack_Internet(CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_HACK_INTERNET, cmd_source);
        AI_Do_Command(&params);
    }

    void AI_Combat_Drop(Object *obj, const Coord3D &pos, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_COMBAT_DROP, cmd_source);
        params.m_obj = obj;
        params.m_pos = pos;
        AI_Do_Command(&params);
    }

    void AI_Attack_Object(Object *victim, int max_shots_to_fire, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_ATTACK_OBJECT, cmd_source);
        params.m_obj = victim;
        params.m_intValue = max_shots_to_fire;
        AI_Do_Command(&params);
    }

    void AI_Attack_Move_To_Position(const Coord3D *pos, int max_shots_to_fire, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_ATTACKMOVE_TO_POSITION, cmd_source);
        params.m_pos = *pos;
        params.m_intValue = max_shots_to_fire;
        AI_Do_Command(&params);
    }

    void AI_Attack_Area(const PolygonTrigger *area_to_guard, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_ATTACK_AREA, cmd_source);
        params.m_polygon = area_to_guard;
        AI_Do_Command(&params);
    }

    void AI_Attack_Follow_Waypoint_Path(const Waypoint *way, int max_shots_to_fire, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_ATTACKFOLLOW_WAYPOINT_PATH, cmd_source);
        params.m_waypoint = way;
        params.m_intValue = max_shots_to_fire;
        AI_Do_Command(&params);
    }

    void AI_Attack_Follow_Waypoint_Path_As_Team(const Waypoint *way, int max_shots_to_fire, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_ATTACKFOLLOW_WAYPOINT_PATH_AS_TEAM, cmd_source);
        params.m_waypoint = way;
        params.m_intValue = max_shots_to_fire;
        AI_Do_Command(&params);
    }

    void AI_Attack_Team(const Team *team, int max_shots_to_fire, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_ATTACK_TEAM, cmd_source);
        params.m_team = team;
        params.m_intValue = max_shots_to_fire;
        AI_Do_Command(&params);
    }

    void AI_Busy(CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_BUSY, cmd_source);
        AI_Do_Command(&params);
    }

    void AI_Dock(Object *obj, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_DOCK, cmd_source);
        params.m_obj = obj;
        AI_Do_Command(&params);
    }

    void AI_Evacuate_Instantly(bool expose_stealth_units, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_EVACUATE_INSTANTLY, cmd_source);
        params.m_intValue = expose_stealth_units;
        AI_Do_Command(&params);
    }

    void AI_Execute_Railed_Transport(CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_EXECUTE_RAILED_TRANSPORT, cmd_source);
        AI_Do_Command(&params);
    }

    void AI_Exit(Object *object_to_exit, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_EXIT, cmd_source);
        params.m_obj = object_to_exit;
        AI_Do_Command(&params);
    }

    void AI_Exit_Instantly(Object *object_to_exit, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_EXIT_INSTANTLY, cmd_source);
        params.m_obj = object_to_exit;
        AI_Do_Command(&params);
    }

    void AI_Face_Object(Object *obj, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_FACE_OBJECT, cmd_source);
        params.m_obj = obj;
        AI_Do_Command(&params);
    }

    void AI_Face_Position(const Coord3D *pos, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_FACE_POSITION, cmd_source);
        params.m_pos = *pos;
        AI_Do_Command(&params);
    }

    void AI_Follow_Exit_Production_Path(const std::vector<Coord3D> *path, Object *obj, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_FOLLOW_EXITPRODUCTION_PATH, cmd_source);
        params.m_coords = *path;
        AI_Do_Command(&params);
    }

    void AI_Follow_Path(const std::vector<Coord3D> *path, Object *obj, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_FOLLOW_PATH, cmd_source);
        params.m_coords = *path;
        params.m_obj = obj;
        AI_Do_Command(&params);
    }

    void AI_Follow_Waypath_Append(const Coord3D *pos, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_FOLLOW_WAYPATH_APPEND, cmd_source);
        params.m_pos = *pos;
        AI_Do_Command(&params);
    }

    void AI_Follow_Waypoint_Path(const Waypoint *way, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_FOLLOW_WAYPOINT_PATH, cmd_source);
        params.m_waypoint = way;
        AI_Do_Command(&params);
    }

    void AI_Follow_Waypoint_Path_As_Team(const Waypoint *way, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_FOLLOW_WAYPOINT_PATH_AS_TEAM, cmd_source);
        params.m_waypoint = way;
        AI_Do_Command(&params);
    }

    void AI_Follow_Waypoint_Path_Exact(const Waypoint *way, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_FOLLOW_WAYPOINT_PATH_EXACT, cmd_source);
        params.m_waypoint = way;
        AI_Do_Command(&params);
    }

    void AI_Follow_Waypoint_Path_Exact_As_Team(const Waypoint *way, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_FOLLOW_WAYPOINT_PATH_AS_TEAM_EXACT, cmd_source);
        params.m_waypoint = way;
        AI_Do_Command(&params);
    }

    void AI_Force_Attack_Object(Object *victim, int max_shots_to_fire, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_FORCE_ATTACK_OBJECT, cmd_source);
        params.m_obj = victim;
        params.m_intValue = max_shots_to_fire;
        AI_Do_Command(&params);
    }

    void AI_Get_Healed(Object *heal_depot, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_GET_HEALED, cmd_source);
        params.m_obj = heal_depot;
        AI_Do_Command(&params);
    }

    void AI_Get_Repaired(Object *repair_depot, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_GET_REPAIRED, cmd_source);
        params.m_obj = repair_depot;
        AI_Do_Command(&params);
    }

    void AI_Go_Prone(const DamageInfo *damage_info, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_GO_PRONE, cmd_source);
        params.m_damage = *damage_info;
        AI_Do_Command(&params);
    }

    void AI_Guard_Area(const PolygonTrigger *area_to_guard, GuardMode mode, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_GUARD_AREA, cmd_source);
        params.m_polygon = area_to_guard;
        params.m_intValue = mode;
        AI_Do_Command(&params);
    }

    void AI_Guard_Object(Object *obj_to_guard, GuardMode mode, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_GUARD_OBJECT, cmd_source);
        params.m_obj = obj_to_guard;
        params.m_intValue = mode;
        AI_Do_Command(&params);
    }

    void AI_Guard_Position(const Coord3D *pos, GuardMode mode, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_GUARD_POSITION, cmd_source);
        params.m_pos = *pos;
        params.m_intValue = mode;
        AI_Do_Command(&params);
    }

    void AI_Guard_Retaliate(Object *obj, const Coord3D *pos, int max_shots_to_fire, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_GUARD_RETALIATE, cmd_source);
        params.m_obj = obj;
        params.m_pos = *pos;
        params.m_intValue = max_shots_to_fire;
        AI_Do_Command(&params);
    }

    void AI_Guard_Tunnel_Network(GuardMode mode, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_GUARD_TUNNEL_NETWORK, cmd_source);
        params.m_intValue = mode;
        AI_Do_Command(&params);
    }

    void AI_Move_Away_From_Unit(Object *obj, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_MOVE_AWAY_FROM_UNIT, cmd_source);
        params.m_obj = obj;
        AI_Do_Command(&params);
    }

    void AI_Move_To_And_Evacuate(const Coord3D *pos, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_MOVE_TO_POSITION_AND_EVACUATE, cmd_source);
        params.m_pos = *pos;
        AI_Do_Command(&params);
    }

    void AI_Move_To_And_Evacuate_And_Exit(const Coord3D *pos, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_MOVE_TO_POSITION_AND_EVACUATE_AND_EXIT, cmd_source);
        params.m_pos = *pos;
        AI_Do_Command(&params);
    }

    void AI_Move_To_Object(Object *obj, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_MOVE_TO_OBJECT, cmd_source);
        params.m_obj = obj;
        AI_Do_Command(&params);
    }

    void AI_Panic(const Waypoint *way, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_PANIC, cmd_source);
        params.m_waypoint = way;
        AI_Do_Command(&params);
    }

    void AI_Rappel_Into(Object *obj, const Coord3D &pos, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_RAPPEL_INTO, cmd_source);
        params.m_obj = obj;
        params.m_pos = pos;
        AI_Do_Command(&params);
    }

    void AI_Repair(Object *obj, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_REPAIR, cmd_source);
        params.m_obj = obj;
        AI_Do_Command(&params);
    }

    void AI_Resume_Construction(Object *obj, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_RESUME_CONSTRUCTION, cmd_source);
        params.m_obj = obj;
        AI_Do_Command(&params);
    }

    void AI_Tighten_To_Position(const Coord3D *pos, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_TIGHTEN_TO_POSITION, cmd_source);
        params.m_pos = *pos;
        AI_Do_Command(&params);
    }

    void AI_Unk50(const Coord3D *pos, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_UNK50, cmd_source);
        params.m_pos = *pos;
        AI_Do_Command(&params);
    }

    void AI_Wander(const Waypoint *way, CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_WANDER, cmd_source);
        params.m_waypoint = way;
        AI_Do_Command(&params);
    }

    void AI_Wander_In_Place(CommandSourceType cmd_source)
    {
        AICommandParms params(AICMD_WANDER_IN_PLACE, cmd_source);
        AI_Do_Command(&params);
    }
};

class AIUpdateModuleData : public UpdateModuleData
{
public:
    virtual ~AIUpdateModuleData() override;
    virtual bool Is_AI_Module_Data() const override;

    static void Parse_Locomotor_Set(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Turret(INI *ini, void *formal, void *store, const void *user_data);

private:
    std::map<LocomotorSetType, std::vector<const LocomotorTemplate *>> m_locomotorTemplates;
    TurretAIData *m_turretData[MAX_TURRETS];
    unsigned int m_moodAttackCheckRate;
    bool m_forbidPlayerCommands;
    bool m_turretsLinked;
    int m_autoAcquireEnemiesWhenIdle;
    friend class AIUpdateInterface;
};

class AIUpdateInterface : public UpdateModule, public AICommandInterface
{
    IMPLEMENT_POOL(AIUpdateInterface)

public:
    enum LocoGoalType
    {
        NONE,
        POSITION_ON_PATH,
        POSITION_EXPLICIT,
        ANGLE,
    };

    virtual ~AIUpdateInterface() override;
    virtual NameKeyType Get_Module_Name_Key() const override;
    virtual void On_Object_Created() override;
    virtual SleepyUpdatePhase Get_Update_Phase() const override;

    virtual void Private_Move_To_Position(const Coord3D *pos, CommandSourceType cmd_source);
    virtual void Private_Move_To_Object(Object *obj, CommandSourceType cmd_source);
    virtual void Private_Move_To_And_Evacuate(const Coord3D *pos, CommandSourceType cmd_source);
    virtual void Private_Move_To_And_Evacuate_And_Exit(const Coord3D *pos, CommandSourceType cmd_source);

    virtual void Private_Idle(CommandSourceType cmd_source);
    virtual void Private_Tighten_To_Position(const Coord3D *pos, CommandSourceType cmd_source);
    virtual void Private_Follow_Waypoint_Path(const Waypoint *way, CommandSourceType cmd_source);
    virtual void Private_Follow_Waypoint_Path_As_Team(const Waypoint *way, CommandSourceType cmd_source);
    virtual void Private_Follow_Waypoint_Path_Exact(const Waypoint *way, CommandSourceType cmd_source);
    virtual void Private_Follow_Waypoint_Path_As_Team_Exact(const Waypoint *way, CommandSourceType cmd_source);
    virtual void Private_Follow_Path(
        const std::vector<Coord3D> *path, Object *ignore_object, CommandSourceType cmd_source, bool exit_production);
    virtual void Private_Follow_Path_Append(const Coord3D *pos, CommandSourceType cmd_source);

    virtual void Private_Attack_Object(Object *victim, int max_shots_to_fire, CommandSourceType cmd_source);
    virtual void Private_Force_Attack_Object(Object *victim, int max_shots_to_fire, CommandSourceType cmd_source);
    virtual void Private_Guard_Retaliate(
        Object *obj, const Coord3D *pos, int max_shots_to_fire, CommandSourceType cmd_source);
    virtual void Private_Attack_Team(const Team *team, int max_shots_to_fire, CommandSourceType cmd_source);
    virtual void Private_Attack_Position(const Coord3D *pos, int max_shots_to_fire, CommandSourceType cmd_source);
    virtual void Private_Attack_Move_To_Position(const Coord3D *pos, int max_shots_to_fire, CommandSourceType cmd_source);
    virtual void Private_Attack_Follow_Waypoint_Path(
        const Waypoint *way, int max_shots_to_fire, bool as_team, CommandSourceType cmd_source);

    virtual void Private_Hunt(CommandSourceType cmd_source);
    virtual void Private_Repair(Object *obj, CommandSourceType cmd_source);
    virtual void Private_Resume_Construction(Object *obj, CommandSourceType cmd_source);
    virtual void Private_Get_Healed(Object *heal_depot, CommandSourceType cmd_source);
    virtual void Private_Get_Repaired(Object *repair_depot, CommandSourceType cmd_source);
    virtual void Private_Enter(Object *obj, CommandSourceType cmd_source);
    virtual void Private_Dock(Object *obj, CommandSourceType cmd_source);
    virtual void Private_Exit(Object *object_to_exit, CommandSourceType cmd_source);
    virtual void Private_Exit_Instantly(Object *object_to_exit, CommandSourceType cmd_source);

    virtual void Private_Evacuate(int expose_stealth_units, CommandSourceType cmd_source);
    virtual void Private_Evacuate_Instantly(int expose_stealth_units, CommandSourceType cmd_source);
    virtual void Private_Execute_Railed_Transport(CommandSourceType cmd_source);
    virtual void Private_Go_Prone(const DamageInfo *damage_info, CommandSourceType cmd_source);
    virtual void Private_Guard_Tunnel_Network(GuardMode mode, CommandSourceType cmd_source);
    virtual void Private_Guard_Position(const Coord3D *pos, GuardMode mode, CommandSourceType cmd_source);
    virtual void Private_Guard_Object(Object *obj, GuardMode mode, CommandSourceType cmd_source);
    virtual void Private_Guard_Area(const PolygonTrigger *area_to_guard, GuardMode mode, CommandSourceType cmd_source);

    virtual void Private_Attack_Area(const PolygonTrigger *area_to_guard, CommandSourceType cmd_source);
    virtual void Private_Hack_Internet(CommandSourceType cmd_source);
    virtual void Private_Face_Object(Object *obj, CommandSourceType cmd_source);
    virtual void Private_Face_Position(const Coord3D *pos, CommandSourceType cmd_source);
    virtual void Private_Rappel_Into(Object *obj, const Coord3D &pos, CommandSourceType cmd_source);
    virtual void Private_Combat_Drop(Object *obj, const Coord3D &pos, CommandSourceType cmd_source);
    virtual void Private_Command_Button(const CommandButton *button, CommandSourceType cmd_source);
    virtual void Private_Command_Button_Position(
        const CommandButton *button, const Coord3D *pos, CommandSourceType cmd_source);
    virtual void Private_Command_Button_Object(const CommandButton *button, Object *obj, CommandSourceType cmd_source);

    virtual void Private_Wander(const Waypoint *way, CommandSourceType cmd_source);
    virtual void Private_Wander_In_Place(CommandSourceType cmd_source);
    virtual void Private_Panic(const Waypoint *way, CommandSourceType cmd_source);
    virtual void Private_Busy(CommandSourceType cmd_source);
    virtual void Private_Move_Away_From_Unit(Object *obj, CommandSourceType cmd_source);

    virtual const DozerAIInterface *Get_Dozer_AI_Interface() const;
    virtual DozerAIInterface *Get_Dozer_AI_Interface();
    virtual const SupplyTruckAIInterface *Get_Supply_Truck_AI_Interface() const;
    virtual SupplyTruckAIInterface *Get_Supply_Truck_AI_Interface();
    virtual const WorkerAIInterface *Get_Worker_AI_Interface() const;
    virtual WorkerAIInterface *Get_Worker_AI_Interface();
    virtual const HackInternetAIInterface *Get_Hack_Internet_AI_Interface() const;
    virtual HackInternetAIInterface *Get_Hack_Internet_AI_Interface();
    virtual const AssaultTransportAIInterface *Get_Assault_Transport_AI_Interface() const;
    virtual AssaultTransportAIInterface *Get_Assault_Transport_AI_Interface();
    virtual const JetAIUpdate *Get_Jet_AI_Update() const;
    virtual JetAIUpdate *Get_Jet_AI_Update();

    virtual void Join_Team();
    virtual AIFreeToExitType Get_Ai_Free_To_Exit(const Object *obj) const;
    virtual bool Is_Allowed_To_Adjust_Destination() const;
    virtual bool Is_Allowed_To_Move_Away_From_Unit() const;
    virtual ObjectID Get_Building_To_Not_Path_Around() const;
    virtual bool Is_Idle() const;
    virtual bool Is_Attacking() const;
    virtual bool Is_Clearing_Mines() const;
    virtual bool Is_Taxiing_To_Parking() const;
    virtual bool Is_Busy() const;

    virtual void Do_Quick_Exit(const std::vector<Coord3D> *path);
    virtual const Coord3D *Get_Guard_Location() const;
    virtual ObjectID const Get_Guard_Object() const;
    virtual const PolygonTrigger *Get_Area_To_Guard() const;
    virtual GuardTargetType Get_Guard_Target_Type() const;
    virtual void Clear_Guard_Target_Type();
    virtual GuardMode Get_Guard_Mode() const;
    virtual Object *Construct(
        const ThingTemplate *what, const Coord3D *pos, float angle, Player *owning_player, bool is_rebuild);

    virtual Object *Get_Enter_Target();
    virtual bool Get_Sneaky_Targeting_Offset(Coord3D *pos) const;
    virtual void Add_Targeter(ObjectID obj, bool b);
    virtual bool Is_Temporarily_Preventing_Aim_Success() const;
    virtual void Set_Locomotor_Goal_Position_On_Path();
    virtual void Set_Locomotor_Goal_Position_Explicit(const Coord3D &pos);
    virtual void Set_Locomotor_Goal_Orientation(float orientation);
    virtual void Set_Locomotor_Goal_None();
    virtual bool Is_Doing_Ground_Movement() const;

    virtual bool Process_Collision(PhysicsBehavior *physics, Object *other);
    virtual bool Choose_Locomotor_Set(LocomotorSetType wst);
    virtual CommandSourceType Get_Last_Command_Source() const;
    virtual void Notify_Victim_Is_Dead();
    virtual bool Is_Allowed_To_Respond_To_AI_Commands(const AICommandParms *params) const;
    virtual UpdateSleepTime Do_Locomotor();
    virtual AIStateMachine *Make_State_Machine();
    virtual bool Get_Treat_As_Aircraft_For_Loco_Dist_To_Goal() const;
    virtual void Friend_Notify_State_Machine_Changed();

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;
    virtual AIUpdateInterface *Get_AI_Update_Interface() override;
    virtual UpdateSleepTime Update() override;
    virtual BitFlags<DISABLED_TYPE_COUNT> Get_Disabled_Types_To_Process() const override;
    virtual void AI_Do_Command(const AICommandParms *params) override;

    float Get_Cur_Locomotor_Speed() const;
    bool Get_Turret_Rot_And_Pitch(WhichTurretType tur, float *turret_angle, float *turret_pitch) const;
    unsigned int Get_Mood_Matrix_Action_Adjustment(MoodMatrixAction action) const;
    Object *Get_Next_Mood_Target(bool called_by_ai, bool called_during_idle);
    void Reset_Next_Mood_Check_Time();
    void Set_Next_Mood_Check_Time(unsigned int time);
    bool Is_Weapon_Slot_On_Turret_And_Aiming_At_Target(WeaponSlotType wslot, const Object *victim) const;
    void Transfer_Attack(ObjectID old_victim_id, ObjectID new_victim_id);

    bool Has_Locomotor_For_Surface(LocomotorSurfaceType t);
    int Friend_Get_Waypoint_Goal_Path_Size() const;
    AIStateType Get_AI_State_Type() const;
    const Coord3D *Get_Goal_Path_Position(int i) const;
    const Coord3D *Get_Goal_Position() const;
    WhichTurretType Get_Which_Turret_For_Weapon_Slot(WeaponSlotType wslot, float *turret_angle, float *turret_pitch) const;
    bool Is_Aircraft_That_Adjusts_Destination() const;
    bool Is_Turret_Enabled(WhichTurretType tur) const;
    bool Can_Auto_Acquire_While_Stealthed() const;
    bool Is_Moving() const;
    void Destroy_Path();
    void Execute_Waypoint_Queue();

    const Locomotor *Get_Cur_Locomotor() const { return m_curLocomotor; }
    Locomotor *Get_Cur_Locomotor() { return m_curLocomotor; }
    LocomotorSetType Get_Cur_Locomotor_Set() const { return m_curLocomotorSet; }
    Path *Get_Path() const { return m_path; }
    ObjectID Get_Ignored_Obstacle_ID() { return m_ignoreObstacleID; }
    const AIStateMachine *Get_State_Machine() const { return m_stateMachine; }
    AIStateMachine *Get_State_Machine() { return m_stateMachine; }
    unsigned int Get_Next_Mood_Check_Time() const { return m_nextMoodCheckTime; }
    WhichTurretType Get_Current_Turret() const { return m_currentTurret; }
    Object *Get_Goal_Object() { return Get_State_Machine()->Get_Goal_Object(); }
    const LocomotorSet *Get_Locomotor_Set() const { return &m_locomotorSet; }
    int Get_Unk3() const { return m_unk3; }
    bool Is_Recruitable() const { return m_isRecruitable; }
    int Is_Auto_Acquire_Enemies_When_Idle() const { return Get_AI_Update_Module_Data()->m_autoAcquireEnemiesWhenIdle != 0; }
    bool Is_Waiting_For_Path() const { return m_waitingForPath; }
    bool Is_AI_In_Dead_State() const { return m_isAiDead; }

    const AIUpdateModuleData *Get_AI_Update_Module_Data() const
    {
        return static_cast<const AIUpdateModuleData *>(Get_Module_Data());
    }

    void Set_Current_Turret(WhichTurretType t) { m_currentTurret = t; }
    void Set_Attitude(AttitudeType attitude) { m_attitude = attitude; }
    void Set_Attack_Priority_Info(const AttackPriorityInfo *info) { m_attackInfo = info; }
    void Set_Is_Recruitable(bool is) { m_isRecruitable = is; }
    void Set_Unk3(int i) { m_unk3 = i; }
#ifdef GAME_DEBUG_STRUCTS
    Utf8String Get_Current_State_Name() { return m_stateMachine->Get_Current_State_Name(); }
#endif

    bool Are_Turrets_Linked() const { return Get_AI_Update_Module_Data()->m_turretsLinked; }
    int Get_Current_Goal_Path_Index() const { return m_currentGoalPathIndex; }
    const AttackPriorityInfo *Get_Attack_Priority_Info() const { return m_attackInfo; }

private:
    unsigned int m_unkWaypoint1;
    unsigned int m_unkWaypoint2;
    AIStateMachine *m_stateMachine;
    unsigned int m_unk1;
    ObjectID m_currentVictimID;
    float m_unkSpeed1;
    CommandSourceType m_lastCommandSource;
    GuardMode m_guardMode;
    GuardType m_guardType;
    GuardTargetType m_guardTargetType;
    Coord3D m_locationToGuard;
    ObjectID m_objectToGuard;
    PolygonTrigger *m_areaToGuard;
    const AttackPriorityInfo *m_attackInfo;
    Coord3D m_waypointQueue[MAX_PLAYER_COUNT];
    int m_waypointCount;
    int m_waypointIndex;
    Waypoint *m_completedWaypoint;
    Path *m_path;
    ObjectID m_requestedVictimID;
    Coord3D m_requestedDestination;
    Coord3D m_unk2;
    unsigned int m_pathTimestamp;
    ObjectID m_ignoreObstacleID;
    float m_pathExtraDistance;
    ICoord2D m_pathfindGoalCell;
    ICoord2D m_pathfindCurCell;
    unsigned int m_blockedFrames;
    float m_curMaxBlockedSpeed;
    float m_unkSpeed2;
    unsigned int m_ignoreCollisionsTimer;
    unsigned int m_queueForPathTime;
    Coord3D m_finalPosition;
    ObjectID m_unkPathObjId1;
    ObjectID m_unkPathObjId2;
    int m_currentGoalPathIndex;
    ObjectID m_unkObjID1;
    ObjectID m_unkObjID2;
    LocomotorSet m_locomotorSet;
    Locomotor *m_curLocomotor;
    LocomotorSetType m_curLocomotorSet;
    LocoGoalType m_locomotorGoalType;
    Coord3D m_locomotorGoalData;
    TurretAI *m_turretAI[MAX_TURRETS];
    WhichTurretType m_currentTurret;
    AttitudeType m_attitude;
    unsigned int m_nextMoodCheckTime;
    ObjectID m_unkObjID3;
    int m_unk3;
    bool m_doFinalPosition;
    bool m_waitingForPath;
    bool m_isAttackPath;
    bool m_unkRequPath;
    bool m_isApproachPath;
    bool m_isSafePath;
    bool m_unkEndingMove;
    bool m_unkStartingToMove;
    bool m_isBlocked;
    bool m_isBlockedAndStuck;
    bool m_unkLocoUpgrade;
    bool m_unkIngoreCollisions;
    bool m_unkNextMoodCheckTime;
    bool m_isAiDead;
    bool m_isRecruitable;
    bool m_executingWaypointQueue;
    bool m_unkPath;
    bool m_unk4;
    bool m_unkWakeup;
    bool m_unk5;
};
