/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Classes for managing scripts.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "scriptlist.h"
#include "script.h"
#include "scriptgroup.h"
#include "xfer.h"

int ScriptList::s_numInReadList = 0;
ScriptGroup *ScriptList::s_emptyGroup = nullptr;

ScriptList::~ScriptList()
{
    Delete_Instance(m_firstGroup);
    m_firstGroup = nullptr;
    Delete_Instance(m_firstScript);
    m_firstScript = nullptr;
}

void ScriptList::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t version = 1;
    xfer->xferVersion(&version, 1);

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

    uint16_t group_count = 0;
    for (ScriptGroup *next = m_firstGroup; next != nullptr; next = next->Get_Next()) {
        ++group_count;
    }

    xfer->xferUnsignedShort(&group_count);

    for (ScriptGroup *next = m_firstGroup; next != nullptr; next = next->Get_Next()) {
        xfer->xferSnapshot(next);

        if (--group_count == 0) {
            break;
        }
    }

    if (group_count > 0) {
        if (ScriptList::s_emptyGroup == nullptr) {
            ScriptList::s_emptyGroup = new ScriptGroup;
        }

        while (group_count--) {
            xfer->xferSnapshot(ScriptList::s_emptyGroup);
        }
    }
}
