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

#ifndef SIDESLIST_H
#define SIDESLIST_H

#include "always.h"
#include "dict.h"
#include "gametype.h"
#include "mempoolobj.h"
#include "scriptlist.h"
#include "snapshot.h"
#include "subsysteminterface.h"
#include "teamsinfo.h"

class RenderObjClass;
class Shadow;

class BuildListInfo : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_POOL(BuildListInfo);

    enum
    {
        UNLIMITED_REBUILDS = -1,
    };

public:
    BuildListInfo();
    virtual ~BuildListInfo();

    // Snapshot interface methods.
    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    BuildListInfo &operator=(const BuildListInfo &that);
    
    BuildListInfo *Get_Next() { return m_nextBuildList; }
    void Set_Next(BuildListInfo *next) { m_nextBuildList = next; }
    void Set_Building_Name(AsciiString name) { m_buildingName = name; }
    void Set_Template_Name(AsciiString name) { m_templateName = name; }
    void Set_Location(Coord3D &location) { m_location = location; }
    void Set_Angle(float angle) { m_angle = angle; }
    void Set_Intially_Built(bool built) { m_isInitiallyBuilt = built; }
    void Parse_Data_Chunk(DataChunkInput &input, DataChunkInfo *info);

private:
    AsciiString m_buildingName;
    AsciiString m_templateName;
    Coord3D m_location;
    Coord2D m_rallyPointOffset;
    float m_angle;
    bool m_isInitiallyBuilt;
    unsigned m_numRebuilds;
    BuildListInfo *m_nextBuildList;
    AsciiString m_script;
    int m_health;
    bool m_whiner;
    bool m_repairable;
    bool m_sellable;
    bool m_autoBuild;
    RenderObjClass *m_renderObj;
    Shadow *m_shadowObj;
    bool m_selected;
    ObjectID m_objectID;
    unsigned m_objectTimestamp;
    bool m_underConstruction;
    int m_unkArray[10];
    bool m_unkbool3;
    int m_unkint1;
    int m_unkint2;
    bool m_unkbool4;
};

class SidesInfo
{
public:
    SidesInfo() : m_buildList(nullptr), m_dict(0), m_scripts(nullptr) {}
    SidesInfo(const SidesInfo &that);
    ~SidesInfo();

    void Init(const Dict *dict = nullptr);
    void Add_To_BuildList(BuildListInfo *list, int pos);
    int Remove_From_BuildList(BuildListInfo *list);
    void Reorder_In_BuildList(BuildListInfo *list, int pos);
    void Set_ScriptList(ScriptList *list) { m_scripts = list; }
    ScriptList *Get_ScriptList() const { return m_scripts; }
    Dict &Get_Dict() { return m_dict; }

    SidesInfo &operator=(const SidesInfo &that);

private:
    BuildListInfo *m_buildList;
    Dict m_dict;
    ScriptList *m_scripts;
};

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
    bool Validate_Ally_Enemy_List(const AsciiString &team, AsciiString &allies);
    void Add_Player_By_Template(AsciiString template_name);
    SidesInfo *Find_Side_Info(AsciiString name, int *index = nullptr);
    SidesInfo *Find_Skirmish_Side_Info(AsciiString name, int *index = nullptr);
    SidesInfo *Get_Sides_Info(int index);
    TeamsInfo *Find_Team_Info(AsciiString name, int *index) { return m_teamRec.Find_Team(name, index); }

    static bool Parse_Sides_Chunk(DataChunkInput &input, DataChunkInfo *info, void *data);

private:
    int m_numSides;
    SidesInfo m_sides[MAX_SIDE_COUNT];
    int m_numSkirmishSides;
    SidesInfo m_skirmishSides[MAX_SIDE_COUNT];
    TeamsInfoRec m_teamRec;
    TeamsInfoRec m_skirmishTeamsRec;
};

#ifndef THYME_STNADALONE
#include "hooker.h"

extern SidesList *&g_theSidesList;
#else
extern SidesList *g_theSidesList;
#endif

#endif // SIDESLIST_H
