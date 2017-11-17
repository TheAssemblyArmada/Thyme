/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Class for maintaining groups of scripts.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "scriptgroup.h"
#include "script.h"
#include "xfer.h"

int ScriptGroup::s_curID = 0;

ScriptGroup::ScriptGroup() :
    m_firstScript(nullptr),
    m_groupName(),
    m_isGroupActive(true),
    m_isGroupSubroutine(false),
    m_nextGroup(nullptr),
    m_hasWarnings(false)
{
    m_groupName.Format("Script Group %d", ++s_curID);
}

ScriptGroup::~ScriptGroup()
{
    delete m_firstScript;
    m_firstScript = nullptr;

    ScriptGroup *saved;
    for (ScriptGroup *next = m_nextGroup; next != nullptr; next = saved) {
        saved = next->m_nextGroup;
        next->m_nextGroup = nullptr;
        delete next;
        next = saved;
    }
}

void ScriptGroup::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t version = 2;
    xfer->xferVersion(&version, 2);
    xfer->xferBool(&m_isGroupActive);

    uint16_t script_count = 0;
    for (Script *next = m_firstScript; next != nullptr; next = next->Get_Next()) {
        ++script_count;
    }

    xfer->xferUnsignedShort(&script_count);

    for (Script *next = m_firstScript; next != nullptr; next = next->Get_Next()) {
        xfer->xferSnapshot(next);

        if (--script_count == 0) {
            break;
        }
    }

    if (script_count > 0) {
        if (Script::s_emptyScript == nullptr) {
            Script::s_emptyScript = new Script;
        }

        while (script_count--) {
            xfer->xferSnapshot(Script::s_emptyScript);
        }
    }
}
