/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Object for handling scripts that should execute sequentially.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "sequentialscript.h"
#include "script.h"
#include "scriptengine.h"
#include "team.h"
#include "xfer.h"

SequentialScript::SequentialScript() :
    m_teamToExecOn(nullptr),
    m_objectID(INVALID_OBJECT_ID),
    m_scriptToExecuteSequentially(nullptr),
    m_currentInstruction(-1),
    m_timesToLoop(0),
    m_framesToWait(-1),
    m_unkbool1(false),
    m_nextScriptInSequence(nullptr)
{
}

void SequentialScript::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    unsigned int id;
    if (m_teamToExecOn != nullptr) {
        id = m_teamToExecOn->Get_Team_ID();
    } else {
        id = 0;
    }

    xfer->xferUser(&id, sizeof(id));

    if (xfer->Get_Mode() == XFER_LOAD) {
        m_teamToExecOn = g_theTeamFactory->Find_Team_By_ID(id);

        if (id != 0) {
            captainslog_relassert(m_teamToExecOn != nullptr,
                6,
                "SequentialScript::xfer - Unable to find team by ID (#%d) for m_teamToExecOn",
                id);
        }
    }

    xfer->xferObjectID(&m_objectID);
    Utf8String name;

    if (xfer->Get_Mode() == XFER_SAVE) {
        name = m_scriptToExecuteSequentially->Get_Name();
        xfer->xferAsciiString(&name);
    } else {
        xfer->xferAsciiString(&name);
        captainslog_dbgassert(
            m_scriptToExecuteSequentially == nullptr, "SequentialScript::xfer - m_scripttoExecuteSequentially");
        m_scriptToExecuteSequentially = g_theScriptEngine->Find_Script_By_Name(name);
        captainslog_dbgassert(m_scriptToExecuteSequentially != nullptr,
            "SequentialScript::xfer - m_scriptToExecuteSequentially is NULL but should not be");
    }

    xfer->xferInt(&m_currentInstruction);
    xfer->xferInt(&m_timesToLoop);
    xfer->xferInt(&m_framesToWait);
    xfer->xferBool(&m_unkbool1);
}
