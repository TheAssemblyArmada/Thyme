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
#pragma once

#ifndef SCRIPT_H
#define SCRIPT_H

#include "always.h"
#include "asciistring.h"
#include "mempoolobj.h"
#include "snapshot.h"

class OrCondition;
class ScriptAction;

class Script : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_POOL(Script);
public:
    Script();
    virtual ~Script();

    // Snapshot interface methods.
    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    Script *Get_Next() { return m_nextScript; }

    static Script *s_emptyScript;

private:
    AsciiString m_scriptName;
    AsciiString m_comment;
    AsciiString m_conditionComment;
    AsciiString m_actionComment;
    int m_unkInt1;
    bool m_isActive;
    bool m_isOneShot;
    bool m_isSubroutine;
    bool m_easy;
    bool m_normal;
    bool m_hard;
    OrCondition *m_condition;
    ScriptAction *m_action;
    ScriptAction *m_actionFalse;
    Script *m_nextScript;
    int m_unkInt2;
    bool m_hasWarnings;
    AsciiString m_conditionTeamName;
    int m_unkInt3;
    int m_unkInt4;
    int m_unkInt5;
};

#endif // SCRIPT_H
