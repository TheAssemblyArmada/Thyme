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
#include "scriptlist.h"
#include "xfer.h"

ScriptGroup::ScriptGroup() :
    m_firstScript(nullptr),
    m_groupName(),
    m_isGroupActive(true),
    m_isGroupSubroutine(false),
    m_nextGroup(nullptr),
    m_hasWarnings(false)
{
    m_groupName.Format("Script Group %d", ScriptList::Get_Next_ID());
}

ScriptGroup::~ScriptGroup()
{
    m_firstScript->Delete_Instance();
    m_firstScript = nullptr;

    ScriptGroup *saved;
    for (ScriptGroup *next = m_nextGroup; next != nullptr; next = saved) {
        saved = next->m_nextGroup;
        next->m_nextGroup = nullptr;
        next->Delete_Instance();
        next = saved;
    }
}

/**
 * @brief Uses the passed Xfer object to perform transfer of this script group.
 *
 * 0x0051C3B0
 */
void ScriptGroup::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t version = 2;
    xfer->xferVersion(&version, 2);

    if (version >= 2) {
        xfer->xferBool(&m_isGroupActive);
    }

    uint16_t script_count = 0;
    for (Script *next = m_firstScript; next != nullptr; next = next->Get_Next()) {
        ++script_count;
    }

    auto dbg_script_count = script_count;
    xfer->xferUnsignedShort(&script_count);
    captainslog_dbgassert(dbg_script_count == script_count,
        "ScriptGroup::Xfer_Snapshot - Script list count has changed, attempting to recover.");

    for (Script *next = m_firstScript; next != nullptr; next = next->Get_Next()) {
        xfer->xferSnapshot(next);

        if (--script_count == 0) {
            break;
        }
    }

    if (script_count > 0) {
        captainslog_dbgassert(false, "Striping out extra scripts - Bad...");
        if (Script::s_emptyScript == nullptr) {
            Script::s_emptyScript = NEW_POOL_OBJ(Script);
        }

        while (script_count--) {
            xfer->xferSnapshot(Script::s_emptyScript);
        }
    }
}

/**
 * @brief Returns a pointer to a duplicate of the script group.
 *
 * 0x0051C510
 */
ScriptGroup *ScriptGroup::Duplicate()
{
    ScriptGroup *new_group = NEW_POOL_OBJ(ScriptGroup);

    for (Script *script = m_firstScript, *new_script = nullptr; script != nullptr; script = script->Get_Next()) {
        Script *duplicate = script->Duplicate();

        if (new_script != nullptr) {
            new_script->Set_Next_Script(duplicate);
        } else {
            new_group->m_firstScript = duplicate;
        }

        new_script = duplicate;
    }

    new_group->m_groupName = m_groupName;
    new_group->m_isGroupActive = m_isGroupActive;
    new_group->m_isGroupSubroutine = m_isGroupSubroutine;
    new_group->m_nextGroup = nullptr;

    return new_group;
}

/**
 * @brief Returns a pointer to a duplicate of the script group, qualifying any parameters.
 *
 * 0x0051C670
 */
ScriptGroup *ScriptGroup::Duplicate_And_Qualify(const Utf8String &str1, const Utf8String &str2, const Utf8String &str3)
{
    ScriptGroup *new_group = NEW_POOL_OBJ(ScriptGroup);

    for (Script *script = m_firstScript, *new_script = nullptr; script != nullptr; script = script->Get_Next()) {
        Script *duplicate = script->Duplicate_And_Qualify(str1, str2, str3);

        if (new_script != nullptr) {
            new_script->Set_Next_Script(duplicate);
        } else {
            new_group->m_firstScript = duplicate;
        }

        new_script = duplicate;
    }

    new_group->m_groupName = m_groupName + str1;
    new_group->m_isGroupActive = m_isGroupActive;
    new_group->m_isGroupSubroutine = m_isGroupSubroutine;
    new_group->m_nextGroup = nullptr;

    return new_group;
}

/**
 * @brief Adds a script at the requested point in the list, or at the end if the index is larger than the script count.
 */
void ScriptGroup::Add_Script(Script *script, int index)
{
    Script *position = nullptr;
    Script *script_list = m_firstScript;
    captainslog_dbgassert(script->Get_Next() == nullptr, "Adding already linked group.");

    for (int i = index; i > 0; --i) {
        if (script_list == nullptr) {
            break;
        }

        position = script_list;
        script_list = script_list->Get_Next();
    }

    if (position != nullptr) {
        script->Set_Next_Script(position->Get_Next());
        position->Set_Next_Script(script);
    } else {
        script->Set_Next_Script(m_firstScript);
        m_firstScript = script;
    }
}

/**
 * @brief Parses a script group chunk from a data chunk stream.
 *
 * 0x0051C860
 */
bool ScriptGroup::Parse_Group_Data_Chunk(DataChunkInput &input, DataChunkInfo *info, void *data)
{
    ScriptGroup *new_group = NEW_POOL_OBJ(ScriptGroup);

    new_group->m_groupName = input.Read_AsciiString();
    new_group->m_isGroupActive = input.Read_Byte() != 0;

    if (info->version >= 2) {
        new_group->m_isGroupSubroutine = input.Read_Byte() != 0;
    }

    static_cast<ScriptList *>(data)->Add_Group(new_group, 0xFFFFFF);
    input.Register_Parser("Script", info->label, Script::Parse_Script_From_Group_Data_Chunk, nullptr);

    return input.Parse(new_group);
}
