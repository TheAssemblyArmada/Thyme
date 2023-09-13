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
    virtual void Init() override{};
    virtual void Reset() override;
    virtual void Update() override {}

    // Snapshot interface methods.
    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    void Clear();
    void Empty_Teams();
    void Empty_Sides();
    bool Validate_Sides();
    bool Validate_Ally_Enemy_List(const Utf8String &team, Utf8String &allies);
    void Add_Player_By_Template(Utf8String template_name);
    SidesInfo *Find_Side_Info(Utf8String name, int *index = nullptr);
    SidesInfo *Find_Skirmish_Side_Info(Utf8String name, int *index = nullptr);
    SidesInfo *Get_Side_Info(int index);
    SidesInfo *Get_Skirmish_Side_Info(int index);
    void Add_Side(const Dict *dict);
    void Remove_Side(int index);
    void Prepare_For_MP_Or_Skirmish();
    bool Is_Player_Default_Team(TeamsInfo *t);

    void Add_Team(const Dict *dict) { m_teamRec.Add_Team(dict); }
    void Add_Skirmish_Team(const Dict *dict) { m_skirmishTeamsRec.Add_Team(dict); }
    void Remove_Team(int index) { m_teamRec.Remove_Team(index); }
    int Get_Num_Sides() const { return m_numSides; }
    int Get_Num_Skirmish_Sides() const { return m_numSkirmishSides; }
    int Get_Num_Teams() { return m_teamRec.Get_Num_Teams(); }
    int Get_Num_Skirmish_Teams() { return m_skirmishTeamsRec.Get_Num_Teams(); }
    TeamsInfo *Get_Team_Info(int index) { return m_teamRec.Get_Team_Info(index); }
    TeamsInfo *Get_Skirmish_Team_Info(int index) { return m_skirmishTeamsRec.Get_Team_Info(index); }
    TeamsInfo *Find_Team_Info(Utf8String name, int *index) { return m_teamRec.Find_Team_Info(name, index); }
    TeamsInfo *Find_Skirmish_Team_Info(Utf8String name, int *index)
    {
        return m_skirmishTeamsRec.Find_Team_Info(name, index);
    }

    static bool Parse_Sides_Data_Chunk(DataChunkInput &input, DataChunkInfo *info, void *data);
    static void Write_Sides_Data_Chunk(DataChunkOutput &output);

private:
    int m_numSides;
    SidesInfo m_sides[MAX_SIDE_COUNT];
    int m_numSkirmishSides;
    SidesInfo m_skirmishSides[MAX_SIDE_COUNT];
    TeamsInfoRec m_teamRec;
    TeamsInfoRec m_skirmishTeamsRec;
};

#ifdef GAME_DLL
#include "hooker.h"

extern SidesList *&g_theSidesList;
#else
extern SidesList *g_theSidesList;
#endif
