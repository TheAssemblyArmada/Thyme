/**
/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief State Machine
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
#include "coord.h"
#include "mempoolobj.h"
#include "object.h"
#include "snapshot.h"
#include <vector>

class State;
class StateMachine;
class Object;

enum StateReturnType
{
    STATE_FAILURE = -2,
    STATE_SUCCESS = -1,
    STATE_CONTINUE = 0,
};

enum StateExitType
{
    EXIT_NORMAL,
    EXIT_RESET,
};

enum
{
    EXIT_MACHINE_WITH_SUCCESS = 0x270e,
    EXIT_MACHINE_WITH_FAILURE = 0x270f,
};

enum
{
    MACHINE_DONE_STATE_ID = 0xf423e,
    INVALID_STATE_ID = 0xf423f,
};

struct StateConditionInfo
{
    bool (*test)(State *, void *);
    uint32_t to_state_id;
    void *user_data;
    StateConditionInfo(bool (*func)(State *, void *), uint32_t id, void *data) :
        test(func), to_state_id(id), user_data(data)
    {
    }
};

class State : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_ABSTRACT_POOL(State);

public:
    struct TransitionInfo
    {
        bool (*test)(State *, void *);
        uint32_t to_state_id;
        void *user_data;
#ifdef GAME_DEBUG_STRUCTS
        const char *description;
#endif
        TransitionInfo(bool (*func)(State *, void *), uint32_t id, void *data, const char *desc) :
            test(func), to_state_id(id), user_data(data)
        {
#ifdef GAME_DEBUG_STRUCTS
            description = desc;
#endif
        }
    };

    State(StateMachine *machine, Utf8String name);

    virtual ~State() override {}
    virtual StateReturnType On_Enter() { return STATE_CONTINUE; }
    virtual void On_Exit(StateExitType status) {}
    virtual StateReturnType Update() = 0;
    virtual bool Is_Idle() const { return false; }
    virtual bool Is_Attack() const { return false; }
    virtual bool Is_Guard_Idle() const { return false; }
    virtual bool Is_Busy() const { return false; }
#ifdef GAME_DEBUG_STRUCTS
    virtual Utf8String Get_Name() const { return m_name; }
#endif

    StateReturnType Friend_Check_For_Sleep_Transitions(StateReturnType status);
    StateReturnType Friend_Check_For_Transitions(StateReturnType status);
    void Friend_On_Condition(
        bool (*test)(State *, void *), uint32_t to_state_id, void *user_data, char const *description);
    Object *Get_Machine_Owner();
    const Object *Get_Machine_Owner() const;
    void Friend_On_Failure(uint32_t id) { m_failureStateID = id; }
    void Friend_On_Success(uint32_t id) { m_successStateID = id; }
    void Friend_Set_ID(uint32_t id) { m_ID = id; }
    uint32_t Get_ID() const { return m_ID; }
    std::vector<uint32_t> *Get_ID_Vector() const;
    Object *Get_Machine_Goal_Object();
    const Object *Get_Machine_Goal_Object() const;
    StateMachine *Get_Machine();

private:
    uint32_t m_ID;
    uint32_t m_successStateID;
    uint32_t m_failureStateID;
    std::vector<TransitionInfo> m_transitions;
    StateMachine *m_machine;
#ifdef GAME_DEBUG_STRUCTS
    Utf8String m_name;
#endif
};

class StateMachine : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_POOL(StateMachine);

public:
#ifdef GAME_DLL
    StateMachine *Hook_Ctor(Object *owner, Utf8String name) { return new (this) StateMachine(owner, name); }
    void Hook_Dtor() { StateMachine::~StateMachine(); }
#endif
    StateMachine(Object *owner, Utf8String name);

    virtual ~StateMachine() override;
    virtual StateReturnType Update_State_Machine();
    virtual void Clear();
    virtual StateReturnType Reset_To_Default_State();
    virtual StateReturnType Init_Default_State();
    virtual StateReturnType Set_State(uint32_t new_state_id);
    virtual void Halt();

    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    void Define_State(uint32_t id,
        State *state,
        uint32_t success_id,
        uint32_t failure_id,
        StateConditionInfo const *conditions);
    Object *Get_Goal_Object();
    Object const *Get_Goal_Object() const;
    void Internal_Clear();
    State *Internal_Get_State(uint32_t id);
    void Internal_Set_Goal_Object(Object const *obj);
    void Internal_Set_Goal_Position(Coord3D const *pos);
    StateReturnType Internal_Set_State(uint32_t new_state_id);
    bool Is_Goal_Object_Destroyed() const;
    void Set_Goal_Object(Object const *obj);
    void Set_Goal_Position(Coord3D const *pos);
    uint32_t Get_Current_State_ID() const;
    const Coord3D *Get_Goal_Position() const;
    Object *Get_Owner();
    Object const *Get_Owner() const;
    bool Is_In_Attack_State() const;
    bool Is_In_Guard_Idle_State() const;
    bool Is_Locked() const;
    void Lock(const char *string);
    void Unlock();

#ifdef GAME_DEBUG_STRUCTS
    bool Get_Wants_Debug_Output() const;
    Utf8String Get_Name() const;
#endif

private:
    std::map<uint32_t, State *> m_stateMap;
    Object *m_owner;
    uint32_t m_updateFrame;
    uint32_t m_defaultStateID;
    State *m_currentState;
    ObjectID m_goalObjectID;
    Coord3D m_goalPosition;
    bool m_locked;
    bool m_defaultStateInited;
#ifdef GAME_DEBUG_STRUCTS
    bool m_debugOutput;
    Utf8String m_name;
    const char *m_lockedString;
#endif
};
