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
#include "object.h"
#include "updatemodule.h"
class INI;
class AICommandParms;
class DozerAIInterface;
class SupplyTruckAIInterface;
class WorkerAIInterface;
class HackInternetAIInterface;
class AssaultTransportAIInterface;
class JetAIUpdate;
class AIStateMachine;
class Locomotor;
class Path;

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
enum LocomotorSetType
{
    LOCOMOTOR_SET_UNK
};

class AICommandInterface
{
public:
    virtual void AI_Do_Command(AICommandParms const *params) = 0;
};

class AIUpdateModuleData
{
public:
    static void Parse_Locomotor_Set(INI *ini, void *formal, void *store, const void *user_data);
};

class AIUpdateInterface : public UpdateModule, public AICommandInterface
{
public:
    virtual ~AIUpdateInterface() override;
    virtual NameKeyType Get_Module_Name_Key() const override;
    virtual void On_Object_Created() override;
    virtual SleepyUpdatePhase Get_Update_Phase() const override;

    virtual void Private_Move_To_Position(Coord3D const *pos, CommandSourceType cmd_source);
    virtual void Private_Move_To_Object(Object *obj, CommandSourceType cmd_source);
    virtual void Private_Move_To_And_Evacuate(Coord3D const *pos, CommandSourceType cmd_source);
    virtual void Private_Move_To_And_Evacuate_And_Exit(Coord3D const *pos, CommandSourceType cmd_source);

    virtual void Private_Idle(CommandSourceType cmd_source);
    virtual void Private_Tighten_To_Position(Coord3D const *pos, CommandSourceType cmd_source);
    virtual void Private_Follow_Waypoint_Path(Waypoint const *way, CommandSourceType cmd_source);
    virtual void Private_Follow_Waypoint_Path_As_Team(Waypoint const *way, CommandSourceType cmd_source);
    virtual void Private_Follow_Waypoint_Path_Exact(Waypoint const *way, CommandSourceType cmd_source);
    virtual void Private_Follow_Waypoint_Path_As_Team_Exact(Waypoint const *way, CommandSourceType cmd_source);
    virtual void Private_Follow_Path(
        std::vector<Coord3D> const *path, Object *ignore_object, CommandSourceType cmd_source, bool exit_production);
    virtual void Private_Follow_Path_Append(Coord3D const *pos, CommandSourceType cmd_source);

    virtual void Private_Attack_Object(Object *victim, int max_shots_to_fire, CommandSourceType cmd_source);
    virtual void Private_Force_Attack_Object(Object *victim, int max_shots_to_fire, CommandSourceType cmd_source);
    virtual void Private_Guard_Retaliate(Coord3D const *pos, int max_shots_to_fire, CommandSourceType cmd_source);
    virtual void Private_Attack_Team(Team const *team, int max_shots_to_fire, CommandSourceType cmd_source);
    virtual void Private_Attack_Position(Coord3D const *pos, int max_shots_to_fire, CommandSourceType cmd_source);
    virtual void Private_Attack_Move_To_Position(Coord3D const *pos, int max_shots_to_fire, CommandSourceType cmd_source);
    virtual void Private_Attack_Follow_Waypoint_Path(
        Waypoint const *way, int max_shots_to_fire, bool as_team, CommandSourceType cmd_source);

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
    virtual void Private_EvacuateInstantly(int expose_stealth_units, CommandSourceType cmd_source);
    virtual void Private_Execute_Railed_Transport(CommandSourceType cmd_source);
    virtual void Private_Go_Prone(DamageInfo const *damage_info, CommandSourceType cmd_source);
    virtual void Private_Guard_Tunnel_Network(GuardMode mode, CommandSourceType cmd_source);
    virtual void Private_Guard_Position(Coord3D const *pos, GuardMode mode, CommandSourceType cmd_source);
    virtual void Private_Guard_Object(Object *obj, GuardMode mode, CommandSourceType cmd_source);
    virtual void Private_Guard_Area(PolygonTrigger const *area_to_guard, GuardMode mode, CommandSourceType cmd_source);

    virtual void Private_Attack_Area(PolygonTrigger const *area_to_guard, CommandSourceType cmd_source);
    virtual void Private_Hack_Internet(CommandSourceType cmd_source);
    virtual void Private_Face_Object(Object *obj, CommandSourceType cmd_source);
    virtual void Private_Face_Position(Coord3D const *pos, CommandSourceType cmd_source);
    virtual void Private_Rappe_lInto(Object *obj, Coord3D const &pos, CommandSourceType cmd_source);
    virtual void Private_Combat_Drop(Object *obj, Coord3D const &pos, CommandSourceType cmd_source);
    virtual void Private_Command_Button(CommandButton const *button, CommandSourceType cmd_source);
    virtual void Private_Command_Button_Position(
        CommandButton const *button, Coord3D const *pos, CommandSourceType cmd_source);
    virtual void Private_Command_Button_Object(CommandButton const *button, Object *obj, CommandSourceType cmd_source);

    virtual void Private_Wander(Waypoint const *way, CommandSourceType cmd_source);
    virtual void Private_Wander_In_Place(CommandSourceType cmd_source);
    virtual void Private_Panic(Waypoint const *way, CommandSourceType cmd_source);
    virtual void Private_Busy(CommandSourceType cmd_source);
    virtual void Private_Move_Away_From_Unit(Object *obj, CommandSourceType cmd_source);

    virtual DozerAIInterface const *Get_Dozer_AI_Interface() const;
    virtual DozerAIInterface *Get_Dozer_AI_Interface();
    virtual SupplyTruckAIInterface const *Get_Supply_Truck_AI_Interface() const;
    virtual SupplyTruckAIInterface *Get_Supply_Truck_AI_Interface();
    virtual WorkerAIInterface const *Get_Worker_AI_Interface() const;
    virtual WorkerAIInterface *Get_Worker_AI_Interface();
    virtual HackInternetAIInterface const *Get_Hack_Internet_AI_Interface() const;
    virtual HackInternetAIInterface *Get_Hack_Internet_AI_Interface();
    virtual AssaultTransportAIInterface const *Get_Assault_Transport_AI_Interface() const;
    virtual AssaultTransportAIInterface *Get_Assault_Transport_AI_Interface();
    virtual JetAIUpdate const *Get_Jet_AI_Update() const;
    virtual JetAIUpdate *Get_Jet_AI_Update();

    virtual void Join_Team();
    virtual AIFreeToExitType Get_Ai_Free_To_Exit(Object const *obj) const;
    virtual bool Is_Allowed_To_Adjust_Destination() const;
    virtual bool Is_Allowed_To_Move_Away_From_Unit() const;
    virtual ObjectID Get_Building_To_Not_Path_Around() const;
    virtual bool Is_Idle() const;
    virtual bool Is_Attacking() const;
    virtual bool Is_Clearing_Mines() const;
    virtual bool Is_Taxiing_To_Parking() const;
    virtual bool Is_Busy() const;

    virtual void Do_Quick_Exit(std::vector<Coord3D> const *path);
    virtual Coord3D const *Get_Guard_Location() const;
    virtual ObjectID const Get_Guard_Object() const;
    virtual PolygonTrigger const *Get_Area_To_Guard() const;
    virtual GuardTargetType Get_Guard_Target_Type() const;
    virtual void Clear_Guard_Target_Type();
    virtual GuardMode Get_Guard_Mode() const;
    virtual Object *Construct(
        ThingTemplate const *what, Coord3D const *pos, float angle, Player *owning_player, bool is_rebuild);

    virtual Object *Get_Enter_Target();
    virtual bool Get_Sneaky_Targeting_Offset(Coord3D *pos) const;
    virtual void Add_Targeter(ObjectID obj, bool b);
    virtual bool Is_Temporarily_Preventing_Aim_Success() const;
    virtual void Set_Locomotor_Goal_Position_On_Path();
    virtual void Set_Locomotor_Goal_Position_Explicit(Coord3D const &pos);
    virtual void Set_Locomotor_Goal_Orientation(float orientation);
    virtual void Set_Locomotor_Goal_None();
    virtual bool Is_Doing_Ground_Movement() const;

    virtual bool Process_Collision(PhysicsBehavior *physics, Object *other);
    virtual bool Choose_Locomotor_Set(LocomotorSetType wst);
    virtual CommandSourceType Get_Last_Command_Source() const;
    virtual void Notify_Victim_Is_Dead();
    virtual bool Is_Allowed_To_Respond_To_AI_Commands(AICommandParms const *params) const;
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
    virtual void AI_Do_Command(AICommandParms const *params) override;

    float Get_Cur_Locomotor_Speed() const;
    bool Get_Turret_Rot_And_Pitch(WhichTurretType tur, float *turret_angle, float *turret_pitch);

    const Locomotor *Get_Locomotor() const { return m_curLocomotor; }
    const Path *Get_Path() const { return m_path; }

private:
    unsigned char unk[0x10C];
    Path *m_path;
    unsigned char unk2[0x84];
    Locomotor *m_curLocomotor;
};
