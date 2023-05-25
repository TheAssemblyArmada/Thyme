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
#include "scriptaction.h"
#include "scriptcondition.h"
#include "scriptgroup.h"
#include "scriptlist.h"
#include "xfer.h"
#include <captainslog.h>

Script *Script::s_emptyScript = nullptr;

Script::Script() :
    m_scriptName(),
    m_comment(),
    m_conditionComment(),
    m_actionComment(),
    m_evaluationInterval(0),
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
    m_evaluationFrame(0),
    m_hasWarnings(false),
    m_conditionTeamName(),
    m_totalExecTime(0.0f),
    m_scriptTiming(0),
    m_evalCount(0)
{
}

Script::~Script()
{
    Script *saved;
    for (Script *next = m_nextScript; next != nullptr; next = saved) {
        saved = next->m_nextScript;
        next->m_nextScript = nullptr;
        next->Delete_Instance();
        next = saved;
    }

    m_condition->Delete_Instance();
    m_action->Delete_Instance();
    m_actionFalse->Delete_Instance();
}

/**
 * @brief Uses the passed Xfer object to perform transfer of this script.
 *
 * 0x0051CD70
 */
void Script::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t version = 1;
    xfer->xferVersion(&version, 1);
    xfer->xferBool(&m_isActive);
}

/**
 * @brief Returns a pointer to a duplicate of the script.
 *
 * 0x0051CDB0
 */
Script *Script::Duplicate()
{
    Script *new_script = NEW_POOL_OBJ(Script);

    // Original calls these deletes here, but the ctor sets them null anyhow...
    // new_script->m_condition->Delete_Instance();
    // new_script->m_action->Delete_Instance();

    new_script->m_scriptName = m_scriptName;
    new_script->m_comment = m_comment;
    new_script->m_conditionComment = m_conditionComment;
    new_script->m_actionComment = m_actionComment;
    new_script->m_isActive = m_isActive;
    new_script->m_isOneShot = m_isOneShot;
    new_script->m_isSubroutine = m_isSubroutine;
    new_script->m_easy = m_easy;
    new_script->m_normal = m_normal;
    new_script->m_hard = m_hard;
    new_script->m_evaluationInterval = m_evaluationInterval;

    if (m_condition) {
        new_script->m_condition = m_condition->Duplicate();
    }

    if (m_action) {
        new_script->m_action = m_action->Duplicate();
    }

    if (m_actionFalse) {
        new_script->m_actionFalse = m_actionFalse->Duplicate();
    }

    return new_script;
}

/**
 * @brief Returns a pointer to a duplicate of the script, qualifying any parameters.
 *
 * 0x0051CFC0
 */
Script *Script::Duplicate_And_Qualify(const Utf8String &str1, const Utf8String &str2, const Utf8String &str3)
{
    Script *new_script = NEW_POOL_OBJ(Script);

    // Original calls these deletes here, but the ctor sets them null anyhow...
    // new_script->m_condition->Delete_Instance();
    // new_script->m_action->Delete_Instance();

    new_script->m_scriptName = m_scriptName + str1;
    new_script->m_comment = m_comment;
    new_script->m_conditionComment = m_conditionComment;
    new_script->m_actionComment = m_actionComment;
    new_script->m_isActive = m_isActive;
    new_script->m_isOneShot = m_isOneShot;
    new_script->m_isSubroutine = m_isSubroutine;
    new_script->m_easy = m_easy;
    new_script->m_normal = m_normal;
    new_script->m_hard = m_hard;
    new_script->m_evaluationInterval = m_evaluationInterval;

    if (m_condition) {
        new_script->m_condition = m_condition->Duplicate_And_Qualify(str1, str2, str3);
    }

    if (m_action) {
        new_script->m_action = m_action->Duplicate_And_Qualify(str1, str2, str3);
    }

    if (m_actionFalse) {
        new_script->m_actionFalse = m_actionFalse->Duplicate_And_Qualify(str1, str2, str3);
    }

    return new_script;
}

/**
 * @brief Parses a script from a script group chunk.
 *
 * 0x0051D610
 */
bool Script::Parse_Script_From_Group_Data_Chunk(DataChunkInput &input, DataChunkInfo *info, void *data)
{
    Script *script = Parse_Script(input, info->version);
    static_cast<ScriptGroup *>(data)->Add_Script(script, 0xFFFFFF);

    captainslog_dbgassert(input.At_End_Of_Chunk(), "Unexpected data left over.");

    return true;
}

/**
 * @brief Parses a script from a script list chunk.
 *
 * 0x0051D5B0
 */
bool Script::Parse_Script_From_List_Data_Chunk(DataChunkInput &input, DataChunkInfo *info, void *data)
{
    Script *script = Parse_Script(input, info->version);
    static_cast<ScriptList *>(data)->Add_Script(script, 0xFFFFFF);

    captainslog_dbgassert(input.At_End_Of_Chunk(), "Unexpected data left over.");

    return true;
}

/**
 * @brief Parses a script from a datachunk stream.
 *
 * 0x0051D200
 */
Script *Script::Parse_Script(DataChunkInput &input, uint16_t version)
{
    // Log if the version is not the normal chunk version seen in ZH.
    if (version != 2) {
        captainslog_info("Parsing script chunk for version %d.", version);
    }

    Script *new_script = NEW_POOL_OBJ(Script);
    new_script->m_scriptName = input.Read_AsciiString();
    new_script->m_comment = input.Read_AsciiString();
    new_script->m_conditionComment = input.Read_AsciiString();
    new_script->m_actionComment = input.Read_AsciiString();
    new_script->m_isActive = input.Read_Byte() != 0;
    new_script->m_isOneShot = input.Read_Byte() != 0;
    new_script->m_easy = input.Read_Byte() != 0;
    new_script->m_normal = input.Read_Byte() != 0;
    new_script->m_hard = input.Read_Byte() != 0;
    new_script->m_isSubroutine = input.Read_Byte() != 0;

    // Scripts version 2 and above appear to have this extra integer.
    if (version >= 2) {
        new_script->m_evaluationInterval = input.Read_Int32();
    }

    input.Register_Parser("OrCondition", "Script", OrCondition::Parse_OrCondition_Chunk, nullptr);
    input.Register_Parser("ScriptAction", "Script", ScriptAction::Parse_Action_Data_Chunk, nullptr);
    input.Register_Parser("ScriptActionFalse", "Script", ScriptAction::Parse_Action_False_Data_Chunk, nullptr);

    if (!input.Parse(new_script)) {
        return nullptr;
    }

    captainslog_dbgassert(input.At_End_Of_Chunk(), "Unexpected data left over.");

    return new_script;
}
