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

ScriptList *ScriptList::s_readLists[16];
int ScriptList::s_numInReadList = 0;
ScriptGroup *ScriptList::s_emptyGroup = nullptr;

ScriptList::~ScriptList()
{
    Delete_Instance(m_firstGroup);
    m_firstGroup = nullptr;
    Delete_Instance(m_firstScript);
    m_firstScript = nullptr;
}

/**
 * @brief Uses the passed Xfer object to perform transfer of this script list.
 *
 * 0x0051B920
 */
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

/**
 * @brief Returns a pointer to a duplicate of the script list.
 *
 * 0x0051BC70
 */
ScriptList *ScriptList::Duplicate()
{
    ScriptList *new_list = new ScriptList;

    for (ScriptGroup *group = m_firstGroup, *new_group = nullptr; group != nullptr; group = group->Get_Next()) {
        ScriptGroup *duplicate = group->Duplicate();

        if (new_group != nullptr) {
            new_group->Set_Next(duplicate);
        } else {
            new_list->m_firstGroup = duplicate;
        }

        new_group = duplicate;
    }

    for (Script *script = m_firstScript, *new_script = nullptr; script != nullptr; script = script->Get_Next()) {
        Script *duplicate = script->Duplicate();

        if (new_script != nullptr) {
            new_script->Set_Next(duplicate);
        } else {
            new_list->m_firstScript = duplicate;
        }

        new_script = duplicate;
    }

    return new_list;
}

/**
 * @brief Returns a pointer to a duplicate of the script list, qualifying any parameters.
 *
 * 0x0051BD80
 */
ScriptList *ScriptList::Duplicate_And_Qualify(const Utf8String &str1, const Utf8String &str2, const Utf8String &str3)
{
    ScriptList *new_list = new ScriptList;

    for (ScriptGroup *group = m_firstGroup, *new_group = nullptr; group != nullptr; group = group->Get_Next()) {
        ScriptGroup *duplicate = group->Duplicate_And_Qualify(str1, str2, str3);

        if (new_group != nullptr) {
            new_group->Set_Next(duplicate);
        } else {
            new_list->m_firstGroup = duplicate;
        }

        new_group = duplicate;
    }

    for (Script *script = m_firstScript, *new_script = nullptr; script != nullptr; script = script->Get_Next()) {
        Script *duplicate = script->Duplicate_And_Qualify(str1, str2, str3);

        if (new_script != nullptr) {
            new_script->Set_Next(duplicate);
        } else {
            new_list->m_firstScript = duplicate;
        }

        new_script = duplicate;
    }

    return new_list;
}

/**
 * @brief Adds a script group at the requested point in the list, or at the end if the index is larger than the script count.
 */
void ScriptList::Add_Group(ScriptGroup *group, int index)
{
    ScriptGroup *position = nullptr;
    ScriptGroup *group_list = m_firstGroup;

    for (int i = index; i > 0; --i) {
        if (group_list == nullptr) {
            break;
        }

        position = group_list;
        group_list = group_list->Get_Next();
    }

    if (position != nullptr) {
        group->Set_Next(position->Get_Next());
        position->Set_Next(group);
    } else {
        group->Set_Next(m_firstGroup);
        m_firstGroup = group;
    }
}

/**
 * @brief Adds a script at the requested point in the list, or at the end if the index is larger than the script count.
 */
void ScriptList::Add_Script(Script *script, int index)
{
    Script *position = nullptr;
    Script *script_list = m_firstScript;

    for (int i = index; i > 0; --i) {
        if (script_list == nullptr) {
            break;
        }

        position = script_list;
        script_list = script_list->Get_Next();
    }

    if (position != nullptr) {
        script->Set_Next(position->Get_Next());
        position->Set_Next(script);
    } else {
        script->Set_Next(m_firstScript);
        m_firstScript = script;
    }
}

/**
 * @brief Moves the contents of the internal list of script lists created during parsing to the memory pointer provided.
 *
 * 0x0051C040
 */
int ScriptList::Get_Read_Scripts(ScriptList **scripts)
{
    for (int i = 0; i < s_numInReadList; ++i) {
        scripts[i] = s_readLists[i];
        s_readLists[i] = nullptr;
    }

    int retval = s_numInReadList;
    s_numInReadList = 0;

    return retval;
}

/**
 * @brief Parses a script list chunk from a data chunk stream.
 *
 * 0x0051C080
 */
bool ScriptList::Parse_Script_List_Chunk(DataChunkInput &input, DataChunkInfo *info, void *data)
{
    DEBUG_ASSERT_PRINT(static_cast<ScriptListReadInfo *>(data)->num_lists < MAX_LIST_COUNT, "Attempting to parse too many script lists.");
    ScriptListReadInfo *read_info = static_cast<ScriptListReadInfo *>(data);

    if (read_info->num_lists >= MAX_LIST_COUNT) {
        return false;
    }

    read_info->read_lists[read_info->num_lists++] = new ScriptList;
    input.Register_Parser("Script", info->label, Script::Parse_Script_From_List_Chunk, nullptr);
    input.Register_Parser("ScriptGroup", info->label, ScriptGroup::Parse_Group_Chunk, nullptr);
    
    return input.Parse(read_info->read_lists[read_info->num_lists - 1]);
}

/**
 * @brief Parses a scripts chunk from a data chunk stream.
 *
 * 0x0051BF00
 */
bool ScriptList::Parse_Scripts_Chunk(DataChunkInput &input, DataChunkInfo *info, void *data)
{
    input.Register_Parser("ScriptList", info->label, ScriptList::Parse_Script_List_Chunk, nullptr);

    for (int i = 0; i < s_numInReadList; ++i) {
        Delete_Instance(s_readLists[i]);
        s_readLists[i] = nullptr;
    }

    ScriptListReadInfo read_info;
    memset(read_info.read_lists, 0, sizeof(read_info.read_lists));
    read_info.num_lists = 0;

    if (input.Parse(&read_info)) {
        s_numInReadList = read_info.num_lists;

        if (s_numInReadList > 0) {
            memcpy(s_readLists, read_info.read_lists, s_numInReadList * sizeof(s_readLists[0]));
        }

        return true;
    }

    return false;
}

#ifdef GAME_DLL
void ScriptList::Hook_Xfer_Snapshot(Xfer *xfer)
{
    ScriptList::Xfer_Snapshot(xfer);
}
#endif
