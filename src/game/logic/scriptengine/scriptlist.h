/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Class for maintaining a list of scripts.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#ifndef SCRIPTLIST_H
#define SCRIPTLIST_H

#include "always.h"
#include "mempoolobj.h"
#include "script.h"
#include "scriptgroup.h"
#include "snapshot.h"

class ScriptList : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_POOL(ScriptList)
public:
    ScriptList() : m_firstGroup(nullptr), m_firstScript(nullptr) {}
    virtual ~ScriptList();

    // Snapshot interface methods.
    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    Script *Get_Scripts() { return m_firstScript; }
    ScriptGroup *Get_Groups() { return m_firstGroup; }

private:
    ScriptGroup *m_firstGroup;
    Script *m_firstScript;
    static int s_numInReadList;
    static ScriptGroup *s_emptyGroup;
};

#endif // SCRIPTLIST_H
