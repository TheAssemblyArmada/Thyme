/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Class representing a script action.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "scriptaction.h"
#include "script.h"

ScriptAction::ScriptAction() : m_actionType(NO_OP), m_numParams(0), m_nextAction(nullptr), m_hasWarnings(false)
{
    memset(m_params, 0, sizeof(m_params));
}

ScriptAction::ScriptAction(ScriptActionType type) :
    m_actionType(type), m_numParams(0), m_nextAction(nullptr), m_hasWarnings(false)
{
    memset(m_params, 0, sizeof(m_params));
    Set_Action_Type(type);
}

ScriptAction::~ScriptAction()
{
    // Clear our paramter instances.
    for (int i = 0; i < m_numParams; ++i) {
        m_params[i]->Delete_Instance();
        m_params[i] = nullptr;
    }

    // Clear our list of action instances.
    for (ScriptAction *next = m_nextAction, *saved = nullptr; next != nullptr; next = saved) {
        saved = next->m_nextAction;
        next->m_nextAction = nullptr; // Prevent trying to next object twice
        next->Delete_Instance();
    }
}

/**
 * @brief Returns a pointer to a duplicate of this action.
 *
 * 0x0051FF80
 */
ScriptAction *ScriptAction::Duplicate()
{
    ScriptAction *new_action = NEW_POOL_OBJ(ScriptAction, m_actionType);

    for (int i = 0; i < m_numParams; ++i) {
        if (new_action->m_params[i] != nullptr) {
            *new_action->m_params[i] = *m_params[i];
        }
    }

    ScriptAction *retval = new_action;

    for (ScriptAction *next = m_nextAction; next != nullptr; next = next->m_nextAction) {
        ScriptAction *new_next = NEW_POOL_OBJ(ScriptAction, next->m_actionType);

        new_action->m_nextAction = new_next;
        new_action = new_next;

        for (int i = 0; i < next->m_numParams; ++i) {
            if (new_action->m_params[i] != nullptr && next->m_params[i] != nullptr) {
                *new_action->m_params[i] = *next->m_params[i];
            }
        }
    }

    return retval;
}

/**
 * @brief Returns a pointer to a duplicate of this action, qualifying any parameters.
 *
 * @see Parameter::Qualify
 *
 * 0x00520240
 */
ScriptAction *ScriptAction::Duplicate_And_Qualify(const Utf8String &str1, const Utf8String &str2, const Utf8String &str3)
{
    ScriptAction *new_action = NEW_POOL_OBJ(ScriptAction, m_actionType);

    for (int i = 0; i < m_numParams; ++i) {
        if (new_action->m_params[i] != nullptr) {
            *new_action->m_params[i] = *m_params[i];
            new_action->m_params[i]->Qualify(str1, str2, str3);
        }
    }

    ScriptAction *current_action = new_action;

    for (ScriptAction *next = m_nextAction; next != nullptr; next = next->m_nextAction) {
        ScriptAction *new_next = NEW_POOL_OBJ(ScriptAction, next->m_actionType);

        current_action->m_nextAction = new_next;
        current_action = current_action->m_nextAction;

        for (int i = 0; i < next->m_numParams; ++i) {
            if (current_action->m_params[i] != nullptr && next->m_params[i] != nullptr) {
                *current_action->m_params[i] = *next->m_params[i];
                current_action->m_params[i]->Qualify(str1, str2, str3);
            }
        }
    }

    return new_action;
}

/**
 * @brief Get the UI text for this action.
 *
 * 0x005206B0
 */
Utf8String ScriptAction::Get_UI_Text()
{
    // TODO Requires ScriptEngine vtable
#ifdef GAME_DLL
    return Call_Method<Utf8String, ScriptAction>(PICK_ADDRESS(0x005206B0, 0x006FAA16), this);
#else
    return Utf8String();
#endif
}

/**
 * @brief Parses an action from a datachunk stream.
 *
 * 0x00521240
 */
bool ScriptAction::Parse_Action_Chunk(DataChunkInput &input, DataChunkInfo *info, void *data)
{
    ScriptAction *paction = Parse_Action(input, info, data);
    ScriptAction *saction = static_cast<Script *>(data)->Get_Action();

    for (ScriptAction *next = saction; next != nullptr; next = next->m_nextAction) {
        if (next->m_nextAction == nullptr) {
            next->m_nextAction = paction;

            return true;
        }
    }

    static_cast<Script *>(data)->Set_Action(paction);

    captainslog_dbgassert(input.At_End_Of_Chunk(), "Unexpected data left over.");

    return true;
}

/**
 * @brief Parses a false action from a datachunk stream.
 *
 * 0x00521280
 */
bool ScriptAction::Parse_False_Action_Chunk(DataChunkInput &input, DataChunkInfo *info, void *data)
{
    ScriptAction *paction = Parse_Action(input, info, data);
    ScriptAction *saction = static_cast<Script *>(data)->Get_False_Action();

    for (ScriptAction *next = saction; next != nullptr; next = next->m_nextAction) {
        if (next->m_nextAction == nullptr) {
            next->m_nextAction = paction;

            return true;
        }
    }

    static_cast<Script *>(data)->Set_False_Action(paction);

    captainslog_dbgassert(input.At_End_Of_Chunk(), "Unexpected data left over.");

    return true;
}

/**
 * @brief Sets the type of the action.
 *
 * 0x0051FE50
 */
void ScriptAction::Set_Action_Type(ScriptActionType type)
{
    // TODO Requires ScriptEngine vtable
#ifdef GAME_DLL
    Call_Method<void, ScriptAction, ScriptActionType>(PICK_ADDRESS(0x0051FE50, 0x006FA4A5), this, type);
#endif
}

/**
 * @brief Internal function for parsing actions from datachunk streams.
 *
 * 0x005208A0
 */
ScriptAction *ScriptAction::Parse_Action(DataChunkInput &input, DataChunkInfo *info, void *data)
{
    // TODO Requires ScriptEngine vtable
#ifdef GAME_DLL
    return Call_Function<ScriptAction *, DataChunkInput &, DataChunkInfo *, void *>(
        PICK_ADDRESS(0x005208A0, 0), input, info, data);
#else
    return nullptr;
#endif
}
