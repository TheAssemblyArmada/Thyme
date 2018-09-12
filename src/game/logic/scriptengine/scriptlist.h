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

#include "always.h"
#include "mempoolobj.h"
#include "script.h"
#include "scriptgroup.h"
#include "snapshot.h"

class ScriptList : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_POOL(ScriptList);

    enum
    {
        MAX_LIST_COUNT = 16,
    };

    struct ScriptListReadInfo
    {
        int num_lists;
        ScriptList *read_lists[MAX_LIST_COUNT];
    };
public:
    ScriptList() : m_firstGroup(nullptr), m_firstScript(nullptr) {}
    virtual ~ScriptList();

    // Snapshot interface methods.
    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    ScriptList *Duplicate();
    ScriptList *Duplicate_And_Qualify(const Utf8String &str1, const Utf8String &str2, const Utf8String &str3);
    void Add_Group(ScriptGroup *group, int index);
    void Add_Script(Script *script, int index);
    Script *Get_Scripts() { return m_firstScript; }
    ScriptGroup *Get_Groups() { return m_firstGroup; }

    static int Get_Read_Scripts(ScriptList **scripts);
    static bool Parse_Script_List_Chunk(DataChunkInput &input, DataChunkInfo *info, void *data);
    static bool Parse_Scripts_Chunk(DataChunkInput &input, DataChunkInfo *info, void *data);

#ifndef THYME_STANDALONE
    void Hook_Xfer_Snapshot(Xfer *xfer);
    static void Hook_Me();
#endif

private:
    ScriptGroup *m_firstGroup;
    Script *m_firstScript;
    static ScriptList *s_readLists[MAX_LIST_COUNT];
    static int s_numInReadList;
    static ScriptGroup *s_emptyGroup;
};

#ifndef THYME_STANDALONE
#include "hooker.h"

inline void ScriptList::Hook_Me()
{
    Hook_Method(0x0051B920, &Hook_Xfer_Snapshot);
    Hook_Method(0x0051BC70, &Duplicate);
    Hook_Method(0x0051BD80, &Duplicate_And_Qualify);
    Hook_Function(0x0051C040, Get_Read_Scripts); // Must hooke with Parse_Scripts_Chunk
    Hook_Function(0x0051C080, Parse_Script_List_Chunk);
    Hook_Function(0x0051BF00, Parse_Scripts_Chunk); // Must hook with Get_Read_Scripts
}
#endif