/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief AI
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
#include "aigroup.h"
#include "aiupdate.h"
#include "gametype.h"
#include "mempoolobj.h"
#include "object.h"
#include "snapshot.h"
#include "subsysteminterface.h"
#include <list>

class AttackPriorityInfo;
class BuildListInfo;
class INI;
class Object;
class PartitionFilter;
class Pathfinder;

class SkillSet
{
private:
    int m_scienceCount;
    ScienceType m_sciences[20];
    friend class AI;
};

class AISideInfo : public MemoryPoolObject
{
    IMPLEMENT_POOL(AISideInfo)

public:
    AISideInfo();
    virtual ~AISideInfo() override {}

private:
    Utf8String m_name;
    int m_resourceGatherersEasy;
    int m_resourceGatherersNormal;
    int m_resourceGatherersHard;
    SkillSet m_skillSets[5];
    Utf8String m_baseDefenseStructure1;
    AISideInfo *m_next;
    friend class TAiData;
    friend class AI;
};

class AISideBuildList : public MemoryPoolObject
{
    IMPLEMENT_POOL(AISideBuildList)

protected:
    virtual ~AISideBuildList() override;

public:
    AISideBuildList(Utf8String name);
    void Add_Info(BuildListInfo *info);

private:
    Utf8String m_name;
    BuildListInfo *m_buildListInfo;
    AISideBuildList *m_next;
    friend class TAiData;
    friend class AI;
};

class TAiData : public SnapShot
{
public:
    TAiData();
    ~TAiData();

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    void Add_Faction_Build_List(AISideBuildList *new_list);
    void Add_Side_Info(AISideInfo *info);

    float m_structureSeconds;
    float m_teamSeconds;
    int m_resourcesWealthy;
    int m_resourcesPoor;
    unsigned int m_forceIdleMSEC;
    float m_structuresWealthyMod;
    float m_teamWealthyMod;
    float m_structuresPoorMod;
    float m_teamPoorMod;
    float m_teamResourcesToBuild;
    float m_guardInnerModifierAI;
    float m_guardOuterModifierAI;
    float m_guardInnerModifierHuman;
    float m_guardOuterModifierHuman;
    unsigned int m_guardChaseUnitsDuration;
    unsigned int m_guardEnemyScanRate;
    unsigned int m_guardEnemyReturnScanRate;
    float m_wallHeight;
    float m_alertRangeModifier;
    float m_aggressiveRangeModifier;
    float m_attackPriorityDistanceModifier;
    float m_skirmishGroupFudgeDistance;
    float m_maxRecruitDistance;
    float m_skirmishBaseDefenseExtraDistance;
    float m_repulsedDistance;
    bool m_enableRepulsors;
    bool m_forceSkirmishAI;
    bool m_rotateSkirmishBases;
    bool m_attackUsesLineOfSight;
    bool m_attackIgnoreInsignificantBuildings;
    int m_minInfantryForGroup;
    int m_minVehiclesForGroup;
    float m_minDistanceForGroup;
    float m_distanceRequiresGroup;
    float m_minClumpDensity;
    int m_infantryPathfindDiameter;
    int m_vehiclePathfindDiameter;
    int m_rebuildDelayTimeSeconds;
    float m_supplyCenterSafeRadius;
    float m_aiDozerBoredRadiusModifier;
    bool m_aiCrushesInfantry;
    float m_maxRetaliationDistance;
    float m_retaliationFriendsRadius;
    AISideInfo *m_sideInfo;
    AISideBuildList *m_AISideBuildList;
    TAiData *m_next;
};

class AI : public SubsystemInterface, public SnapShot
{
public:
#ifdef GAME_DLL
    AI *Hook_Ctor() { return new (this) AI(); }
#endif

    AI();
    virtual ~AI() override;

    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override;

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    static void Parse_Side_Info(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Skill_Set(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Science(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Skirmish_Build_List(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_AI_Data_Definition(INI *ini);
    static float Get_Adjusted_Vision_Range_For_Object(Object *obj, int factors_to_consider);

    void New_Override();

    AIGroup *Create_Group();
    AIGroup *Find_Group(int id);
    void Destroy_Group(AIGroup *group);
    int Get_Next_Formation_ID();
    Object *Find_Closest_Enemy(const Object *me,
        float range,
        unsigned int qualifiers,
        const AttackPriorityInfo *info,
        PartitionFilter *optional_filter);
    Object *Find_Closest_Ally(const Object *me, float range, unsigned int qualifiers);
    Object *Find_Closest_Repulsor(const Object *me, float range);
    void Add_Side_Info(AISideInfo *info);

    Pathfinder *Get_Pathfinder() { return m_pathfinder; }
    const TAiData *Get_AI_Data() { return m_aiData; }
    int Get_Next_Group_ID() { return ++m_groupID; }

private:
    Pathfinder *m_pathfinder;
    std::list<AIGroup *> m_groupList;
    TAiData *m_aiData;
    int m_groupID;
    int m_formationID;
};

#ifdef GAME_DLL
extern AI *&g_theAI;
#else
extern AI *g_theAI;
#endif
