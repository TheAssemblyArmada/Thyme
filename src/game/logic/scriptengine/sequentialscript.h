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
#pragma once

#include "always.h"
#include "gametype.h"
#include "mempoolobj.h"
#include "snapshot.h"

class Script;
class Team;

class SequentialScript : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_POOL(SequentialScript);
    friend class ScriptEngine;

public:
    SequentialScript();
    virtual ~SequentialScript() override {}

    // Snapshot virtuals
    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer);
    virtual void Load_Post_Process() override {}

private:
    Team *m_teamToExecOn;
    ObjectID m_objectID;
    Script *m_scriptToExecuteSequentially;
    int m_currentInstruction;
    int m_timesToLoop;
    int m_framesToWait;
    bool m_unkbool1; // Used in ScriptEngine::evaluateAndProgressAllSequentialScripts
    SequentialScript *m_nextScriptInSequence;
};
