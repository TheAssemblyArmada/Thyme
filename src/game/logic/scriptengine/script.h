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
    void Update_From(Script *source);
    void Delete_Or_Condition(OrCondition *condition);
    void Delete_Action(ScriptAction *action);
    void Delete_False_Action(ScriptAction *action);
    Utf8String Get_UI_Text();
    OrCondition *Find_Previous_Or_Condition(OrCondition *condition);

    Script *Get_Next() { return m_nextScript; }
    OrCondition *Get_Or_Condition() { return m_condition; }
    ScriptAction *Get_Action() { return m_action; }
    ScriptAction *Get_False_Action() { return m_actionFalse; }
    float Get_Total_Exec_Time() const { return m_totalExecTime; }
    int Get_Eval_Count() const { return m_evalCount; }
    Utf8String Get_Name() const { return m_scriptName; }
    unsigned int Get_Evaluation_Frame() const { return m_evaluationFrame; }
    int Get_Evaluation_Interval() const { return m_evaluationInterval; }
    Utf8String Get_Comment() const { return m_comment; }
    Utf8String Get_Action_Comment() const { return m_actionComment; }
    Utf8String Get_Condition_Comment() const { return m_conditionComment; }
    Utf8String Get_Condition_Team_Name() const { return m_conditionTeamName; }
    float Get_Script_Timing() const { return m_scriptTiming; }

    bool Is_Normal() const { return m_normal; }
    bool Is_Hard() const { return m_hard; }
    bool Is_Easy() const { return m_easy; }
    bool Is_One_Shot() const { return m_isOneShot; }
    bool Is_Subroutine() const { return m_isSubroutine; }
    bool Is_Active() const { return m_isActive; }
    bool Has_Warnings() const { return m_hasWarnings; }

    void Set_Next_Script(Script *next) { m_nextScript = next; }
    void Set_Or_Condition(OrCondition *condition) { m_condition = condition; }
    void Set_Action(ScriptAction *action) { m_action = action; }
    void Set_False_Action(ScriptAction *action) { m_actionFalse = action; }
    void Set_Evaluation_Frame(unsigned int set) { m_evaluationFrame = set; }
    void Set_Comment(Utf8String comment) { m_comment = comment; }
    void Set_Action_Comment(Utf8String comment) { m_actionComment = comment; }
    void Set_Condition_Comment(Utf8String comment) { m_conditionComment = comment; }
    void Set_Name(Utf8String name) { m_scriptName = name; }
    void Set_Warnings(bool set) { m_hasWarnings = set; }
    void Set_Active(bool set) { m_isActive = set; }
    void Set_One_Shot(bool set) { m_isOneShot = set; }
    void Set_Easy(bool set) { m_easy = set; }
    void Set_Normal(bool set) { m_normal = set; }
    void Set_Hard(bool set) { m_hard = set; }
    void Set_Is_Subroutine(bool set) { m_isSubroutine = set; }
    void Set_Evaluation_Interval(int32_t set) { m_evaluationInterval = set; }
    void Set_Script_Timing(float timing) { m_scriptTiming = timing; }
    void Set_Condition_Team_Name(Utf8String name) { m_conditionTeamName = name; }

    void Update_Exec_Time(float passed_time) { m_totalExecTime += passed_time; }
    void Inc_Eval_Count() { m_evalCount++; }

    static bool Parse_Script_From_Group_Data_Chunk(DataChunkInput &input, DataChunkInfo *info, void *data);
    static bool Parse_Script_From_List_Data_Chunk(DataChunkInput &input, DataChunkInfo *info, void *data);
    static void Write_Script_Data_Chunk(Script *script);

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
    unsigned int m_evaluationFrame;
    bool m_hasWarnings;
    Utf8String m_conditionTeamName;
    float m_totalExecTime;
    float m_scriptTiming;
    int m_evalCount;
};
