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

#include "always.h"
#include "asciistring.h"
#include "mempoolobj.h"
#include "scriptaction.h"
#include "scriptcondition.h"
#include "snapshot.h"

class Script : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_POOL(Script);

protected:
    virtual ~Script() override;

public:
    Script();

    // Snapshot interface methods.
    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    Script *Duplicate();
    Script *Duplicate_And_Qualify(const Utf8String &str1, const Utf8String &str2, const Utf8String &str3);
    Script *Get_Next() { return m_nextScript; }
    OrCondition *Get_Condition() { return m_condition; }
    ScriptAction *Get_Action() { return m_action; }
    ScriptAction *Get_False_Action() { return m_actionFalse; }
    float Get_Total_Exec_Time() const { return m_totalExecTime; }
    int Get_Evalulation_Count() const { return m_evalCount; }
    Utf8String Get_Name() const { return m_scriptName; }
    void Set_Next(Script *next) { m_nextScript = next; }
    void Set_Condition(OrCondition *condition) { m_condition = condition; }
    void Set_Action(ScriptAction *action) { m_action = action; }
    void Set_False_Action(ScriptAction *action) { m_actionFalse = action; }
    void Update_Exec_Time(float passed_time) { m_totalExecTime += passed_time; }

    static bool Parse_Script_From_Group_Chunk(DataChunkInput &input, DataChunkInfo *info, void *data);
    static bool Parse_Script_From_List_Chunk(DataChunkInput &input, DataChunkInfo *info, void *data);

private:
    static Script *Parse_Script(DataChunkInput &input, uint16_t version);

public:
    static Script *s_emptyScript;

private:
    Utf8String m_scriptName;
    Utf8String m_comment;
    Utf8String m_conditionComment;
    Utf8String m_actionComment;
    int32_t m_evaluationInterval;
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
    Utf8String m_conditionTeamName;
    float m_totalExecTime;
    int m_scriptTiming;
    int m_evalCount;
};
