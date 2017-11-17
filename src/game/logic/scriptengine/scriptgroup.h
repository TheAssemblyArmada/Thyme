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
#pragma once

#ifndef SCRIPTGROUP_H
#define SCRIPTGROUP_H

#include "always.h"
#include "asciistring.h"
#include "mempoolobj.h"
#include "snapshot.h"

class Script;

class ScriptGroup : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_POOL(ScriptGroup);

public:
    ScriptGroup();
    ~ScriptGroup();

    // Snapshot interface methods.
    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

private:
    Script *m_firstScript;
    AsciiString m_groupName;
    bool m_isGroupActive;
    bool m_isGroupSubroutine;
    ScriptGroup *m_nextGroup;
    bool m_hasWarnings;

    static int s_curID;
};

#endif
