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
#include "statemachine.h"
#include "gamelogic.h"
#include "globaldata.h"

#ifdef GAME_DEBUG_STRUCTS
#ifdef GAME_DLL
extern unsigned int &s_theObjectIDToDebug;
#else
extern unsigned int s_theObjectIDToDebug;
#endif
#endif

State::State(StateMachine *machine, Utf8String name) :
    m_ID(INVALID_STATE_ID), m_successStateID(INVALID_STATE_ID), m_failureStateID(INVALID_STATE_ID), m_machine(machine)
{
#ifdef GAME_DEBUG_STRUCTS
    m_name = name;
#endif
}

Object *State::Get_Machine_Owner()
{
    return m_machine->Get_Owner();
}

const Object *State::Get_Machine_Owner() const
{
    return m_machine->Get_Owner();
}

const Object *State::Get_Machine_Goal_Object() const
{
    return m_machine->Get_Goal_Object();
}

Object *State::Get_Machine_Goal_Object()
{
    return m_machine->Get_Goal_Object();
}

StateMachine *State::Get_Machine()
{
    return m_machine;
}

void State::Friend_On_Condition(
    bool (*test)(State *, void *), unsigned int to_state_id, void *user_data, char const *description)
{
    TransitionInfo info(test, to_state_id, user_data, description);
    m_transitions.push_back(info);
}

std::vector<unsigned int> *State::Get_ID_Vector() const
{
    std::vector<unsigned int> *vector = new std::vector<unsigned int>;
    vector->push_back(m_successStateID);
    vector->push_back(m_failureStateID);

    for (auto &info : m_transitions) {
        vector->push_back(info.to_state_id);
    }

    return vector;
}

struct StIncrementer
{
    int *value;
    StIncrementer(int *v) : value(v) { (*value)++; }
    ~StIncrementer() { (*value)--; }
};

StateReturnType State::Friend_Check_For_Sleep_Transitions(StateReturnType status)
{
    static int checkfortransitionsnum;
    StIncrementer incrementer(&checkfortransitionsnum);

    if (checkfortransitionsnum >= 20) {
        captainslog_dbgassert(checkfortransitionsnum < 20, "checkforsleeptransitionsnum is > 20");
        return STATE_FAILURE;
    } else {
        captainslog_dbgassert(status > STATE_CONTINUE, "Please only pass sleep states here");

        for (auto &info : m_transitions) {
            if (info.test(this, info.user_data)) {
#ifdef GAME_DEBUG_STRUCTS
                if (Get_Machine()->Get_Wants_Debug_Output()) {
                    const char *description;

                    if (info.description != nullptr) {
                        description = info.description;
                    } else {
                        description = "[no description]";
                    }

                    captainslog_debug("%d '%s' -- '%s' condition '%s' returned true!",
                        g_theGameLogic->Get_Frame(),
                        Get_Machine_Owner()->Get_Template()->Get_Name().Str(),
                        Get_Machine()->Get_Name().Str(),
                        description);
                }
#endif

                if (info.to_state_id == EXIT_MACHINE_WITH_SUCCESS) {
                    return STATE_SUCCESS;
                } else if (info.to_state_id == EXIT_MACHINE_WITH_FAILURE) {
                    return STATE_FAILURE;
                } else {
                    return Get_Machine()->Internal_Set_State(info.to_state_id);
                }
            }
        }

        return status;
    }
}

StateReturnType State::Friend_Check_For_Transitions(StateReturnType status)
{
    static int checkfortransitionsnum;
    StIncrementer incrementer(&checkfortransitionsnum);

    if (checkfortransitionsnum >= 20) {
        // assert disabled, the game ends up with checkfortransitionsnum >= 20
#if 0
        captainslog_dbgassert(checkfortransitionsnum < 20, "checkfortransitionsnum is > 20");
#endif
        return STATE_FAILURE;
    }

    captainslog_dbgassert(status <= STATE_CONTINUE, "Please handle sleep states prior to this");

    switch (status) {
        case STATE_FAILURE: {
            if (m_failureStateID == EXIT_MACHINE_WITH_SUCCESS) {
                Get_Machine()->Internal_Set_State(MACHINE_DONE_STATE_ID);
                return STATE_SUCCESS;
            } else if (m_failureStateID == EXIT_MACHINE_WITH_FAILURE) {
                Get_Machine()->Internal_Set_State(MACHINE_DONE_STATE_ID);
                return STATE_FAILURE;
            } else {
                return Get_Machine()->Internal_Set_State(m_failureStateID);
            }
        } break;

        case STATE_SUCCESS: {
            if (m_successStateID == EXIT_MACHINE_WITH_SUCCESS) {
                Get_Machine()->Internal_Set_State(MACHINE_DONE_STATE_ID);
                return STATE_SUCCESS;
            } else if (m_successStateID == EXIT_MACHINE_WITH_FAILURE) {
                Get_Machine()->Internal_Set_State(MACHINE_DONE_STATE_ID);
                return STATE_FAILURE;
            } else {
                return Get_Machine()->Internal_Set_State(m_successStateID);
            }
        } break;

        case STATE_CONTINUE: {
            for (auto &info : m_transitions) {
                if (info.test(this, info.user_data)) {
#ifdef GAME_DEBUG_STRUCTS
                    if (Get_Machine()->Get_Wants_Debug_Output()) {
                        const char *description;

                        if (info.description) {
                            description = info.description;
                        } else {
                            description = "[no description]";
                        }

                        captainslog_debug("%d '%s' -- '%s' condition '%s' returned true!",
                            g_theGameLogic->Get_Frame(),
                            Get_Machine_Owner()->Get_Template()->Get_Name().Str(),
                            Get_Machine()->Get_Name().Str(),
                            description);
                    }
#endif
                    if (info.to_state_id == EXIT_MACHINE_WITH_SUCCESS) {
                        return STATE_SUCCESS;
                    } else if (info.to_state_id == EXIT_MACHINE_WITH_FAILURE) {
                        return STATE_FAILURE;
                    } else {
                        return Get_Machine()->Internal_Set_State(info.to_state_id);
                    }
                }
            }
        } break;
    }

    return STATE_CONTINUE;
}

StateMachine::StateMachine(Object *owner, Utf8String name) :
    m_owner(owner),
    m_sleepTill(0),
    m_defaultStateID(INVALID_STATE_ID),
    m_currentState(nullptr),
    m_locked(false),
    m_defaultStateInited(false)
#ifdef GAME_DEBUG_STRUCTS
    ,
    m_debugOutput(false),
    m_name(name),
    m_lockedString(nullptr)
#endif
{
    Internal_Clear();
}

StateMachine::~StateMachine()
{
    if (m_currentState != nullptr) {
        m_currentState->On_Exit(EXIT_RESET);
    }

    for (auto &iter : m_stateMap) {
        if (iter.second != nullptr) {
            iter.second->Delete_Instance();
        }
    }
}

#ifdef GAME_DEBUG_STRUCTS
bool StateMachine::Get_Wants_Debug_Output() const
{
    if (m_debugOutput) {
        return true;
    }

    if (g_theWriteableGlobalData->m_stateMachineDebug) {
        return true;
    }

    if (s_theObjectIDToDebug != 0) {
        if (Get_Owner() != nullptr) {
            if (Get_Owner()->Get_ID() == s_theObjectIDToDebug) {
                return true;
            }
        }
    }

    return false;
}

Utf8String StateMachine::Get_Name() const
{
    return m_name;
}
#endif

void StateMachine::Internal_Clear()
{
    m_goalObjectID = INVALID_OBJECT_ID;
    m_goalPosition.x = 0.0f;
    m_goalPosition.y = 0.0f;
    m_goalPosition.z = 0.0f;
#ifdef GAME_DEBUG_STRUCTS
    if (Get_Wants_Debug_Output()) {
        captainslog_debug("%d '%s'%x -- '%s' %x Internal_Clear()",
            g_theGameLogic->Get_Frame(),
            m_owner->Get_Template()->Get_Name().Str(),
            m_owner,
            m_name.Str(),
            this);
    }
#endif
}

void StateMachine::Clear()
{
    if (m_locked) {
#ifdef GAME_DEBUG_STRUCTS
        if (m_currentState != nullptr) {
            captainslog_debug(" cur state '%s'", m_currentState->Get_Name().Str());
        }

        captainslog_debug("machine is locked (by %s), cannot be cleared (Please don't ignore; this generally indicates a "
                          "potential logic flaw)",
            m_lockedString);
#endif
    } else {
        if (m_currentState != nullptr) {
            m_currentState->On_Exit(EXIT_RESET);
            m_currentState = nullptr;
        }

        Internal_Clear();
    }
}

StateReturnType StateMachine::Reset_To_Default_State()
{
    if (m_locked) {
#ifdef GAME_DEBUG_STRUCTS
        if (m_currentState != nullptr) {
            captainslog_debug(" cur state '%s'", m_currentState->Get_Name().Str());
        }

        captainslog_debug("machine is locked (by %s), cannot be cleared (Please don't ignore; this generally indicates a "
                          "potential logic flaw)",
            m_lockedString);
#endif
        return STATE_FAILURE;
    } else if (m_defaultStateInited) {
        if (m_currentState != nullptr) {
            m_currentState->On_Exit(EXIT_RESET);
            m_currentState = nullptr;
        }

        Internal_Clear();
        StateReturnType state = Internal_Set_State(m_defaultStateID);
        captainslog_dbgassert(state != STATE_FAILURE, "StateMachine::Reset_To_Default_State() Error setting default state");
        return state;
    } else {
        captainslog_debug("you may not call Reset_To_Default_State before Init_Default_State");
        return STATE_FAILURE;
    }
}

StateReturnType StateMachine::Update_State_Machine()
{
    unsigned int frame = g_theGameLogic->Get_Frame();

    if (m_sleepTill != 0 && frame < m_sleepTill) {
        if (m_currentState != nullptr) {
            return m_currentState->Friend_Check_For_Sleep_Transitions((StateReturnType)(m_sleepTill - frame));
        } else {
            return STATE_FAILURE;
        }
    } else {
        m_sleepTill = 0;
        if (m_currentState != nullptr) {
            State *old_state = m_currentState;
            StateReturnType state = old_state->Update();

            if (m_currentState != nullptr) {
                if (old_state != m_currentState) {
                    state = STATE_CONTINUE;
                }

                if (state <= STATE_CONTINUE) {
                    return m_currentState->Friend_Check_For_Transitions(state);
                } else {
                    m_sleepTill = state + frame;
                    return m_currentState->Friend_Check_For_Sleep_Transitions((StateReturnType)(m_sleepTill - frame));
                }
            } else {
                return STATE_FAILURE;
            }
        } else {
            captainslog_debug("State machine has no current state -- did you remember to call Init_Default_State?");
            return STATE_FAILURE;
        }
    }
}

void StateMachine::Define_State(
    unsigned int id, State *state, unsigned int success_id, unsigned int failure_id, StateConditionInfo const *conditions)
{
#ifdef GAME_DEBUG_STRUCTS
    captainslog_dbgassert(m_stateMap.find(id) == m_stateMap.end(), "duplicate state ID in statemachine %s", m_name.Str());
#endif
    m_stateMap.insert(std::pair<unsigned int, State *>(id, state));
    state->Friend_Set_ID(id);
    state->Friend_On_Success(success_id);
    state->Friend_On_Failure(failure_id);

    while (conditions != nullptr && conditions->test != nullptr) {
        state->Friend_On_Condition(conditions->test, conditions->to_state_id, conditions->user_data, nullptr);
        conditions++;
    }

    if (m_defaultStateID == INVALID_STATE_ID) {
        m_defaultStateID = id;
    }
}

State *StateMachine::Internal_Get_State(unsigned int id)
{
    auto iter = m_stateMap.find(id);
    if (iter == m_stateMap.end()) {
        captainslog_debug("StateMachine::internalGetState(): Invalid state for object %s using state %d",
            m_owner->Get_Template()->Get_Name().Str(),
            id);
        captainslog_debug("Transisioning to state %d", id);
        captainslog_debug("Attempting to recover - locating default state...");

        iter = m_stateMap.find(m_defaultStateID);
        captainslog_relassert(iter != m_stateMap.end(), CODE_03, "Failed to located default state.  Aborting...");
        captainslog_debug("Located default state to recover.");
    }

    return iter->second;
}

StateReturnType StateMachine::Set_State(unsigned int new_state_id)
{
    if (!m_locked) {
        return Internal_Set_State(new_state_id);
    } else {
#ifdef GAME_DEBUG_STRUCTS
        if (m_currentState != nullptr) {
            captainslog_debug(" cur state '%s'", m_currentState->Get_Name().Str());
        }

        captainslog_debug("machine is locked (by %s), cannot be cleared (Please don't ignore; this generally indicates a "
                          "potential logic flaw)",
            m_lockedString);
#endif
        return STATE_CONTINUE;
    }
}

StateReturnType StateMachine::Internal_Set_State(unsigned int new_state_id)
{
    State *new_state = nullptr;
    m_sleepTill = 0;

    if (new_state_id != MACHINE_DONE_STATE_ID) {
        if (new_state_id == INVALID_STATE_ID) {
            new_state_id = m_defaultStateID;

            if (new_state_id == INVALID_STATE_ID) {
                captainslog_debug("you may NEVER set the current state to an invalid state id.");
                return STATE_FAILURE;
            }
        }

        new_state = Internal_Get_State(new_state_id);
#ifdef GAME_DEBUG_STRUCTS
        if (Get_Wants_Debug_Output()) {
            unsigned int id = INVALID_STATE_ID;

            if (m_currentState != nullptr) {
                id = m_currentState->Get_ID();
            }

            captainslog_debug(
                "%d '%s'%x -- '%s' %x exit ", g_theGameLogic->Get_Frame(), m_name.Str(), m_owner, m_name.Str(), this);

            if (m_currentState != nullptr) {
                captainslog_debug(" '%s' ", m_currentState->Get_Name().Str());
            } else {
                captainslog_debug(" INVALID_STATE_ID ");
            }

            if (new_state != nullptr) {
                captainslog_debug("enter '%s' ", new_state->Get_Name().Str());
            } else {
                captainslog_debug("to INVALID_STATE");
            }
        }
#endif
    }

    if (m_currentState != nullptr) {
        m_currentState->On_Exit(EXIT_NORMAL);
    }

    m_currentState = new_state;

    if (m_currentState == nullptr) {
        return STATE_CONTINUE;
    }

    State *old_state = m_currentState;
    StateReturnType state = m_currentState->On_Enter();

    if (!m_currentState) {
        return STATE_FAILURE;
    }

    if (old_state != m_currentState) {
        state = STATE_CONTINUE;
    }

    if (state <= STATE_CONTINUE) {
        return m_currentState->Friend_Check_For_Transitions(state);
    }

    unsigned int frame = g_theGameLogic->Get_Frame();
    m_sleepTill = state + frame;
    return m_currentState->Friend_Check_For_Sleep_Transitions((StateReturnType)(m_sleepTill - frame));
}

StateReturnType StateMachine::Init_Default_State()
{
#ifdef GAME_DEBUG_STRUCTS
    for (auto &iter : m_stateMap) {
        State *state = iter.second;
        unsigned int state_id = state->Get_ID();
        std::vector<unsigned int> *vector = state->Get_ID_Vector();

        for (auto &sid : *vector) {
            if (sid != INVALID_STATE_ID && sid != EXIT_MACHINE_WITH_SUCCESS && sid != EXIT_MACHINE_WITH_FAILURE) {
                auto iter2 = m_stateMap.find(state_id);
                if (iter2 == m_stateMap.end()) {
                    captainslog_debug("State %s(%d) : ", state->Get_Name().Str(), state_id);
                    captainslog_debug("Transition %d not found", sid);
                    captainslog_debug("This MUST BE FIXED!!!");
                    captainslog_debug("Invalid transition.");
                }
            }
        }

        delete vector;
        vector = nullptr;
    }
#endif

    captainslog_dbgassert(!m_locked, "Machine is locked here, but probably should not be");

    if (m_defaultStateInited) {
        captainslog_debug("you may not call initDefaultState twice for the same StateMachine");
        return STATE_FAILURE;
    } else {
        m_defaultStateInited = true;
        return Internal_Set_State(m_defaultStateID);
    }
}

void StateMachine::Set_Goal_Object(Object const *obj)
{
    if (!m_locked) {
        Internal_Set_Goal_Object(obj);
    }
}

bool StateMachine::Is_Goal_Object_Destroyed() const
{
    return m_goalObjectID != INVALID_OBJECT_ID && Get_Goal_Object() == nullptr;
}

void StateMachine::Halt()
{
    m_locked = true;
    m_currentState = nullptr;

#ifdef GAME_DEBUG_STRUCTS
    if (Get_Wants_Debug_Output()) {
        captainslog_debug("%d '%s' -- '%s' %x halt()",
            g_theGameLogic->Get_Frame(),
            m_owner->Get_Template()->Get_Name().Str(),
            m_name.Str(),
            this);
    }
#endif
}

void StateMachine::Internal_Set_Goal_Object(Object const *obj)
{
    if (obj) {
        m_goalObjectID = obj->Get_ID();
        Internal_Set_Goal_Position(obj->Get_Position());
    } else {
        m_goalObjectID = INVALID_OBJECT_ID;
    }
}

Object *StateMachine::Get_Goal_Object()
{
    return g_theGameLogic->Find_Object_By_ID(m_goalObjectID);
}

Object const *StateMachine::Get_Goal_Object() const
{
    return g_theGameLogic->Find_Object_By_ID(m_goalObjectID);
}

void StateMachine::Set_Goal_Position(Coord3D const *pos)
{
    if (!m_locked) {
        Internal_Set_Goal_Position(pos);
    }
}

void StateMachine::Internal_Set_Goal_Position(Coord3D const *pos)
{
    if (pos) {
        m_goalPosition = *pos;
    }
}

void StateMachine::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    xfer->xferUnsignedInt(&m_sleepTill);
    xfer->xferUnsignedInt(&m_defaultStateID);
    unsigned int id = Get_Current_State_ID();
    xfer->xferUnsignedInt(&id);

    if (xfer->Get_Mode() == XFER_LOAD) {
        m_currentState = Internal_Get_State(id);
    }

    bool has_state = false;
    xfer->xferBool(&has_state);

    if (has_state) {
        int count = 0;

        for (auto &iter : m_stateMap) {
            count++;
        }

        int count2 = count;
        xfer->xferInt(&count2);
        captainslog_dbgassert(count2 == count, "State count mismatch - %d expected, %d read", count, count2);

        for (auto &iter : m_stateMap) {
            State *state = iter.second;
            captainslog_assert(state != nullptr);
            unsigned int state_id = state->Get_ID();
            xfer->xferUnsignedInt(&state_id);

            captainslog_dbgassert(
                state_id == state->Get_ID(), "State ID mismatch - %d expected, %d read", state->Get_ID(), state_id);

            if (state == nullptr) {
                captainslog_dbgassert(0, "state was NULL on xfer, trying to heal...");
                state = Internal_Get_State(m_defaultStateID);
            }

            state->Xfer_Snapshot(xfer);
        }
    } else {
        if (m_currentState == nullptr) {
            captainslog_dbgassert(0, "currentState was NULL on xfer, trying to heal...");
            m_currentState = Internal_Get_State(m_defaultStateID);
        }

        m_currentState->Xfer_Snapshot(xfer);
    }

    xfer->xferObjectID(&m_goalObjectID);
    xfer->xferCoord3D(&m_goalPosition);
    xfer->xferBool(&m_locked);
    xfer->xferBool(&m_defaultStateInited);
}

unsigned int StateMachine::Get_Current_State_ID() const
{
    if (m_currentState != nullptr) {
        return m_currentState->Get_ID();
    } else {
        return INVALID_STATE_ID;
    }
}

const Coord3D *StateMachine::Get_Goal_Position() const
{
    return &m_goalPosition;
}

Object *StateMachine::Get_Owner()
{
    return m_owner;
}

Object const *StateMachine::Get_Owner() const
{
    return m_owner;
}

bool StateMachine::Is_In_Attack_State() const
{
    return m_currentState == nullptr || m_currentState->Is_Attack();
}

bool StateMachine::Is_In_Guard_Idle_State() const
{
    return m_currentState != nullptr && m_currentState->Is_Guard_Idle();
}

bool StateMachine::Is_Locked() const
{
    return m_locked;
}

void StateMachine::Lock(const char *string)
{
    m_locked = true;
#ifdef GAME_DEBUG_STRUCTS
    m_lockedString = string;
#endif
}

void StateMachine::Unlock()
{
    m_locked = false;
#ifdef GAME_DEBUG_STRUCTS
    m_lockedString = nullptr;
#endif
}

#ifdef GAME_DEBUG_STRUCTS
Utf8String StateMachine::Get_Current_State_Name()
{
    if (m_currentState != nullptr) {
        return m_currentState->Get_Name();
    } else {
        return Utf8String::s_emptyString;
    }
}
#endif
