/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Class representing a script object.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "script.h"
#include "xfer.h"
#include "scriptaction.h"
#include "scriptcondition.h"

Script *Script::s_emptyScript = nullptr;

Script::Script() :
    m_scriptName(),
    m_comment(),
    m_conditionComment(),
    m_actionComment(),
    m_unkInt1(0),
    m_isActive(true),
    m_isOneShot(true),
    m_isSubroutine(false),
    m_easy(true),
    m_normal(true),
    m_hard(true),
    m_condition(nullptr),
    m_action(nullptr),
    m_actionFalse(nullptr),
    m_nextScript(nullptr),
    m_unkInt2(0),
    m_hasWarnings(false),
    m_conditionTeamName(),
    m_unkInt3(0),
    m_unkInt4(0),
    m_unkInt5(0)
{

}

Script::~Script()
{
    Script *saved;
    for (Script *next = m_nextScript; next != nullptr; next = saved) {
        saved = next->m_nextScript;
        next->m_nextScript = nullptr;
        Delete_Instance(next);
        next = saved;
    }

    Delete_Instance(m_condition);
    Delete_Instance(m_action);
    Delete_Instance(m_actionFalse);
}

void Script::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t version = 1;
    xfer->xferVersion(&version, 1);
    xfer->xferBool(&m_isActive);
}
