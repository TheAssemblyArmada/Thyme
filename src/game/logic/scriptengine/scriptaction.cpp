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

ScriptAction::ScriptAction() :
    m_actionType(DEFAULT),
    m_numParams(0),
    m_nextAction(nullptr),
    m_hasWarnings(false)
{
    memset(m_params, 0, sizeof(m_params));
}

ScriptAction::~ScriptAction()
{
    for (int i = m_numParams; i < MAX_ACTION_PARAMETERS; ++i) {
        delete m_params[i];
        m_params[i] = nullptr;
    }

    ScriptAction *saved;
    for (ScriptAction *next = m_nextAction; next != nullptr; next = saved) {
        saved = next->m_nextAction;
        delete next;
        next = saved;
    }
}
