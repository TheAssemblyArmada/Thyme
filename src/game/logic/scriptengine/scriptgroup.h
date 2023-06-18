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

protected:
    virtual ~ScriptGroup() override;

public:
    ScriptGroup();

    // Snapshot interface methods.
    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    ScriptGroup *Duplicate();
    ScriptGroup *Duplicate_And_Qualify(const Utf8String &str1, const Utf8String &str2, const Utf8String &str3);
    void Add_Script(Script *script, int index);
    void Delete_Script(Script *script);

    ScriptGroup *Get_Next() const { return m_nextGroup; }
    Script *Get_Script() const { return m_firstScript; }
    Utf8String Get_Name() const { return m_groupName; }

    bool Is_Active() const { return m_isGroupActive; }
    bool Is_Subroutine() const { return m_isGroupSubroutine; }
    bool Has_Warnings() const { return m_hasWarnings; }

    void Set_Next_Group(ScriptGroup *next) { m_nextGroup = next; }
    void Set_Name(Utf8String name) { m_groupName = name; }
    void Set_Active(bool set) { m_isGroupActive = set; }
    void Set_Subroutine(bool set) { m_isGroupSubroutine = set; }
    void Set_Warnings(bool set) { m_hasWarnings = set; }

    static bool Parse_Group_Data_Chunk(DataChunkInput &input, DataChunkInfo *info, void *data);
    static void Write_Group_Data_Chunk(DataChunkOutput &output, ScriptGroup *group);

private:
    Script *m_firstScript;
    Utf8String m_groupName;
    bool m_isGroupActive;
    bool m_isGroupSubroutine;
    ScriptGroup *m_nextGroup;
    bool m_hasWarnings;
};
