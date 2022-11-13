/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief AI States
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
#include "statemachine.h"

enum AIStateType
{
    AI_IDLE = 0,
    AI_MOVE_TO = 1,
    AI_FOLLOW_WAYPOINT_PATH_AS_TEAM = 2,
    AI_FOLLOW_WAYPOINT_PATH_AS_INDIVIDUALS = 3,
    AI_FOLLOW_WAYPOINT_PATH_EXACT_AS_TEAM = 4,
    AI_FOLLOW_WAYPOINT_PATH_EXACT_AS_INDIVIDUALS = 5,
    AI_FOLLOW_PATH = 6,
    AI_FOLLOW_EXITPRODUCTION_PATH = 7,
    AI_WAIT = 8,
    AI_ATTACK_POSITION = 9,
    AI_ATTACK_OBJECT = 10,
    AI_FORCE_ATTACK_OBJECT = 11,
    AI_ATTACK_AND_FOLLOW_OBJECT = 12,
    AI_DEAD = 13,
    AI_DOCK = 14,
    AI_ENTER = 15,
    AI_GUARD = 16,
    AI_HUNT = 17,
    AI_WANDER = 18,
    AI_PANIC = 19,
    AI_ATTACK_SQUAD = 20,
    AI_GUARD_TUNNEL_NETWORK = 21,
    AI_MOVE_OUT_OF_THE_WAY = 23,
    AI_MOVE_AND_TIGHTEN = 24,
    AI_MOVE_AND_EVACUATE = 25,
    AI_MOVE_AND_EVACUATE_AND_EXIT = 26,
    AI_MOVE_AND_DELETE = 27,
    AI_ATTACK_AREA = 28,
    AI_ATTACK_MOVE_TO = 30,
    AI_ATTACKFOLLOW_WAYPOINT_PATH_AS_INDIVIDUALS = 31,
    AI_ATTACKFOLLOW_WAYPOINT_PATH_AS_TEAM = 32,
    AI_FACE_POSITION = 33,
    AI_FACE_OBJECT = 34,
    AI_RAPPEL = 35,
    AI_EXIT = 37,
    AI_PICK_UP_CRATE = 38,
    AI_MOVE_AWAY_FROM_REPULSORS = 39,
    AI_WANDER_IN_PLACE = 40,
    AI_BUSY = 41,
    AI_EXIT_INSTANTLY = 42,
    AI_GUARD_RETALIATE = 43,
    NUM_AI_STATES = 44,
};

class Squad;

class AIStateMachine : public StateMachine
{
    IMPLEMENT_POOL(AIStateMachine)

public:
    virtual ~AIStateMachine() override;
    virtual StateReturnType Update_State_Machine() override;
    virtual void Clear() override;
    virtual StateReturnType Reset_To_Default_State() override;
    virtual StateReturnType Set_State(unsigned int new_state_id) override;
#ifdef GAME_DEBUG_STRUCTS
    virtual Utf8String Get_Current_State_Name() override;
#endif

    int Get_Goal_Path_Count() const { return m_goalPath.size(); }
    const Coord3D *Get_Goal_Path_Position(int i) const;

private:
    std::vector<Coord3D> m_goalPath;
    Waypoint *m_goalWaypoint;
    Squad *m_goalSquad;
    State *m_temporaryState;
    unsigned int m_temporaryStateFrame;
};

class NotifyWeaponFiredInterface
{
public:
    virtual void Notify_Fired() = 0;
    virtual void Notify_New_Victim_Chosen(Object *victim) = 0;
    virtual bool Is_Weapon_Slot_Ok_To_Fire(WeaponSlotType wslot) const = 0;
    virtual bool Is_Attacking_Object() const = 0;
    virtual const Coord3D *Get_Original_Victim_Pos() const = 0;
};

class AIAttackFireWeaponState : public State
{
    IMPLEMENT_POOL(AIAttackFireWeaponState);

public:
    AIAttackFireWeaponState(StateMachine *machine, NotifyWeaponFiredInterface *iface) :
        State(machine, "AIAttackFireWeaponState"), m_att(iface)
    {
    }

    virtual StateReturnType On_Enter() override;
    virtual void On_Exit(StateExitType status) override;
    virtual StateReturnType Update() override;

    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

private:
    NotifyWeaponFiredInterface *m_att;
};

bool Out_Of_Weapon_Range_Object(State *state, void *user_data);
