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
#include "ai.h"
#include "aipathfind.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif
#include "xfer.h"
#include <algorithm>

AISideInfo::AISideInfo() :
    m_resourceGatherersEasy(0), m_resourceGatherersNormal(1), m_resourceGatherersHard(2), m_next(nullptr)
{
    m_unkString.Clear();
    m_baseDefenseStructure1.Clear();
}

TAiData::TAiData() :
    m_structureSeconds(0.0f),
    m_teamSeconds(0.0f),
    m_resourcesWealthy(0),
    m_resourcesPoor(0),
    m_forceIdleMSEC(1),
    m_structuresWealthyMod(0.0f),
    m_teamWealthyMod(0.0f),
    m_structuresPoorMod(0.0f),
    m_teamPoorMod(0.0f),
    m_teamResourcesToBuild(0.0f),
    m_guardInnerModifierAI(0.0f),
    m_guardOuterModifierAI(0.0f),
    m_guardInnerModifierHuman(0.0f),
    m_guardOuterModifierHuman(0.0f),
    m_guardChaseUnitsDuration(0),
    m_guardEnemyScanRate(15),
    m_guardEnemyReturnScanRate(30),
    m_wallHeight(0.0f),
    m_alertRangeModifier(0.0f),
    m_aggressiveRangeModifier(0.0f),
    m_attackPriorityDistanceModifier(0.0f),
    m_skirmishGroupFudgeDistance(0.0f),
    m_maxRecruitDistance(0.0f),
    m_skirmishBaseDefenseExtraDistance(0.0f),
    m_repulsedDistance(0.0f),
    m_enableRepulsors(false),
    m_forceSkirmishAI(false),
    m_rotateSkirmishBases(false),
    m_attackUsesLineOfSight(true),
    m_attackIgnoreInsignificantBuildings(false),
    m_minInfantryForGroup(3),
    m_minVehiclesForGroup(4),
    m_minDistanceForGroup(100.0f),
    m_distanceRequiresGroup(0.0f),
    m_minClumpDensity(0.5f),
    m_infantryPathfindDiameter(6),
    m_vehiclePathfindDiameter(6),
    m_rebuildDelayTimeSeconds(10),
    m_supplyCenterSafeRadius(250.0f),
    m_aiDozerBoredRadiusModifier(2.0f),
    m_aiCrushesInfantry(true),
    m_maxRetaliationDistance(210.0f),
    m_retaliationFriendsRadius(120.0f),
    m_sideInfo(nullptr),
    m_AISideBuildList(nullptr),
    m_next(nullptr)
{
}

TAiData::~TAiData()
{
    AISideInfo *info = m_sideInfo;
    m_sideInfo = nullptr;

    while (info != nullptr) {
        AISideInfo *info2 = info;
        info = info->m_next;
        info2->Delete_Instance();
    }

    AISideBuildList *list = m_AISideBuildList;
    m_AISideBuildList = nullptr;

    while (list != nullptr) {
        AISideBuildList *list2 = list;
        list = list->m_next;
        list2->Delete_Instance();
    }
}

void TAiData::CRC_Snapshot(Xfer *xfer)
{
#ifdef GAME_DLL
    Call_Method<void, SnapShot, Xfer *>(PICK_ADDRESS(0x004AB860, 0x0086B099), this, xfer);
#endif
}

void TAiData::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
}

AI::AI() : m_groupID(0), m_formationID(0)
{
    m_aiData = new TAiData();
    m_pathfinder = new Pathfinder();
}

AI::~AI()
{
    if (m_pathfinder != nullptr) {
        delete m_pathfinder;
    }

    m_pathfinder = nullptr;

    while (m_aiData != nullptr) {
        TAiData *data = m_aiData;
        m_aiData = data->m_next;
        delete data;
    }
}

void AI::Init()
{
    m_groupID = 0;
}

void AI::Reset()
{
#ifdef GAME_DLL
    Call_Method<void, AI>(PICK_ADDRESS(0x004AA960, 0x00869EF9), this);
#endif
}

void AI::Update()
{
#ifdef GAME_DLL
    Call_Method<void, AI>(PICK_ADDRESS(0x004AAA70, 0x00869FCA), this);
#endif
}

void AI::CRC_Snapshot(Xfer *xfer)
{
#ifdef GAME_DLL
    Call_Method<void, SnapShot, Xfer *>(PICK_ADDRESS(0x004AB980, 0x0086B2B7), this, xfer);
#endif
}

void AI::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
}

#ifndef GAME_DLL
AI *g_theAI = nullptr;
#endif

AIGroup *AI::Create_Group()
{
    AIGroup *group = new AIGroup();
    m_groupList.push_back(group);
    return group;
}

void AI::Destroy_Group(AIGroup *group)
{
    auto it = std::find(m_groupList.begin(), m_groupList.end(), group);

    if (!(it == m_groupList.end())) {
        captainslog_dbgassert(group != nullptr, "A NULL group made its way into the AIGroup list..");
        m_groupList.erase(it);
    }
}

AIGroup *AI::Find_Group(int id)
{
    for (auto it = m_groupList.begin(); it != m_groupList.end(); it++) {
        if ((*it)->Get_ID() == id) {
            return *it;
        }
    }

    return nullptr;
}

int AI::Get_Next_Formation_ID()
{
    return ++m_formationID;
}

float AI::Get_Adjusted_Vision_Range_For_Object(Object *obj, int factors_to_consider)
{
#ifdef GAME_DLL
    return Call_Function<float, Object *, int>(PICK_ADDRESS(0x004AB690, 0x0086AB43), obj, factors_to_consider);
#else
    return 0.0f;
#endif
}
