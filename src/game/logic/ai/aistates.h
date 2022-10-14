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
