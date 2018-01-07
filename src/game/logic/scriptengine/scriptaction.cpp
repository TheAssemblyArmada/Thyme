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

ScriptAction::ScriptAction() : m_actionType(DEFAULT), m_numParams(0), m_nextAction(nullptr), m_hasWarnings(false)
{
    memset(m_params, 0, sizeof(m_params));
}

ScriptAction::ScriptAction(ScriptActionType type) :
    m_actionType(type),
    m_numParams(0),
    m_nextAction(nullptr),
    m_hasWarnings(false)
{
    memset(m_params, 0, sizeof(m_params));
    Set_Action_Type(type);
}

ScriptAction::~ScriptAction()
{
    for (int i = m_numParams; i < MAX_ACTION_PARAMETERS; ++i) {
        Delete_Instance(m_params[i]);
        m_params[i] = nullptr;
    }

    ScriptAction *saved;
    for (ScriptAction *next = m_nextAction; next != nullptr; next = saved) {
        saved = next->m_nextAction;
        Delete_Instance(next);
        next = saved;
    }
}

/**
 * @brief Returns a pointer to a duplicate of this action.
 *
 * 0x0051FF80
 */
ScriptAction *ScriptAction::Duplicate()
{
    ScriptAction *new_action = new ScriptAction(m_actionType);

    for (int i = 0; i < m_numParams; ++i) {
        new_action->m_params[i] = m_params[i];
    }

    ScriptAction *retval = new_action;

    for (ScriptAction *next = m_nextAction; next != nullptr; next = next->m_nextAction) {
        ScriptAction *new_next = new ScriptAction(next->m_actionType);

        new_action->m_nextAction = new_next;
        new_action = new_next;

        for (int i = 0; i < m_numParams; ++i) {
            *new_action->m_params[i] = *next->m_params[i];
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
ScriptAction *ScriptAction::Duplicate_And_Qualify(const AsciiString &str1, const AsciiString &str2, const AsciiString &str3)
{
    ScriptAction *new_action = new ScriptAction(m_actionType);

    for (int i = 0; i < m_numParams; ++i) {
        *new_action->m_params[i] = *m_params[i];
        new_action->m_params[i]->Qualify(str1, str2, str3);
    }

    ScriptAction *retval = new_action;

    for (ScriptAction *next = m_nextAction; next != nullptr; next = next->m_nextAction) {
        ScriptAction *new_next = new ScriptAction(next->m_actionType);

        new_action->m_nextAction = new_next;
        new_action = new_next;

        for (int i = 0; i < m_numParams; ++i) {
            *new_action->m_params[i] = *next->m_params[i];
            new_action->m_params[i]->Qualify(str1, str2, str3);
        }
    }

    return retval;
}

/**
 * @brief Get the UI text for this action.
 *
 * 0x005206B0
 */
AsciiString ScriptAction::Get_UI_Text()
{
    // TODO Requires ScriptEngine vtable
#ifndef THYME_STANDALONE
    return Call_Method<AsciiString, ScriptAction>(0x005206B0, this);
#else
    return AsciiString();
#endif
}

/**
 * @brief Parses an action from a datachunk stream.
 *
 * 0x00521240
 */
bool ScriptAction::Parse_Action_Chunk(DataChunkInput &input, DataChunkInfo *info, void *data)
{
    ScriptAction *paction = Parse_Action(input, info);
    ScriptAction *saction = static_cast<Script *>(data)->Get_Action();

    for (ScriptAction *next = saction; next != nullptr; next = next->m_nextAction) {
        if (next->m_nextAction == nullptr) {
            next->m_nextAction = paction;

            return true;
        }
    }

    static_cast<Script *>(data)->Set_Action(paction);

    return true;
}

/**
 * @brief Parses a false action from a datachunk stream.
 *
 * 0x00521280
 */
bool ScriptAction::Parse_False_Action_Chunk(DataChunkInput &input, DataChunkInfo *info, void *data)
{
    ScriptAction *paction = Parse_Action(input, info);
    ScriptAction *saction = static_cast<Script *>(data)->Get_False_Action();

    for (ScriptAction *next = saction; next != nullptr; next = next->m_nextAction) {
        if (next->m_nextAction == nullptr) {
            next->m_nextAction = paction;

            return true;
        }
    }

    static_cast<Script *>(data)->Set_False_Action(paction);

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
#ifndef THYME_STANDALONE
    Call_Method<void, ScriptAction, ScriptActionType>(0x0051FE50, this, type);
#endif
}

/**
 * @brief Internal function for parsing actions from datachunk streams.
 *
 * 0x005208A0
 */
ScriptAction *ScriptAction::Parse_Action(DataChunkInput &input, DataChunkInfo *info)
{
    // TODO Requires ScriptEngine vtable
#ifndef THYME_STANDALONE
    return Call_Function<ScriptAction *, DataChunkInput &, DataChunkInfo *>(0x005208A0, input, info);
#else
    return nullptr;
#endif
}
