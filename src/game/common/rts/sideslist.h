/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Classes for managing sides.
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
#include "sidesinfo.h"
#include "snapshot.h"
#include "subsysteminterface.h"
#include "teamsinfo.h"

class SidesList : public SubsystemInterface, public SnapShot
{
    enum
    {
        MAX_TEAM_DEPTH = 3,
        MAX_SIDE_COUNT = 16,
        MAX_LIST_COUNT = 24,
    };
public:
    SidesList() : m_numSides(0), m_numSkirmishSides(0), m_teamRec(), m_skirmishTeamsRec() {}
    virtual ~SidesList() {}

    // Subsystem interface methods.
    virtual void Init() override {};
    virtual void Reset() override;
    virtual void Update() override {}

    // Snapshot interface methods.
    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    void Clear();
    void Add_Side(const Dict *dict);
    void Add_Team(const Dict *dict) { m_teamRec.Add_Team(dict); }
    void Remove_Team(int index) { m_teamRec.Remove_Team(index); }
    void Empty_Teams();
    void Empty_Sides();
    bool Validate_Sides();
    bool Validate_Ally_Enemy_List(const Utf8String &team, Utf8String &allies);
    void Add_Player_By_Template(Utf8String template_name);
    SidesInfo *Find_Side_Info(Utf8String name, int *index = nullptr);
    SidesInfo *Find_Skirmish_Side_Info(Utf8String name, int *index = nullptr);
    SidesInfo *Get_Sides_Info(int index);
    TeamsInfo *Find_Team_Info(Utf8String name, int *index) { return m_teamRec.Find_Team(name, index); }

    static bool Parse_Sides_Chunk(DataChunkInput &input, DataChunkInfo *info, void *data);

#ifndef THYME_STANDALONE
    static void Hook_Me();
#endif

private:
    int m_numSides;
    SidesInfo m_sides[MAX_SIDE_COUNT];
    int m_numSkirmishSides;
    SidesInfo m_skirmishSides[MAX_SIDE_COUNT];
    TeamsInfoRec m_teamRec;
    TeamsInfoRec m_skirmishTeamsRec;
};

#ifndef THYME_STANDALONE
#include "hooker.h"

extern SidesList *&g_theSidesList;

inline void SidesList::Hook_Me()
{
    Hook_Method(0x004D77A0, &Clear);
    Hook_Method(0x004D7F70, &Validate_Sides);
    Hook_Method(0x004D79A0, &Validate_Ally_Enemy_List);
    Hook_Method(0x004D7BC0, &Add_Player_By_Template);
    Hook_Method(0x004D6A60, &Find_Side_Info);
    Hook_Method(0x004D6BD0, &Find_Skirmish_Side_Info);
    Hook_Function(0x004D62A0, &Parse_Sides_Chunk);
}
#else
extern SidesList *g_theSidesList;
#endif