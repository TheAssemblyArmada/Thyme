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
#include "buildinfo.h"
#include "playerlist.h"
#include "xfer.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif
#include <algorithm>

// clang-format off
static const FieldParse s_AIFieldParseTable[] = {
    {"StructureSeconds", &INI::Parse_Real, nullptr, offsetof(TAiData, m_structureSeconds)}, 
    {"TeamSeconds", &INI::Parse_Real, nullptr, offsetof(TAiData, m_teamSeconds)}, 
    {"Wealthy", &INI::Parse_Int, nullptr, offsetof(TAiData, m_resourcesWealthy)}, 
    {"Poor", &INI::Parse_Int, nullptr, offsetof(TAiData, m_resourcesPoor)}, 
    {"ForceIdleMSEC", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(TAiData, m_forceIdleMSEC)}, 
    {"StructuresWealthyRate", &INI::Parse_Real, nullptr, offsetof(TAiData, m_structuresWealthyMod)}, 
    {"TeamsWealthyRate", &INI::Parse_Real, nullptr, offsetof(TAiData, m_teamWealthyMod)}, 
    {"StructuresPoorRate", &INI::Parse_Real, nullptr, offsetof(TAiData, m_structuresPoorMod)}, 
    {"TeamsPoorRate", &INI::Parse_Real, nullptr, offsetof(TAiData, m_teamPoorMod)}, 
    {"TeamResourcesToStart", &INI::Parse_Real, nullptr, offsetof(TAiData, m_teamResourcesToBuild)}, 
    {"GuardInnerModifierAI", &INI::Parse_Real, nullptr, offsetof(TAiData, m_guardInnerModifierAI)}, 
    {"GuardOuterModifierAI", &INI::Parse_Real, nullptr, offsetof(TAiData, m_guardOuterModifierAI)}, 
    {"GuardInnerModifierHuman", &INI::Parse_Real, nullptr, offsetof(TAiData, m_guardInnerModifierHuman)}, 
    {"GuardOuterModifierHuman", &INI::Parse_Real, nullptr, offsetof(TAiData, m_guardOuterModifierHuman)}, 
    {"GuardChaseUnitsDuration", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(TAiData, m_guardChaseUnitsDuration)}, 
    {"GuardEnemyScanRate", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(TAiData, m_guardEnemyScanRate)}, 
    {"GuardEnemyReturnScanRate", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(TAiData, m_guardEnemyReturnScanRate)}, 
    {"SkirmishGroupFudgeDistance", &INI::Parse_Real, nullptr, offsetof(TAiData, m_skirmishGroupFudgeDistance)}, 
    {"RepulsedDistance", &INI::Parse_Real, nullptr, offsetof(TAiData, m_repulsedDistance)}, 
    {"EnableRepulsors", &INI::Parse_Bool, nullptr, offsetof(TAiData, m_enableRepulsors)}, 
    {"AlertRangeModifier", &INI::Parse_Real, nullptr, offsetof(TAiData, m_alertRangeModifier)}, 
    {"AggressiveRangeModifier", &INI::Parse_Real, nullptr, offsetof(TAiData, m_aggressiveRangeModifier)}, 
    {"ForceSkirmishAI", &INI::Parse_Bool, nullptr, offsetof(TAiData, m_forceSkirmishAI)}, 
    {"RotateSkirmishBases", &INI::Parse_Bool, nullptr, offsetof(TAiData, m_rotateSkirmishBases)}, 
    {"AttackUsesLineOfSight", &INI::Parse_Bool, nullptr, offsetof(TAiData, m_attackUsesLineOfSight)}, 
    {"AttackIgnoreInsignificantBuildings", &INI::Parse_Bool, nullptr, offsetof(TAiData, m_attackIgnoreInsignificantBuildings)}, 
    {"AttackPriorityDistanceModifier", &INI::Parse_Real, nullptr, offsetof(TAiData, m_attackPriorityDistanceModifier)}, 
    {"MaxRecruitRadius", &INI::Parse_Real, nullptr, offsetof(TAiData, m_maxRecruitDistance)}, 
    {"SkirmishBaseDefenseExtraDistance", &INI::Parse_Real, nullptr, offsetof(TAiData, m_skirmishBaseDefenseExtraDistance)}, 
    {"WallHeight", &INI::Parse_Real, nullptr, offsetof(TAiData, m_wallHeight)}, 
    {"SideInfo", &AI::Parse_Side_Info, nullptr, 0}, 
    {"SkirmishBuildList", &AI::Parse_Skirmish_Build_List, nullptr, 0}, 
    {"MinInfantryForGroup", &INI::Parse_Int, nullptr, offsetof(TAiData, m_minInfantryForGroup)}, 
    {"MinVehiclesForGroup", &INI::Parse_Int, nullptr, offsetof(TAiData, m_minVehiclesForGroup)}, 
    {"MinDistanceForGroup", &INI::Parse_Real, nullptr, offsetof(TAiData, m_minDistanceForGroup)}, 
    {"DistanceRequiresGroup", &INI::Parse_Real, nullptr, offsetof(TAiData, m_distanceRequiresGroup)}, 
    {"MinClumpDensity", &INI::Parse_Real, nullptr, offsetof(TAiData, m_minClumpDensity)}, 
    {"InfantryPathfindDiameter", &INI::Parse_Int, nullptr, offsetof(TAiData, m_infantryPathfindDiameter)}, 
    {"VehiclePathfindDiameter", &INI::Parse_Int, nullptr, offsetof(TAiData, m_vehiclePathfindDiameter)}, 
    {"RebuildDelayTimeSeconds", &INI::Parse_Int, nullptr, offsetof(TAiData, m_rebuildDelayTimeSeconds)}, 
    {"SupplyCenterSafeRadius", &INI::Parse_Real, nullptr, offsetof(TAiData, m_supplyCenterSafeRadius)}, 
    {"AIDozerBoredRadiusModifier", &INI::Parse_Real, nullptr, offsetof(TAiData, m_aiDozerBoredRadiusModifier)}, 
    {"AICrushesInfantry", &INI::Parse_Bool, nullptr, offsetof(TAiData, m_aiCrushesInfantry)}, 
    {"MaxRetaliationDistance", &INI::Parse_Real, nullptr, offsetof(TAiData, m_maxRetaliationDistance)}, 
    {"RetaliationFriendsRadius", &INI::Parse_Real, nullptr, offsetof(TAiData, m_retaliationFriendsRadius)}, 
    {nullptr, nullptr, nullptr, 0}
};
// clang-format on

AISideInfo::AISideInfo() :
    m_resourceGatherersEasy(0), m_resourceGatherersNormal(1), m_resourceGatherersHard(2), m_next(nullptr)
{
    m_name.Clear();
    m_baseDefenseStructure1.Clear();
}

AISideBuildList::AISideBuildList(Utf8String name) : m_name(name), m_buildListInfo(nullptr), m_next(nullptr) {}

AISideBuildList::~AISideBuildList()
{
    if (m_buildListInfo != nullptr) {
        m_buildListInfo->Delete_Instance();
        m_buildListInfo = nullptr;
    }
}

void AISideBuildList::Add_Info(BuildListInfo *info)
{
    if (m_buildListInfo != nullptr) {
        BuildListInfo *old_info;

        for (old_info = m_buildListInfo; old_info != nullptr && old_info->Get_Next() != nullptr;
             old_info = old_info->Get_Next()) {
        }

        captainslog_dbgassert(old_info != nullptr && old_info->Get_Next() == nullptr, "Logic error.");
        old_info->Set_Next_Build_List(info);
    } else {
        m_buildListInfo = info;
    }

    info->Set_Next_Build_List(nullptr);
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

void TAiData::Add_Side_Info(AISideInfo *info)
{
    info->m_next = m_sideInfo;
    m_sideInfo = info;
}

void TAiData::Add_Faction_Build_List(AISideBuildList *new_list)
{
    AISideBuildList *cur_list;

    for (cur_list = m_AISideBuildList;; cur_list = cur_list->m_next) {
        if (cur_list == nullptr) {
            new_list->m_next = m_AISideBuildList;
            m_AISideBuildList = new_list;
            return;
        }

        if (new_list->m_name == cur_list->m_name) {
            break;
        }
    }

    if (cur_list->m_buildListInfo != nullptr) {
        cur_list->m_buildListInfo->Delete_Instance();
    }

    cur_list->m_buildListInfo = new_list->m_buildListInfo;
    new_list->m_buildListInfo = nullptr;
    new_list->m_next = nullptr;
    new_list->Delete_Instance();
}

void TAiData::CRC_Snapshot(Xfer *xfer)
{
    xfer->xferReal(&m_structureSeconds);
    xfer->xferReal(&m_teamSeconds);
    xfer->xferInt(&m_resourcesWealthy);
    xfer->xferInt(&m_resourcesPoor);
    xfer->xferUnsignedInt(&m_forceIdleMSEC);
    xfer->xferReal(&m_structuresWealthyMod);
    xfer->xferReal(&m_teamWealthyMod);
    xfer->xferReal(&m_structuresPoorMod);
    xfer->xferReal(&m_teamPoorMod);
    xfer->xferReal(&m_teamResourcesToBuild);
    xfer->xferReal(&m_guardInnerModifierAI);
    xfer->xferReal(&m_guardOuterModifierAI);
    xfer->xferReal(&m_guardInnerModifierHuman);
    xfer->xferReal(&m_guardOuterModifierHuman);
    xfer->xferUnsignedInt(&m_guardChaseUnitsDuration);
    xfer->xferUnsignedInt(&m_guardEnemyScanRate);
    xfer->xferUnsignedInt(&m_guardEnemyReturnScanRate);
    xfer->xferReal(&m_alertRangeModifier);
    xfer->xferReal(&m_aggressiveRangeModifier);
    xfer->xferReal(&m_attackPriorityDistanceModifier);
    xfer->xferReal(&m_maxRecruitDistance);
    xfer->xferReal(&m_skirmishBaseDefenseExtraDistance);
    xfer->xferReal(&m_repulsedDistance);
    xfer->xferBool(&m_enableRepulsors);
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
    m_pathfinder->Reset();

    while (m_aiData != nullptr && m_aiData->m_next != nullptr) {
        TAiData *data = m_aiData;
        m_aiData = data->m_next;
        delete data;
    }

    while (m_groupList.size() != 0) {
        AIGroup *group = m_groupList.front();

        if (group != nullptr) {
            Destroy_Group(group);
        } else {
            m_groupList.pop_front();
        }
    }

    m_groupID = 0;
    m_formationID = 0;
    Get_Next_Formation_ID();
}

void AI::Update()
{
    m_pathfinder->Process_Pathfind_Queue();
    g_thePlayerList->Update();
}

void AI::CRC_Snapshot(Xfer *xfer)
{
    xfer->xferSnapshot(m_pathfinder);
    Utf8String marker;

    for (TAiData *data = m_aiData; data != nullptr; data = data->m_next) {
        marker = "MARKER:TAiData";
        xfer->xferAsciiString(&marker);
        xfer->xferSnapshot(data);
    }

    for (auto it = m_groupList.begin(); it != m_groupList.end(); it++) {
        if (*it != nullptr) {
            marker = "MARKER:AIGroup";
            xfer->xferAsciiString(&marker);
            xfer->xferSnapshot(*it);
        }
    }
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
        group->Delete_Instance();
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

void AI::Parse_Side_Info(INI *ini, void *formal, void *store, const void *user_data)
{
    static FieldParse myFieldParse[] = {
        { "ResourceGatherersEasy", &INI::Parse_Int, nullptr, offsetof(AISideInfo, m_resourceGatherersEasy) },
        { "ResourceGatherersNormal", &INI::Parse_Int, nullptr, offsetof(AISideInfo, m_resourceGatherersNormal) },
        { "ResourceGatherersHard", &INI::Parse_Int, nullptr, offsetof(AISideInfo, m_resourceGatherersHard) },
        { "BaseDefenseStructure1", &INI::Parse_AsciiString, nullptr, offsetof(AISideInfo, m_baseDefenseStructure1) },
        { "SkillSet1", &AI::Parse_Skill_Set, nullptr, offsetof(AISideInfo, m_skillSets[0]) },
        { "SkillSet2", &AI::Parse_Skill_Set, nullptr, offsetof(AISideInfo, m_skillSets[1]) },
        { "SkillSet3", &AI::Parse_Skill_Set, nullptr, offsetof(AISideInfo, m_skillSets[2]) },
        { "SkillSet4", &AI::Parse_Skill_Set, nullptr, offsetof(AISideInfo, m_skillSets[3]) },
        { "SkillSet5", &AI::Parse_Skill_Set, nullptr, offsetof(AISideInfo, m_skillSets[4]) },
        { nullptr, nullptr, nullptr, 0 }
    };

    Utf8String name(ini->Get_Next_Token());
    TAiData *data = static_cast<TAiData *>(formal);
    AISideInfo *info;

    for (info = data->m_sideInfo; info != nullptr && !(name == info->m_name); info = info->m_next) {
    }

    if (info == nullptr) {
        info = new AISideInfo();
        data->Add_Side_Info(info);
    }

    info->m_name = name;
    ini->Init_From_INI(info, myFieldParse);
}

void AI::Parse_Skill_Set(INI *ini, void *formal, void *store, const void *user_data)
{
    static FieldParse myFieldParse[] = { { "Science", &AI::Parse_Science, nullptr, 0 }, { nullptr, nullptr, nullptr, 0 } };

    SkillSet *skill = static_cast<SkillSet *>(store);
    skill->m_scienceCount = 0;
    ini->Init_From_INI(skill, myFieldParse);
}

void AI::Parse_Science(INI *ini, void *formal, void *store, const void *user_data)
{
    SkillSet *skill = static_cast<SkillSet *>(formal);

    if (skill->m_scienceCount < 20) {
        skill->m_sciences[skill->m_scienceCount] = SCIENCE_INVALID;
        ScienceStore::Parse_Science(ini, skill, &skill->m_sciences[skill->m_scienceCount], nullptr);
        ScienceType science = skill->m_sciences[skill->m_scienceCount];

        if (science != SCIENCE_INVALID) {
            if (g_theScienceStore->Get_Science_Purchase_Cost(science) != 0) {
                skill->m_scienceCount++;
            } else {
                captainslog_dbgassert(false,
                    "Science %s is not purchaseable, can't be bought.",
                    g_theScienceStore->Get_Internal_Name_From_Science(science).Str());
            }
        }
    } else {
        captainslog_dbgassert("Too many SCIENCE skills in skillset. Skill = %s, max is %d", ini->Get_Next_Token(), 20);
    }
}

void AI::Parse_Skirmish_Build_List(INI *ini, void *formal, void *store, const void *user_data)
{
    static FieldParse myFieldParse[] = { { "Structure", &BuildListInfo::Parse_Structure, nullptr, 0 },
        { nullptr, nullptr, nullptr, 0 } };

    TAiData *data = static_cast<TAiData *>(formal);
    Utf8String name(ini->Get_Next_Token());
    AISideBuildList *build = new AISideBuildList(name);
    ini->Init_From_INI(build, myFieldParse);
    data->Add_Faction_Build_List(build);
}

void AI::Parse_AI_Data_Definition(INI *ini)
{
    if (g_theAI != nullptr && ini->Get_Load_Type() == INI_LOAD_CREATE_OVERRIDES) {
        g_theAI->New_Override();
    }

    if (g_theAI != nullptr) {
        ini->Init_From_INI(g_theAI->m_aiData, s_AIFieldParseTable);
    }
}

void AI::New_Override()
{
    TAiData *ai_data = m_aiData;
    TAiData *new_ai_data = new TAiData();
    m_aiData = new_ai_data;
    *m_aiData = *ai_data;
    m_aiData->m_sideInfo = nullptr;

    for (AISideInfo *info = ai_data->m_sideInfo; info != nullptr; info = info->m_next) {
        AISideInfo *new_info = new AISideInfo();
        *new_info = *info;
        new_info->m_next = nullptr;
        Add_Side_Info(new_info);
    }

    m_aiData->m_AISideBuildList = nullptr;

    for (AISideBuildList *list = ai_data->m_AISideBuildList; list != nullptr; list = list->m_next) {
        AISideBuildList *new_list = new AISideBuildList(list->m_name);
        new_list->m_next = nullptr;
        new_list->m_buildListInfo = list->m_buildListInfo->Duplicate();
        m_aiData->Add_Faction_Build_List(new_list);
    }

    m_aiData->m_next = ai_data;
}

void AI::Add_Side_Info(AISideInfo *info)
{
    m_aiData->Add_Side_Info(info);
}
