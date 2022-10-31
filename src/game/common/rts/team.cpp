/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Team
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "team.h"
#include "gamelogic.h"
#include "playerlist.h"
#include "script.h"
#include "scriptengine.h"
#include "sideslist.h"
#include "staticnamekey.h"
#include "terrainlogic.h"
#include "xfer.h"

#ifndef GAME_DLL
TeamFactory *g_theTeamFactory = nullptr;
#endif

TeamRelationMap::~TeamRelationMap()
{
    m_relationships.clear();
}

void TeamRelationMap::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    unsigned short size = (unsigned short)m_relationships.size();
    xfer->xferUnsignedShort(&size);

    if (xfer->Get_Mode() == XFER_SAVE) {
        for (auto it = m_relationships.begin(); it != m_relationships.end(); ++it) {
            unsigned int team_id = (*it).first;
            xfer->xferUser(&team_id, sizeof(team_id));
            Relationship r = (*it).second;
            xfer->xferUser(&r, sizeof(r));
        }
    } else {
        for (int i = 0; i < size; i++) {
            unsigned int team_id;
            xfer->xferUser(&team_id, sizeof(team_id));
            Relationship r;
            xfer->xferUser(&r, sizeof(r));
            m_relationships[i] = r;
        }
    }
}

TeamFactory::TeamFactory() : m_nextPrototypeID(0), m_nextTeamID(0) {}

TeamFactory::~TeamFactory()
{
    Clear();
}

void TeamFactory::Init()
{
    Clear();
}

void TeamFactory::Reset()
{
    m_nextPrototypeID = 0;
    m_nextTeamID = 0;
    Clear();
}

void TeamFactory::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    xfer->xferUser(&m_nextTeamID, sizeof(m_nextTeamID));
    unsigned short size = (unsigned short)m_prototypes.size();
    xfer->xferUnsignedShort(&size);
    captainslog_relassert(size == m_prototypes.size(),
        6,
        "TeamFactory::xfer - Prototype count mismatch '%d should be '%d'",
        size,
        m_prototypes.size());
    std::map<NameKeyType, TeamPrototype *>::iterator it;
    Utf8String str;

    if (xfer->Get_Mode() == XFER_SAVE) {
        for (it = m_prototypes.begin(); it != m_prototypes.end(); ++it) {
            TeamPrototype *proto = it->second;
            unsigned int team_id = proto->Get_ID();
            xfer->xferUser(&team_id, sizeof(team_id));
            xfer->xferSnapshot(proto);
        }
    } else {
        for (int i = 0; i < size; i++) {
            unsigned int team_id;
            xfer->xferUser(&team_id, sizeof(team_id));
            TeamPrototype *proto = Find_Team_Prototype_By_ID(team_id);
            captainslog_relassert(proto != nullptr, 6, "TeamFactory::xfer - Unable to find team prototype by id");
            xfer->xferSnapshot(proto);
        }
    }
}

void TeamFactory::Load_Post_Process()
{
    m_nextTeamID = 0;
    m_nextPrototypeID = 0;
    std::map<NameKeyType, TeamPrototype *>::iterator it;

    for (it = m_prototypes.begin(); it != m_prototypes.end(); ++it) {
        TeamPrototype *proto = it->second;

        if (proto->Get_ID() >= m_nextPrototypeID) {
            m_nextPrototypeID = proto->Get_ID() + 1;
        }

        DLINK_ITERATOR<Team> iter = proto->Iterate_Team_Instance_List();

        while (!iter.Done()) {
            Team *team = iter.Cur();

            if (team->Get_Team_ID() >= m_nextTeamID) {
                m_nextTeamID = team->Get_Team_ID() + 1;
            }

            iter.Advance();
        }
    }
}

void TeamFactory::Clear()
{
    std::map<NameKeyType, TeamPrototype *> temp(m_prototypes);
    m_prototypes.clear();

    for (std::map<NameKeyType, TeamPrototype *>::iterator it = temp.begin(); it != temp.end(); ++it) {
        it->second->Delete_Instance();
    }
}

void TeamFactory::Init_From_Sides(SidesList *sides)
{
    Clear();

    for (int i = 0; i < sides->Get_Num_Teams(); i++) {
        Dict *d = sides->Get_Team_Info(i)->Get_Dict();
        Utf8String name = d->Get_AsciiString(g_teamNameKey);
        Utf8String owner = d->Get_AsciiString(g_teamOwnerKey);
        bool is_singleton = d->Get_Bool(g_teamIsSingletonKey);
        Init_Team(name, owner, is_singleton, d);
    }
}

void TeamFactory::Init_Team(const Utf8String &name, const Utf8String &owner, bool is_singleton, Dict *d)
{
    captainslog_dbgassert(!Find_Team_Prototype(name), "team already exists");
    Player *player = g_thePlayerList->Find_Player_With_NameKey(Name_To_Key(owner));
    captainslog_dbgassert(player != nullptr, "no owner found for team %s (%s)", owner.Str(), name.Str());

    if (player == nullptr) {
        player = g_thePlayerList->Get_Neutral_Player();
    }

    new TeamPrototype(this, name, player, is_singleton, d, ++m_nextPrototypeID);

    if (is_singleton) {
        Create_Inactive_Team(name);
    }
}

void TeamFactory::Add_Team_Prototype_To_List(TeamPrototype *team)
{
    const Utf8String &name = team->Get_Name();
    NameKeyType key = Name_To_Key(name);
    std::map<NameKeyType, TeamPrototype *>::iterator it = m_prototypes.find(key);

    if (it != m_prototypes.end()) {
        captainslog_dbgassert(
            it->second == team, "TeamFactory::Add_Team_Prototype_To_List: Team %s already exists... skipping.", name.Str());
    } else {
        m_prototypes[key] = team;
    }
}

void TeamFactory::Remove_Team_Prototype_From_List(TeamPrototype *team)
{
    const Utf8String &name = team->Get_Name();
    NameKeyType key = Name_To_Key(name);
    std::map<NameKeyType, TeamPrototype *>::iterator it = m_prototypes.find(key);

    if (it != m_prototypes.end()) {
        m_prototypes.erase(it);
    }
}

TeamPrototype *TeamFactory::Find_Team_Prototype(const Utf8String &name)
{
    NameKeyType key = Name_To_Key(name);
    std::map<NameKeyType, TeamPrototype *>::iterator it = m_prototypes.find(key);

    if (it != m_prototypes.end()) {
        return it->second;
    } else {
        return nullptr;
    }
}

TeamPrototype *TeamFactory::Find_Team_Prototype_By_ID(unsigned int id)
{
    std::map<NameKeyType, TeamPrototype *>::iterator it;

    for (it = m_prototypes.begin(); it != m_prototypes.end(); ++it) {
        if (it->second->Get_ID() == id) {
            return it->second;
        }
    }

    return nullptr;
}

Team *TeamFactory::Find_Team_By_ID(unsigned int id)
{
    if (id == 0) {
        return nullptr;
    }

    std::map<NameKeyType, TeamPrototype *>::iterator it;

    for (it = m_prototypes.begin(); it != m_prototypes.end(); ++it) {
        Team *team = it->second->Find_Team_By_ID(id);

        if (team != nullptr) {
            return team;
        }
    }

    return nullptr;
}

Team *TeamFactory::Create_Inactive_Team(const Utf8String &name)
{
    TeamPrototype *proto = Find_Team_Prototype(name);

    if (proto == nullptr) {
        throw CODE_03;
    }

    if (proto->Get_Singleton()) {
        Team *team = proto->Get_First_Item_In_Team_Instance_List();

        if (team != nullptr) {
            if (proto->Get_Template_Info()->m_executesActionsOnCreate) {
                Script *script = g_theScriptEngine->Find_Script_By_Name(proto->Get_Template_Info()->m_productionCondition);

                if (script != nullptr) {
                    g_theScriptEngine->Friend_Execute_Action(script->Get_Action(), nullptr);
                }
            }

            return team;
        }
    }

    Team *team = new Team(proto, ++m_nextTeamID);

    if (proto->Get_Template_Info()->m_executesActionsOnCreate) {
        Script *script = g_theScriptEngine->Find_Script_By_Name(proto->Get_Template_Info()->m_productionCondition);

        if (script != nullptr) {
            g_theScriptEngine->Friend_Execute_Action(script->Get_Action(), nullptr);
        }
    }

    return team;
}

Team *TeamFactory::Create_Team(const Utf8String &name)
{
    Team *team = Create_Inactive_Team(name);
    team->Set_Active();
    return team;
}

Team *TeamFactory::Create_Team_On_Prototype(TeamPrototype *proto)
{
    if (proto == nullptr) {
        throw CODE_03;
    }

    Team *team = nullptr;

    if (proto->Get_Singleton()) {
        team = proto->Get_First_Item_In_Team_Instance_List();

        if (team != nullptr) {
            return team;
        }
    }

    team = new Team(proto, ++m_nextTeamID);
    team->Set_Active();
    return team;
}

Team *TeamFactory::Find_Team(const Utf8String &name)
{
    TeamPrototype *proto = Find_Team_Prototype(name);

    if (proto == nullptr) {
        return nullptr;
    }

    Team *team = proto->Get_First_Item_In_Team_Instance_List();

    if (team == nullptr && !proto->Get_Singleton()) {
        return Create_Inactive_Team(name);
    }

    return team;
}

void TeamFactory::Team_About_To_Be_Deleted(Team *team)
{
    std::map<NameKeyType, TeamPrototype *>::iterator it;

    for (it = m_prototypes.begin(); it != m_prototypes.end(); ++it) {
        it->second->Team_About_To_Be_Deleted(team);
    }

    if (g_thePlayerList != nullptr) {
        g_thePlayerList->Team_About_To_Be_Deleted(team);
    }
}

TeamTemplateInfo::TeamTemplateInfo(Dict *d) : m_numUnitsInfo(0)
{
    Utf8String str;
    bool exists;
    int min = d->Get_Int(g_teamUnitMinCount1Key, &exists);
    int max = d->Get_Int(g_teamUnitMaxCount1Key, &exists);
    str = d->Get_AsciiString(g_teamUnitType1Key, &exists);

    if (max > 0 && exists) {
        m_unitsInfo[m_numUnitsInfo].min_units = min;
        m_unitsInfo[m_numUnitsInfo].max_units = max;
        m_unitsInfo[m_numUnitsInfo].unit_thing_name = str;
        m_numUnitsInfo++;
    }

    min = d->Get_Int(g_teamUnitMinCount2Key, &exists);
    max = d->Get_Int(g_teamUnitMaxCount2Key, &exists);
    str = d->Get_AsciiString(g_teamUnitType2Key, &exists);

    if (max > 0 && exists) {
        m_unitsInfo[m_numUnitsInfo].min_units = min;
        m_unitsInfo[m_numUnitsInfo].max_units = max;
        m_unitsInfo[m_numUnitsInfo].unit_thing_name = str;
        m_numUnitsInfo++;
    }

    min = d->Get_Int(g_teamUnitMinCount3Key, &exists);
    max = d->Get_Int(g_teamUnitMaxCount3Key, &exists);
    str = d->Get_AsciiString(g_teamUnitType3Key, &exists);

    if (max > 0 && exists) {
        m_unitsInfo[m_numUnitsInfo].min_units = min;
        m_unitsInfo[m_numUnitsInfo].max_units = max;
        m_unitsInfo[m_numUnitsInfo].unit_thing_name = str;
        m_numUnitsInfo++;
    }

    min = d->Get_Int(g_teamUnitMinCount4Key, &exists);
    max = d->Get_Int(g_teamUnitMaxCount4Key, &exists);
    str = d->Get_AsciiString(g_teamUnitType4Key, &exists);

    if (max > 0 && exists) {
        m_unitsInfo[m_numUnitsInfo].min_units = min;
        m_unitsInfo[m_numUnitsInfo].max_units = max;
        m_unitsInfo[m_numUnitsInfo].unit_thing_name = str;
        m_numUnitsInfo++;
    }

    min = d->Get_Int(g_teamUnitMinCount5Key, &exists);
    max = d->Get_Int(g_teamUnitMaxCount5Key, &exists);
    str = d->Get_AsciiString(g_teamUnitType5Key, &exists);

    if (max > 0 && exists) {
        m_unitsInfo[m_numUnitsInfo].min_units = min;
        m_unitsInfo[m_numUnitsInfo].max_units = max;
        m_unitsInfo[m_numUnitsInfo].unit_thing_name = str;
        m_numUnitsInfo++;
    }

    min = d->Get_Int(g_teamUnitMinCount6Key, &exists);
    max = d->Get_Int(g_teamUnitMaxCount6Key, &exists);
    str = d->Get_AsciiString(g_teamUnitType6Key, &exists);

    if (max > 0 && exists) {
        m_unitsInfo[m_numUnitsInfo].min_units = min;
        m_unitsInfo[m_numUnitsInfo].max_units = max;
        m_unitsInfo[m_numUnitsInfo].unit_thing_name = str;
        m_numUnitsInfo++;
    }

    min = d->Get_Int(g_teamUnitMinCount7Key, &exists);
    max = d->Get_Int(g_teamUnitMaxCount7Key, &exists);
    str = d->Get_AsciiString(g_teamUnitType7Key, &exists);

    if (max > 0 && exists) {
        m_unitsInfo[m_numUnitsInfo].min_units = min;
        m_unitsInfo[m_numUnitsInfo].max_units = max;
        m_unitsInfo[m_numUnitsInfo].unit_thing_name = str;
        m_numUnitsInfo++;
    }

    str = d->Get_AsciiString(g_teamHomeKey, &exists);

    m_homeLocation.x = 0.0f;
    m_homeLocation.y = 0.0f;
    m_homeLocation.z = 0.0f;
    m_hasHomeLocation = false;

    if (exists) {
        for (const Waypoint *i = g_theTerrainLogic->Get_First_Waypoint(); i != nullptr; i = i->Get_Next()) {
            if (i->Get_Name() == str) {
                m_homeLocation = *i->Get_Location();
                m_hasHomeLocation = true;
            }
        }
    }

    m_scriptOnCreate = d->Get_AsciiString(g_teamOnCreateScriptKey, &exists);
    m_isAIRecruitable = d->Get_Bool(g_teamIsAIRecruitableKey, &exists);

    if (!exists) {
        m_isAIRecruitable = false;
    }

    m_isBaseDefense = d->Get_Bool(g_teamIsBaseDefenseKey, &exists);
    m_isPerimeterDefense = d->Get_Bool(g_teamIsPerimeterDefenseKey, &exists);
    m_automaticallyReinforce = d->Get_Bool(g_teamAutoReinforceKey, &exists);
    AttitudeType aggressiveness = static_cast<AttitudeType>(d->Get_Int(g_teamAggressivenessKey, &exists));
    m_initialTeamAttitude = AI_NORMAL;

    if (exists) {
        m_initialTeamAttitude = aggressiveness;
    }

    m_transportsReturn = d->Get_Bool(g_teamTransportsReturnKey, &exists);
    m_avoidThreats = d->Get_Bool(g_teamAvoidThreatsKey, &exists);
    m_attackCommonTarget = d->Get_Bool(g_teamAttackCommonTargetKey, &exists);
    m_maxInstances = d->Get_Int(g_teamMaxInstancesKey, &exists);
    m_scriptOnIdle = d->Get_AsciiString(g_teamOnIdleScriptKey, &exists);
    m_initialIdleFrames = d->Get_Int(g_teamInitialIdleFramesKey, &exists);
    m_scriptOnEnemySighted = d->Get_AsciiString(g_teamEnemySightedScriptKey, &exists);
    m_scriptOnAllClear = d->Get_AsciiString(g_teamAllClearScriptKey, &exists);
    m_scriptOnDestroyed = d->Get_AsciiString(g_teamOnDestroyedScriptKey, &exists);
    m_destroyedThreshold = d->Get_Real(g_teamDestroyedThresholdKey, &exists);
    m_scriptOnUnitDestroyed = d->Get_AsciiString(g_teamOnUnitDestroyedScriptKey, &exists);
    m_productionPriority = d->Get_Int(g_teamProductionPriorityKey, &exists);
    m_productionPrioritySuccessIncrease = d->Get_Int(g_teamProductionPrioritySuccessIncreaseKey, &exists);
    m_productionPriorityFailureDecrease = d->Get_Int(g_teamProductionPriorityFailureDecreaseKey, &exists);
    m_productionCondition = d->Get_AsciiString(g_teamProductionConditionKey, &exists);
    m_executesActionsOnCreate = d->Get_Bool(g_teamExecutesActionsOnCreateKey, &exists);

    for (int i = 0; i < ARRAY_SIZE(m_genericScripts); i++) {
        Utf8String str;
        Utf8String name = g_theNameKeyGenerator->Key_To_Name(g_teamGenericScriptHookKey);
        str.Format("%s%d", name.Str(), i);
        m_genericScripts[i] = d->Get_AsciiString(Name_To_Key(str), &exists);

        if (!exists) {
            m_genericScripts[i].Clear();
        }
    }

    m_transportUnitType = d->Get_AsciiString(g_teamTransportKey, &exists);
    m_transportsExit = d->Get_Bool(g_teamTransportsExitKey, &exists);
    m_teamStartsFull = d->Get_Bool(g_teamStartsFullKey, &exists);
    m_startReinforceWaypoint = d->Get_AsciiString(g_teamReinforcementOriginKey, &exists);
    m_veterancy = d->Get_Int(g_teamVeterancyKey, &exists);
}

void TeamTemplateInfo::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t version = 1;
    xfer->xferVersion(&version, 1);
    xfer->xferInt(&m_productionPriority);
}

TeamPrototype::TeamPrototype(
    TeamFactory *tf, const Utf8String &name, Player *owner_player, bool is_singleton, Dict *d, unsigned int id) :
    m_factory(tf),
    m_owningPlayer(owner_player),
    m_ID(id),
    m_name(name),
    m_flags(is_singleton),
    m_productionConditionAlwaysFalse(false),
    m_productionConditionScript(nullptr),
    m_teamTemplate(d)
{
    captainslog_dbgassert(m_owningPlayer != nullptr, "bad args to TeamPrototype ctor");

    if (m_factory != nullptr) {
        m_factory->Add_Team_Prototype_To_List(this);
    }

    if (m_owningPlayer != nullptr) {
        m_owningPlayer->Add_Team_To_List(this);
    }

    m_genericScriptsInitalized = false;

    for (int i = 0; i < ARRAY_SIZE(m_genericScripts); i++) {
        m_genericScripts[i] = nullptr;
    }
}

void Delete_Team_Callback(Team *team)
{
    if (team != nullptr) {
        g_theTeamFactory->Team_About_To_Be_Deleted(team);
        team->Delete_Instance();
    }
}

TeamPrototype::~TeamPrototype()
{
    Remove_All_Team_Instance_List(Delete_Team_Callback);

    if (m_owningPlayer != nullptr) {
        m_owningPlayer->Remove_Team_From_List(this);
    }

    if (m_factory != nullptr) {
        m_factory->Remove_Team_Prototype_From_List(this);
    }

    if (m_productionConditionScript != nullptr) {
        m_productionConditionScript->Delete_Instance();
    }

    m_productionConditionScript = nullptr;

    for (int i = 0; i < ARRAY_SIZE(m_genericScripts); i++) {
        if (m_genericScripts[i] != nullptr) {
            m_genericScripts[i]->Delete_Instance();
            m_genericScripts[i] = nullptr;
        }
    }
}

void TeamPrototype::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 2;
    xfer->xferVersion(&version, 2);

    int index;

    if (xfer->Get_Mode() == XFER_SAVE) {
        index = m_owningPlayer->Get_Player_Index();
    }

    xfer->xferInt(&index);

    m_owningPlayer = g_thePlayerList->Get_Nth_Player(index);

    if (version >= 2) {
        xfer->xferAsciiString(&m_attackPriorityName);
    }

    xfer->xferBool(&m_productionConditionAlwaysFalse);
    xfer->xferSnapshot(&m_teamTemplate);
    unsigned short size = 0;
    DLINK_ITERATOR<Team> iter = Iterate_Team_Instance_List();

    while (!iter.Done()) {
        size++;
        iter.Advance();
    }

    xfer->xferUnsignedShort(&size);

    if (xfer->Get_Mode() == XFER_SAVE) {
        DLINK_ITERATOR<Team> iter2 = Iterate_Team_Instance_List();

        while (!iter2.Done()) {
            Team *team = iter2.Cur();
            unsigned int team_id = team->Get_Team_ID();
            xfer->xferUser(&team_id, sizeof(team_id));
            xfer->xferSnapshot(team);
            iter2.Advance();
        }
    } else {
        for (int i = 0; i < size; i++) {
            unsigned int team_id;
            xfer->xferUser(&team_id, sizeof(team_id));
            Team *team = g_theTeamFactory->Find_Team_By_ID(team_id);

            if (team == nullptr) {
                team = g_theTeamFactory->Create_Team_On_Prototype(this);
                team->Set_ID(team_id);
            }

            xfer->xferSnapshot(team);
        }
    }
}

void TeamPrototype::Team_About_To_Be_Deleted(Team *team)
{
    DLINK_ITERATOR<Team> iter = Iterate_Team_Instance_List();

    while (!iter.Done()) {
        unsigned int team_id;

        if (team != nullptr) {
            team_id = team->Get_Team_ID();
        } else {
            team_id = 0;
        }

        iter.Cur()->Remove_Override_Team_Relationship(team_id);
        iter.Advance();
    }
}

Team *TeamPrototype::Find_Team_By_ID(unsigned int id)
{
    DLINK_ITERATOR<Team> iter = Iterate_Team_Instance_List();

    while (!iter.Done()) {
        if (iter.Cur()->Get_Team_ID() == id) {
            return iter.Cur();
        }

        iter.Advance();
    }

    return nullptr;
}

Team::Team(TeamPrototype *proto, unsigned int id) :
    m_proto(proto),
    m_teamID(id),
    m_enteredOrExited(false),
    m_active(false),
    m_checkEnemySighted(false),
    m_seeEnemy(false),
    m_prevSeeEnemy(false),
    m_wasIdle(false),
    m_destroyThreshold(0),
    m_curUnits(0),
    m_canRecruit(false),
    m_availableForRecruitment(false),
    m_currentWaypoint(nullptr)
{
    m_created = false;
    m_targetObjectID = OBJECT_UNK;
    m_playerRelations = new PlayerRelationMap();
    m_teamRelations = new TeamRelationMap();

    if (proto != nullptr) {
        proto->Prepend_To_Team_Instance_List(this);
        const TeamTemplateInfo *info = proto->Get_Template_Info();

        if (!info->m_scriptOnAllClear.Is_Empty() || !info->m_scriptOnEnemySighted.Is_Empty()) {
            m_checkEnemySighted = true;
        }
    }

    for (int i = 0; i < ARRAY_SIZE(m_genericScriptActive); i++) {
        m_genericScriptActive[i] = true;
    }
}

Team::~Team()
{
    g_theScriptEngine->Notify_Of_Team_Destruction(this);

    for (int i = 0; i < g_thePlayerList->Get_Player_Count(); i++) {
        Player *player = g_thePlayerList->Get_Nth_Player(i);

        if (player != nullptr) {
            player->Pre_Team_Destroy(this);
        }
    }

    for (;;) {
        Object *obj = Get_First_Item_In_Team_Member_List();

        if (obj == nullptr) {
            break;
        }

        obj->Set_Team(nullptr);
    }

    captainslog_dbgassert(m_proto != nullptr, "proto should not be null");

    if (m_proto != nullptr && m_proto->Is_In_List_Team_Instance_List(this)) {
        m_proto->Remove_From_Team_Instance_List(this);
    }

    m_teamRelations->Delete_Instance();
    m_playerRelations->Delete_Instance();
}

void Team::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    unsigned int team_id = m_teamID;
    xfer->xferUser(&team_id, sizeof(team_id));
    captainslog_relassert(
        team_id == m_teamID, 6, "Team::xfer - TeamID mismatch.  Xfered '%d' but should be '%d'", team_id, m_teamID);
    unsigned short size = 0;
    DLINK_ITERATOR<Object> iter = Iterate_Team_Member_List();

    while (!iter.Done()) {
        size++;
        iter.Advance();
    }

    xfer->xferUnsignedShort(&size);

    if (xfer->Get_Mode() == XFER_SAVE) {
        DLINK_ITERATOR<Object> iter2 = Iterate_Team_Member_List();

        while (!iter2.Done()) {
            Object *obj = iter2.Cur();
            ObjectID id = obj->Get_ID();
            xfer->xferObjectID(&id);
            iter2.Advance();
        }
    } else {
        for (int i = 0; i < size; i++) {
            ObjectID id;
            xfer->xferObjectID(&id);
            m_objectIDList.push_back(id);
        }
    }

    xfer->xferAsciiString(&m_state);
    xfer->xferBool(&m_enteredOrExited);
    xfer->xferBool(&m_active);
    xfer->xferBool(&m_created);
    xfer->xferBool(&m_checkEnemySighted);
    xfer->xferBool(&m_seeEnemy);
    xfer->xferBool(&m_prevSeeEnemy);
    xfer->xferBool(&m_wasIdle);
    xfer->xferInt(&m_destroyThreshold);
    xfer->xferInt(&m_curUnits);
    WaypointID waypoint_id;

    if (m_currentWaypoint != nullptr) {
        waypoint_id = m_currentWaypoint->Get_ID();
    } else {
        waypoint_id = WAYPOINT_UNK;
    }

    xfer->xferUnsignedInt(reinterpret_cast<unsigned int *>(&waypoint_id));

    if (xfer->Get_Mode() == XFER_LOAD) {
        m_currentWaypoint = g_theTerrainLogic->Get_Waypoint_By_ID(waypoint_id);
    }

    unsigned short count = ARRAY_SIZE(m_genericScriptActive);
    xfer->xferUnsignedShort(&count);
    captainslog_relassert(count == ARRAY_SIZE(m_genericScriptActive),
        6,
        "Team::xfer - The number of allowable Generic scripts has changed, and this chunk needs to be versioned.");

    for (int i = 0; i < count; i++) {
        xfer->xferBool(&m_genericScriptActive[i]);
    }

    xfer->xferBool(&m_canRecruit);
    xfer->xferBool(&m_availableForRecruitment);
    xfer->xferObjectID(&m_targetObjectID);
    xfer->xferSnapshot(m_teamRelations);
    xfer->xferSnapshot(m_playerRelations);
}

void Team::Load_Post_Process()
{
    std::list<ObjectID>::iterator it;
    for (it = m_objectIDList.begin(); it != m_objectIDList.end(); ++it) {
        ObjectID id = *it;
        Object *obj = g_theGameLogic->Find_Object_By_ID(id);
        captainslog_relassert(obj != nullptr,
            6,
            "Team::loadPostProcess - Unable to post process object to member list, object ID = '%d'",
            *it);
        captainslog_relassert(Is_In_List_Team_Member_List(obj),
            6,
            "Team::loadPostProcess - Object '%s'(%d) should be in team list but is not",
            obj->Get_Template()->Get_Name().Str(),
            obj->Get_ID());
    }

    m_objectIDList.clear();
}

bool Team::Remove_Override_Team_Relationship(unsigned int id)
{
    if (m_teamRelations->m_relationships.empty()) {
        return false;
    }

    if (id == 0) {
        m_teamRelations->m_relationships.clear();
        return true;
    }

    auto it = m_teamRelations->m_relationships.find(id);

    if (!(it != m_teamRelations->m_relationships.end())) {
        return false;
    }

    m_teamRelations->m_relationships.erase(it);
    return true;
}
