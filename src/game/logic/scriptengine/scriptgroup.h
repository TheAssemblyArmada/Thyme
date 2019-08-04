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

#include "always.h"
#include "asciistring.h"
#include "datachunk.h"
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

    ScriptGroup *Duplicate();
    ScriptGroup *Duplicate_And_Qualify(const Utf8String &str1, const Utf8String &str2, const Utf8String &str3);
    void Add_Script(Script *script, int index);
    ScriptGroup *Get_Next() { return m_nextGroup; }
    Script *Get_Scripts() { return m_firstScript; }
    void Set_Next(ScriptGroup *next) { m_nextGroup = next; }

    static bool Parse_Group_Chunk(DataChunkInput &input, DataChunkInfo *info, void *data);

#ifdef GAME_DLL
    void Hook_Xfer_Snapshot(Xfer *xfer);
    static void Hook_Me();
#endif

private:
    Script *m_firstScript;
    Utf8String m_groupName;
    bool m_isGroupActive;
    bool m_isGroupSubroutine;
    ScriptGroup *m_nextGroup;
    bool m_hasWarnings;

    static int s_curID;
};

#ifdef GAME_DLL
#include "hooker.h"

inline void ScriptGroup::Hook_Me()
{
    Hook_Method(0x0051C3B0, &Hook_Xfer_Snapshot);
    Hook_Method(0x0051C510, &Duplicate);
    Hook_Method(0x0051C670, &Duplicate_And_Qualify);
    Hook_Function(0x0051C860, Parse_Group_Chunk);
}
#endif