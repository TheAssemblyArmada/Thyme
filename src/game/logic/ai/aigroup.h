/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief AI Group
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
#include "aiupdate.h"
#include "controlbar.h"
#include "gametype.h"
#include "mempoolobj.h"
#include "snapshot.h"
#include "subsysteminterface.h"
#include <list>

class AIGroup : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_NAMED_POOL(AIGroup, AIGroupPool)

public:
    AIGroup();
    virtual ~AIGroup() override;

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    int Get_ID();
    float Get_Speed();
    const std::vector<ObjectID> &Get_All_IDs() const;
    bool Get_Center(Coord3D *center);
    bool Get_Min_Max_And_Center(Coord2D *min, Coord2D *max, Coord3D *center);
    unsigned int Get_Count();
    AttitudeType Get_Attitude() const;
    Object *Get_Special_Power_Source_Object(unsigned int special_power_id);
    Object *Get_Command_Button_Source_Object(GUICommand command_type);

    bool Is_Member(Object *obj);
    bool Is_Empty();
    bool Is_Idle() const;
    bool Is_Busy() const;
    bool Is_Group_AI_Dead() const;

    void Set_Attitude(AttitudeType tude);
    void Set_Mine_Clearing_Detail(bool set);
    bool Set_Weapon_Lock_For_Group(WeaponSlotType wslot, WeaponLockType wlock);
    void Set_Weapon_Set_Flag(WeaponSetType wst);

    void Add(Object *obj);
    bool Remove(Object *obj);
    void Recompute();
    bool Contains_Any_Objects_Not_Owned_By_Player(const Player *player);
    bool Remove_Any_Objects_Not_Owned_By_Player(const Player *player);
    void Compute_Individual_Destination(
        Coord3D *dest, const Coord3D *group_dest, Object *obj, const Coord3D *center, bool adjust);
    bool Friend_Compute_Ground_Path(const Coord3D *pos, CommandSourceType cmd_source);
    bool Friend_Move_Infantry_To_Pos(const Coord3D *pos, CommandSourceType cmd_source);
    void Friend_Move_Formation_To_Pos(const Coord3D *pos, CommandSourceType cmd_source);
    bool Friend_Move_Vehicle_To_Pos(const Coord3D *pos, CommandSourceType cmd_source);
    void Release_Weapon_Lock_For_Group(WeaponLockType wlock);
    void Queue_Upgrade(UpgradeTemplate *upgrade);

    void Group_Move_To_Position(const Coord3D *pos, bool append, CommandSourceType cmd_source);
    void Group_Scatter(CommandSourceType cmd_source);
    void Group_Tighten_To_Position(const Coord3D *target_pos, bool append, CommandSourceType cmd_source);
    void Group_Follow_Waypoint_Path(const Waypoint *way, CommandSourceType cmd_source);
    void Group_Follow_Waypoint_Path_Exact(const Waypoint *way, CommandSourceType cmd_source);
    void Group_Move_To_And_Evacuate(const Coord3D *pos, CommandSourceType cmd_source);
    void Group_Move_To_And_Evacuate_And_Exit(const Coord3D *pos, CommandSourceType cmd_source);
    void Group_Follow_Waypoint_Path_As_Team(const Waypoint *way, CommandSourceType cmd_source);
    void Group_Follow_Waypoint_Path_As_Team_Exact(const Waypoint *way, CommandSourceType cmd_source);
    void Group_Idle(CommandSourceType cmd_source);
    void Group_Follow_Path(const std::vector<Coord3D> *path, Object *ignore_object, CommandSourceType cmd_source);
    void Group_Attack_Object_Private(bool force, Object *victim, int max_shots_to_fire, CommandSourceType cmd_source);
    void Group_Attack_Team(const Team *team, int max_shots_to_fire, CommandSourceType cmd_source);
    void Group_Attack_Position(const Coord3D *pos, int max_shots_to_fire, CommandSourceType cmd_source);
    void Group_Attack_Move_To_Position(const Coord3D *pos, int max_shots_to_fire, CommandSourceType cmd_source);
    void Group_Hunt(CommandSourceType cmd_source);
    void Group_Repair(Object *obj, CommandSourceType cmd_source);
    void Group_Resume_Construction(Object *obj, CommandSourceType cmd_source);
    void Group_Get_Healed(Object *heal_depot, CommandSourceType cmd_source);
    void Group_Get_Repaired(Object *repair_depot, CommandSourceType cmd_source);
    void Group_Enter(Object *obj, CommandSourceType cmd_source);
    void Group_Dock(Object *obj, CommandSourceType cmd_source);
    void Group_Exit(Object *object_to_exit, CommandSourceType cmd_source);
    void Group_Evacuate(CommandSourceType cmd_source);
    void Group_Execute_Railed_Transport(CommandSourceType cmd_source);
    void Group_Go_Prone(const DamageInfo *damage_info, CommandSourceType cmd_source);
    void Group_Guard_Position(const Coord3D *pos, GuardMode mode, CommandSourceType cmd_source);
    void Group_Guard_Object(Object *obj_to_guard, GuardMode mode, CommandSourceType cmd_source);
    void Group_Guard_Area(const PolygonTrigger *area_to_guard, GuardMode mode, CommandSourceType cmd_source);
    void Group_Attack_Area(const PolygonTrigger *area_to_guard, CommandSourceType cmd_source);
    void Group_Hack_Internet(CommandSourceType cmd_source);
    void Group_Create_Formation(CommandSourceType cmd_source);
    void Group_Do_Special_Power(unsigned int special_power_id, unsigned int options);
    void Group_Do_Special_Power_At_Location(
        unsigned int special_power_id, const Coord3D *location, float f, const Object *object_in_way, unsigned int options);
    void Group_Do_Special_Power_At_Object(unsigned int special_power_id, Object *target, unsigned int options);
    void Group_Cheer(CommandSourceType cmd_source);
    void Group_Sell(CommandSourceType cmd_source);
    void Group_Toggle_Overcharge(CommandSourceType cmd_source);
    void Group_Combat_Drop(Object *obj, const Coord3D &pos, CommandSourceType cmd_source);
    void Group_Do_Command_Button(const CommandButton *button, CommandSourceType cmd_source);
    void Group_Do_Command_Button_At_Position(const CommandButton *button, const Coord3D *pos, CommandSourceType cmd_source);
    void Group_Do_Command_Button_Using_Waypoints(
        const CommandButton *button, const Waypoint *way, CommandSourceType cmd_source);
    void Group_Do_Command_Button_At_Object(const CommandButton *button, Object *obj, CommandSourceType cmd_source);
    void Group_Set_Emoticon(const Utf8String &emoticon, int frames);
    void Group_Override_Special_Power_Destination(
        SpecialPowerType type, const Coord3D *destination, CommandSourceType cmd_source);

    void Set_Dirty() { m_dirty = true; }

    void Group_Force_Attack_Object(Object *victim, int max_shots_to_fire, CommandSourceType cmd_source)
    {
        Group_Attack_Object_Private(true, victim, max_shots_to_fire, cmd_source);
    }

    void Group_Attack_Object(Object *victim, int max_shots_to_fire, CommandSourceType cmd_source)
    {
        Group_Attack_Object_Private(false, victim, max_shots_to_fire, cmd_source);
    }

private:
    std::list<Object *> m_memberList;
    unsigned int m_memberListSize;
    float m_speed;
    bool m_dirty;
    unsigned int m_id;
    Path *m_groundPath;
    mutable std::vector<ObjectID> m_lastRequestedIDList;
};
