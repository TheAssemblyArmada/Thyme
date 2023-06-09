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
#include "ai.h"
#include "damage.h"
#include "gamelogic.h"
#include "opencontain.h"
#include "partitionmanager.h"
#include "playerlist.h"
#include "playertemplate.h"
#include "script.h"
#include "scriptengine.h"
#include "sideslist.h"
#include "staticnamekey.h"
#include "terrainlogic.h"
#include "thingfactory.h"
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
    Player *player = g_thePlayerList->Find_Player_With_NameKey(Name_To_Key(owner.Str()));
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
    NameKeyType key = Name_To_Key(name.Str());
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
    NameKeyType key = Name_To_Key(name.Str());
    std::map<NameKeyType, TeamPrototype *>::iterator it = m_prototypes.find(key);

    if (it != m_prototypes.end()) {
        m_prototypes.erase(it);
    }
}

TeamPrototype *TeamFactory::Find_Team_Prototype(const Utf8String &name)
{
    NameKeyType key = Name_To_Key(name.Str());
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
        Utf8String str2;
        Utf8String name = g_theNameKeyGenerator->Key_To_Name(g_teamGenericScriptHookKey);
        str2.Format("%s%d", name.Str(), i);
        m_genericScripts[i] = d->Get_AsciiString(Name_To_Key(str2.Str()), &exists);

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
    m_currentWaypoint(nullptr),
    m_canRecruit(false),
    m_availableForRecruitment(false)
{
    m_created = false;
    m_targetObjectID = INVALID_OBJECT_ID;
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
        waypoint_id = INVALID_WAYPOINT_ID;
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

Player *TeamPrototype::Get_Controlling_Player() const
{
    return m_owningPlayer;
}

void TeamPrototype::Set_Controlling_Player(Player *new_controller)
{
    captainslog_dbgassert(new_controller != nullptr, "Attempted to set NULL player as team-owner, illegal.");

    if (new_controller != nullptr) {
        if (m_owningPlayer != nullptr) {
            m_owningPlayer->Remove_Team_From_List(this);
        }

        m_owningPlayer = new_controller;
        m_owningPlayer->Add_Team_To_List(this);
    }
}

void TeamPrototype::Count_Objects_By_Thing_Template(int num_tmplates,
    const ThingTemplate *const *things,
    bool ignore_dead,
    int *counts,
    bool ignore_under_construction) const
{
    DLINK_ITERATOR<Team> it = Iterate_Team_Instance_List();

    while (!it.Done()) {
        it.Cur()->Count_Objects_By_Thing_Template(num_tmplates, things, ignore_dead, counts, ignore_under_construction);
        it.Advance();
    }
}

Script *TeamPrototype::Get_Generic_Script(int script)
{
    if (!m_genericScriptsInitalized) {
        m_genericScriptsInitalized = true;

        for (int i = 0; i < ARRAY_SIZE(m_genericScripts); i++) {
            Script *new_script = nullptr;

            if (!m_teamTemplate.m_genericScripts[i].Is_Empty()) {
                Script *found_script = g_theScriptEngine->Find_Script_By_Name(m_teamTemplate.m_genericScripts[i]);

                if (found_script != nullptr) {
                    new_script = found_script->Duplicate();
                } else {
                    captainslog_dbgassert(false,
                        "We attempted to find a generic script, but couldn't. ('%s')",
                        m_teamTemplate.m_genericScripts[i].Str());
                }
            }

            m_genericScripts[i] = new_script;
        }
    }

    return m_genericScripts[script];
}

void TeamPrototype::Increase_AI_Priority_For_Success()
{
    m_teamTemplate.m_productionPriority += m_teamTemplate.m_productionPrioritySuccessIncrease;
}

void TeamPrototype::Decrease_AI_Priority_For_Failure()
{
    m_teamTemplate.m_productionPriority -= m_teamTemplate.m_productionPriorityFailureDecrease;
}

int TeamPrototype::Count_Buildings()
{
    int count = 0;
    DLINK_ITERATOR<Team> it = Iterate_Team_Instance_List();

    while (!it.Done()) {
        count += it.Cur()->Count_Buildings();
        it.Advance();
    }

    return count;
}

int TeamPrototype::Count_Objects(BitFlags<KINDOF_COUNT> must_be_set, BitFlags<KINDOF_COUNT> must_be_clear)
{
    int count = 0;
    DLINK_ITERATOR<Team> it = Iterate_Team_Instance_List();

    while (!it.Done()) {
        count += it.Cur()->Count_Objects(must_be_set, must_be_clear);
        it.Advance();
    }

    return count;
}

void TeamPrototype::Heal_All_Objects()
{
    DLINK_ITERATOR<Team> it = Iterate_Team_Instance_List();

    while (!it.Done()) {
        it.Cur()->Heal_All_Objects();
        it.Advance();
    }
}

void TeamPrototype::Iterate_Objects(void (*func)(Object *, void *), void *data)
{
    DLINK_ITERATOR<Team> it = Iterate_Team_Instance_List();

    while (!it.Done()) {
        it.Cur()->Iterate_Objects(func, data);
        it.Advance();
    }
}

int TeamPrototype::Count_Team_Instances()
{
    int count = 0;
    DLINK_ITERATOR<Team> it = Iterate_Team_Instance_List();

    while (!it.Done()) {
        count++;
        it.Advance();
    }

    return count;
}

bool TeamPrototype::Has_Any_Buildings() const
{
    DLINK_ITERATOR<Team> it = Iterate_Team_Instance_List();

    while (!it.Done()) {
        if (it.Cur()->Has_Any_Buildings()) {
            return true;
        }

        it.Advance();
    }

    return false;
}

bool TeamPrototype::Has_Any_Buildings(BitFlags<KINDOF_COUNT> must_be_set) const
{
    DLINK_ITERATOR<Team> it = Iterate_Team_Instance_List();

    while (!it.Done()) {
        if (it.Cur()->Has_Any_Buildings(must_be_set)) {
            return true;
        }

        it.Advance();
    }

    return false;
}

bool TeamPrototype::Has_Any_Units() const
{
    DLINK_ITERATOR<Team> it = Iterate_Team_Instance_List();

    while (!it.Done()) {
        if (it.Cur()->Has_Any_Units()) {
            return true;
        }

        it.Advance();
    }

    return false;
}

bool TeamPrototype::Has_Any_Objects() const
{
    DLINK_ITERATOR<Team> it = Iterate_Team_Instance_List();

    while (!it.Done()) {
        if (it.Cur()->Has_Any_Objects()) {
            return true;
        }

        it.Advance();
    }

    return false;
}

void TeamPrototype::Update_State()
{
    DLINK_ITERATOR<Team> it1 = Iterate_Team_Instance_List();

    while (!it1.Done()) {
        it1.Cur()->Update_State();
        it1.Advance();
    }

    bool done = false;

    while (!done) {
        done = true;
        DLINK_ITERATOR<Team> it2 = Iterate_Team_Instance_List();

        while (!it2.Done()) {
            if (!it2.Cur()->Get_First_Item_In_Team_Member_List() && !Get_Singleton()) {
                if (!it2.Cur()->Get_Controlling_Player()
                    || it2.Cur()->Get_Controlling_Player()->Get_Default_Team() != it2.Cur()) {
                    if (it2.Cur()->Is_Active()) {
                        g_theTeamFactory->Team_About_To_Be_Deleted(it2.Cur());
                        it2.Cur()->Delete_Instance();
                        done = false;
                        break;
                    }
                }
            }

            it2.Advance();
        }
    }
}

bool TeamPrototype::Has_Any_Build_Facility() const
{
    DLINK_ITERATOR<Team> it = Iterate_Team_Instance_List();

    while (!it.Done()) {
        if (it.Cur()->Has_Any_Build_Facility()) {
            return true;
        }

        it.Advance();
    }

    return false;
}

void TeamPrototype::Damage_Team_Members(float amount)
{
    DLINK_ITERATOR<Team> it = Iterate_Team_Instance_List();

    while (!it.Done()) {
        it.Cur()->Damage_Team_Members(amount);
        it.Advance();
    }
}

void TeamPrototype::Move_Team_To(Coord3D destination)
{
    DLINK_ITERATOR<Team> it = Iterate_Team_Instance_List();

    while (!it.Done()) {
        it.Cur()->Move_Team_To(destination);
        it.Advance();
    }
}

bool TeamPrototype::Evaluate_Production_Condition()
{
    if (m_productionConditionAlwaysFalse) {
        return false;
    }

    if (m_productionConditionScript != nullptr) {
        if (g_theGameLogic->Get_Frame() < static_cast<unsigned int>(m_productionConditionScript->Get_Evaluation_Frame())) {
            return false;
        } else {
            int interval = m_productionConditionScript->Get_Evaluation_Interval();

            if (interval > 0) {
                m_productionConditionScript->Set_Evaluation_Frame(30 * interval + g_theGameLogic->Get_Frame());
            }

            return g_theScriptEngine->Evaluate_Conditions(m_productionConditionScript, nullptr, Get_Controlling_Player());
        }
    } else {
        if (m_teamTemplate.m_productionCondition.Is_Empty()) {
            m_productionConditionAlwaysFalse = true;
            return false;
        }

        Script *script = g_theScriptEngine->Find_Script_By_Name(m_teamTemplate.m_productionCondition);

        if (script != nullptr) {
            GameDifficulty difficulty = Get_Controlling_Player()->Get_Player_Difficulty();

            if (difficulty != DIFFICULTY_EASY) {
                if (difficulty == DIFFICULTY_NORMAL) {
                    if (!script->Is_Normal()) {
                        m_productionConditionAlwaysFalse = true;
                        return false;
                    }
                } else if (difficulty == DIFFICULTY_HARD && !script->Is_Hard()) {
                    m_productionConditionAlwaysFalse = true;
                    return false;
                }
            } else if (!script->Is_Easy()) {
                m_productionConditionAlwaysFalse = true;
                return false;
            }

            m_productionConditionScript = script->Duplicate();
            return g_theScriptEngine->Evaluate_Conditions(m_productionConditionScript, nullptr, Get_Controlling_Player());
        } else {
            m_productionConditionAlwaysFalse = true;
            return false;
        }
    }
}

void Team::Count_Objects_By_Thing_Template(int num_tmplates,
    const ThingTemplate *const *things,
    bool ignore_dead,
    int *counts,
    bool ignore_under_construction) const
{
    DLINK_ITERATOR<Object> it = Iterate_Team_Member_List();

    while (!it.Done()) {
        const ThingTemplate *t = it.Cur()->Get_Template();

        for (int i = 0; i < num_tmplates; i++) {
            if (t->Is_Equivalent_To(things[i])) {
                if (!ignore_dead || !it.Cur()->Is_Effectively_Dead()) {
                    if (!ignore_under_construction || !it.Cur()->Get_Status_Bits().Test(OBJECT_STATUS_UNDER_CONSTRUCTION)) {
                        counts[i]++;
                    }
                }
            }
        }

        it.Advance();
    }
}

void Team::Iterate_Objects(void (*func)(Object *, void *), void *data)
{
    DLINK_ITERATOR<Object> it = Iterate_Team_Member_List();

    while (!it.Done()) {
        func(it.Cur(), data);
        it.Advance();
    }
}

void Team::Heal_All_Objects()
{
    DLINK_ITERATOR<Object> it = Iterate_Team_Member_List();

    while (!it.Done()) {
        it.Cur()->Heal_Completely();
        it.Advance();
    }
}

int Team::Count_Buildings()
{
    int count = 0;
    DLINK_ITERATOR<Object> it = Iterate_Team_Member_List();

    while (!it.Done()) {
        const ThingTemplate *t = it.Cur()->Get_Template();

        if (t != nullptr) {
            if (t->Is_KindOf(KINDOF_STRUCTURE)) {
                count++;
            }
        }

        it.Advance();
    }

    return count;
}

int Team::Count_Objects(BitFlags<KINDOF_COUNT> must_be_set, BitFlags<KINDOF_COUNT> must_be_clear)
{
    int count = 0;
    DLINK_ITERATOR<Object> it = Iterate_Team_Member_List();

    while (!it.Done()) {
        const ThingTemplate *t = it.Cur()->Get_Template();

        if (t != nullptr) {
            if (t->Is_KindOf_Multi(must_be_set, must_be_clear)) {
                count++;
            }
        }

        it.Advance();
    }

    return count;
}

bool Team::Has_Any_Buildings() const
{
    DLINK_ITERATOR<Object> it = Iterate_Team_Member_List();

    while (!it.Done()) {

        if (!it.Cur()->Is_Effectively_Dead()) {
            if (!it.Cur()->Is_Destroyed()) {
                if (it.Cur()->Is_KindOf(KINDOF_STRUCTURE)) {
                    return true;
                }
            }
        }

        it.Advance();
    }

    return false;
}

bool Team::Has_Any_Buildings(BitFlags<KINDOF_COUNT> must_be_set) const
{
    DLINK_ITERATOR<Object> it = Iterate_Team_Member_List();

    while (!it.Done()) {

        if (!it.Cur()->Is_Effectively_Dead()) {
            if (!it.Cur()->Is_Destroyed()) {
                must_be_set.Set(KINDOF_STRUCTURE, true);

                if (it.Cur()->Is_KindOf_Multi(must_be_set, KINDOFMASK_NONE)) {
                    return true;
                }
            }
        }

        it.Advance();
    }

    return false;
}

bool Team::Has_Any_Units() const
{
    DLINK_ITERATOR<Object> it = Iterate_Team_Member_List();

    while (!it.Done()) {

        if (!it.Cur()->Is_Effectively_Dead()) {
            if (!it.Cur()->Is_Destroyed()) {
                if (!it.Cur()->Is_KindOf(KINDOF_STRUCTURE)) {
                    if (!it.Cur()->Is_KindOf(KINDOF_PROJECTILE)) {
                        if (!it.Cur()->Is_KindOf(KINDOF_MINE)) {
                            return true;
                        }
                    }
                }
            }
        }

        it.Advance();
    }

    return false;
}

bool Team::Has_Any_Objects() const
{
    DLINK_ITERATOR<Object> it = Iterate_Team_Member_List();

    while (!it.Done()) {

        if (!it.Cur()->Is_Effectively_Dead()) {
            if (!it.Cur()->Is_Destroyed()) {
                if (!it.Cur()->Is_KindOf(KINDOF_PROJECTILE)) {
                    if (!it.Cur()->Is_KindOf(KINDOF_INERT)) {
                        if (!it.Cur()->Is_KindOf(KINDOF_MINE)) {
                            return true;
                        }
                    }
                }
            }
        }

        it.Advance();
    }

    return false;
}

void Team::Update_State()
{
    m_enteredOrExited = false;

    if (m_active) {
        const TeamTemplateInfo *info = m_proto->Get_Template_Info();

        if (m_created) {
            m_created = false;

            if (!info->m_scriptOnCreate.Is_Empty()) {
                g_theScriptEngine->Run_Script(info->m_scriptOnCreate, this);
            }

            if (!info->m_scriptOnDestroyed.Is_Empty()) {
                DLINK_ITERATOR<Object> it = Iterate_Team_Member_List();

                while (!it.Done()) {
                    m_curUnits++;
                    it.Advance();
                }

                m_destroyThreshold = (m_curUnits - m_curUnits * info->m_destroyedThreshold);

                if (m_destroyThreshold > m_curUnits - 1) {
                    m_destroyThreshold = m_curUnits - 1;
                }

                if (m_destroyThreshold < 0) {
                    m_destroyThreshold = 0;
                }
            }
        }

        if (m_checkEnemySighted) {
            m_prevSeeEnemy = m_seeEnemy;
            m_seeEnemy = false;
            bool found_object = false;

            DLINK_ITERATOR<Object> it = Iterate_Team_Member_List();

            while (!it.Done()) {
                if (!it.Cur()->Is_Effectively_Dead()) {
                    PartitionFilterRelationship r(it.Cur(), 1);
                    PartitionFilterAlive a;
                    PartitionFilterSameMapStatus s(it.Cur());
                    PartitionFilter *filters[] = { &r, &a, &s, nullptr };
                    found_object = true;

                    if (g_thePartitionManager->Get_Closest_Object(
                            it.Cur(), it.Cur()->Get_Vision_Range(), FROM_CENTER_2D, filters, nullptr, nullptr)) {
                        m_seeEnemy = true;
                        break;
                    }
                }

                it.Advance();
            }

            if (found_object && m_prevSeeEnemy != m_seeEnemy) {
                if (m_seeEnemy) {
                    g_theScriptEngine->Run_Script(info->m_scriptOnEnemySighted, this);
                } else {
                    g_theScriptEngine->Run_Script(info->m_scriptOnAllClear, this);
                }
            }
        }

        if (!info->m_scriptOnDestroyed.Is_Empty()) {
            int units = m_curUnits;
            m_curUnits = 0;
            DLINK_ITERATOR<Object> it = Iterate_Team_Member_List();

            while (!it.Done()) {
                if (!it.Cur()->Is_Effectively_Dead()) {
                    m_curUnits++;
                }

                it.Advance();
            }

            if (m_curUnits != units && m_curUnits <= m_destroyThreshold) {
                g_theScriptEngine->Run_Script(info->m_scriptOnDestroyed, this);
                m_destroyThreshold = -1;
            }
        }

        if (!info->m_scriptOnIdle.Is_Empty()) {
            bool is_idle = true;
            bool found_update = false;
            DLINK_ITERATOR<Object> it = Iterate_Team_Member_List();

            while (!it.Done()) {
                if (!it.Cur()->Is_Effectively_Dead()) {
                    AIUpdateInterface *update = it.Cur()->Get_AI_Update_Interface();

                    if (update != nullptr) {
                        found_update = true;

                        if (!update->Is_Idle()) {
                            is_idle = false;
                        }
                    }
                }

                it.Advance();
            }

            if (found_update && is_idle && m_wasIdle) {
                g_theScriptEngine->Run_Script(info->m_scriptOnIdle, this);
            }

            m_wasIdle = is_idle;
        }
    }
}

Player *Team::Get_Controlling_Player() const
{
    return m_proto->Get_Controlling_Player();
}

void Team::Move_Team_To(Coord3D destination)
{
    DLINK_ITERATOR<Object> it = Iterate_Team_Member_List();

    while (!it.Done()) {
        if (!it.Cur()->Is_Effectively_Dead()) {
            it.Cur()->Is_Destroyed();
        }

        it.Advance();
    }
}

bool Team::Has_Any_Build_Facility() const
{
    DLINK_ITERATOR<Object> it = Iterate_Team_Member_List();

    while (!it.Done()) {
        if (!it.Cur()->Is_Effectively_Dead()) {
            if (it.Cur()->Get_Template()->Is_Build_Facility()) {
                return true;
            }
        }

        it.Advance();
    }

    return false;
}

bool Team::Damage_Team_Members(float amount)
{
    DLINK_ITERATOR<Object> it = Iterate_Team_Member_List();

    while (!it.Done()) {
        if (!it.Cur()->Is_Effectively_Dead()) {
            if (!it.Cur()->Is_Destroyed()) {
                if (amount < 0.0f) {
                    it.Cur()->Kill(DAMAGE_UNRESISTABLE, DEATH_NORMAL);
                } else {
                    DamageInfo info;
                    info.m_in.m_damageType = DAMAGE_UNRESISTABLE;
                    info.m_in.m_deathType = DEATH_NORMAL;
                    info.m_in.m_sourceID = INVALID_OBJECT_ID;
                    info.m_in.m_amount = amount;
                    it.Cur()->Attempt_Damage(&info);
                }
            }
        }
    }

    return false;
}

void Team::Set_Controlling_Player(Player *new_controller)
{
    m_proto->Set_Controlling_Player(new_controller);
    DLINK_ITERATOR<Object> it = Iterate_Team_Member_List();

    while (!it.Done()) {
        Object *obj = it.Cur();

        if (obj != nullptr) {
            obj->Handle_Partition_Cell_Maintenance();
        }

        it.Advance();
    }
}

void Team::Set_Attack_Priority_Name(Utf8String name)
{
    if (m_proto != nullptr) {
        m_proto->Set_Attack_Priority_Name(name);
    }
}

void Team::Get_Team_As_AI_Group(AIGroup *aigroup)
{
    if (aigroup != nullptr) {
        DLINK_ITERATOR<Object> it = Iterate_Team_Member_List();

        while (!it.Done()) {
            if (it.Cur() != nullptr) {
                aigroup->Add(it.Cur());
            }

            it.Advance();
        }
    }
}

int Team::Get_Targetable_Count() const
{
    int count = 0;
    DLINK_ITERATOR<Object> it = Iterate_Team_Member_List();

    while (!it.Done()) {
        Object *obj = it.Cur();

        if (obj != nullptr && !obj->Is_Effectively_Dead()
            && (obj->Get_AI_Update_Interface() != nullptr || obj->Is_KindOf(KINDOF_STRUCTURE))) {
            count++;
        }

        it.Advance();
    }

    return count;
}

Relationship Team::Get_Relationship(const Team *that) const
{
    if (!m_teamRelations->m_relationships.empty()) {
        if (that != nullptr) {
            auto it = m_teamRelations->m_relationships.find(that->Get_Team_ID());

            if (it != m_teamRelations->m_relationships.end()) {
                return (*it).second;
            }
        }
    }

    if (!m_playerRelations->m_relationships.empty()) {
        if (that != nullptr) {
            Player *player = that->Get_Controlling_Player();

            if (player != nullptr) {
                auto it = m_playerRelations->m_relationships.find(player->Get_Player_Index());

                if (it != m_playerRelations->m_relationships.end()) {
                    return (*it).second;
                }
            }
        }
    }

    return Get_Controlling_Player()->Get_Relationship(that);
}

void Team::Set_Team_Target_Object(const Object *obj)
{
    if (obj != nullptr) {
        if (Get_Controlling_Player()->Get_Player_Type() == Player::PLAYER_COMPUTER) {
            if (Get_Controlling_Player()->Get_Player_Difficulty() != DIFFICULTY_EASY) {
                m_targetObjectID = obj->Get_ID();
            }
        }
    } else {
        m_targetObjectID = INVALID_OBJECT_ID;
    }
}

Object *Team::Get_Team_Target_Object()
{
    if (m_targetObjectID == INVALID_OBJECT_ID) {
        return nullptr;
    }

    Object *obj = g_theGameLogic->Find_Object_By_ID(m_targetObjectID);

    if (obj != nullptr && obj->Get_Status(OBJECT_STATUS_STEALTHED) && !obj->Get_Status(OBJECT_STATUS_DETECTED)
        && !obj->Get_Status(OBJECT_STATUS_DISGUISED)) {
        obj = nullptr;
    }

    if (obj != nullptr && obj->Is_Effectively_Dead()) {
        obj = nullptr;
    }

    if (obj != nullptr && obj->Get_Contained_By() != nullptr) {
        obj = nullptr;
    }

    if (obj != nullptr && obj->Is_KindOf(KINDOF_AIRCRAFT)) {
        obj = nullptr;
    }

    if (obj == nullptr) {
        m_targetObjectID = INVALID_OBJECT_ID;
    }

    return obj;
}

void Team::Set_Override_Team_Relationship(unsigned int id, Relationship relationship)
{
    if (id != 0) {
        m_teamRelations->m_relationships[id] = relationship;
    }
}

void Team::Set_Override_Player_Relationship(int id, Relationship relationship)
{
    if (id != -1) {
        m_playerRelations->m_relationships[id] = relationship;
    }
}

bool Team::Remove_Override_Player_Relationship(int id)
{
    if (m_playerRelations->m_relationships.empty()) {
        return false;
    }

    if (id == -1) {
        m_playerRelations->m_relationships.clear();
        return true;
    }

    auto it = m_playerRelations->m_relationships.find(id);

    if (!(it != m_playerRelations->m_relationships.end())) {
        return false;
    }

    m_playerRelations->m_relationships.erase(it);
    return true;
}

bool Team::Is_Idle() const
{
    DLINK_ITERATOR<Object> it = Iterate_Team_Member_List();

    while (!it.Done()) {
        AIUpdateInterface *update = it.Cur()->Get_AI_Update_Interface();

        if (update != nullptr) {
            if (!it.Cur()->Is_Effectively_Dead() && !update->Is_Idle()) {
                return false;
            }
        }

        it.Advance();
    }

    return true;
}

void Team::Notify_Team_Of_Object_Death()
{
    const TeamTemplateInfo *info = m_proto->Get_Template_Info();

    if (info != nullptr) {
        if (!info->m_scriptOnUnitDestroyed.Is_Empty()) {
            g_theScriptEngine->Run_Script(info->m_scriptOnUnitDestroyed, this);
        }
    }
}

bool Loco_Set_Matches(int lstm, unsigned int surface_bit_flags)
{
    return static_cast<unsigned char>((lstm & ((4 * (surface_bit_flags & 2)) | surface_bit_flags & 1))) != 0;
}

bool Team::Did_All_Enter(PolygonTrigger *poly, unsigned int surfaces) const
{
    if (!m_enteredOrExited) {
        return false;
    }

    bool can_move = false;
    bool did_enter = false;
    bool is_outside = false;
    DLINK_ITERATOR<Object> it = Iterate_Team_Member_List();

    while (!it.Done()) {
        AIUpdateInterface *update = it.Cur()->Get_AI_Update_Interface();

        if (update != nullptr) {
            if (!Loco_Set_Matches(update->Get_Locomotor_Set()->Get_Valid_Surfaces(), surfaces)) {
                it.Advance();
                continue;
            }
        } else if (!Loco_Set_Matches(1, surfaces)) {
            it.Advance();
            continue;
        }

        if (!it.Cur()->Is_Effectively_Dead()) {
            if (!it.Cur()->Is_KindOf(KINDOF_INERT)) {
                if (it.Cur()->Did_Enter(poly)) {
                    did_enter = true;
                } else {
                    if (!it.Cur()->Is_Inside(poly)) {
                        is_outside = true;
                    }
                }

                can_move = true;
            }
        }

        it.Advance();
    }

    return did_enter && !is_outside;
}

bool Team::Did_Partial_Enter(PolygonTrigger *poly, unsigned int surfaces) const
{
    if (!m_enteredOrExited) {
        return false;
    }

    DLINK_ITERATOR<Object> it = Iterate_Team_Member_List();

    while (!it.Done()) {
        AIUpdateInterface *update = it.Cur()->Get_AI_Update_Interface();

        if (update != nullptr) {
            if (!Loco_Set_Matches(update->Get_Locomotor_Set()->Get_Valid_Surfaces(), surfaces)) {
                it.Advance();
                continue;
            }
        } else if (!Loco_Set_Matches(1, surfaces)) {
            it.Advance();
            continue;
        }

        if (!it.Cur()->Is_Effectively_Dead()) {
            if (!it.Cur()->Is_KindOf(KINDOF_INERT)) {
                if (it.Cur()->Did_Enter(poly)) {
                    return true;
                }
            }
        }

        it.Advance();
    }

    return false;
}

bool Team::Did_Partial_Exit(PolygonTrigger *poly, unsigned int surfaces) const
{
    if (!m_enteredOrExited) {
        return false;
    }

    DLINK_ITERATOR<Object> it = Iterate_Team_Member_List();

    while (!it.Done()) {
        AIUpdateInterface *update = it.Cur()->Get_AI_Update_Interface();

        if (update != nullptr) {
            if (!Loco_Set_Matches(update->Get_Locomotor_Set()->Get_Valid_Surfaces(), surfaces)) {
                it.Advance();
                continue;
            }
        } else if (!Loco_Set_Matches(1, surfaces)) {
            it.Advance();
            continue;
        }

        if (!it.Cur()->Is_Effectively_Dead()) {
            if (!it.Cur()->Is_KindOf(KINDOF_INERT)) {
                if (it.Cur()->Did_Exit(poly)) {
                    return true;
                }
            }
        }

        it.Advance();
    }

    return false;
}

bool Team::Did_All_Exit(PolygonTrigger *poly, unsigned int surfaces) const
{
    if (!m_enteredOrExited) {
        return false;
    }

    bool can_move = false;
    bool did_exit = false;
    bool is_inside = false;
    DLINK_ITERATOR<Object> it = Iterate_Team_Member_List();

    while (!it.Done()) {
        AIUpdateInterface *update = it.Cur()->Get_AI_Update_Interface();

        if (update != nullptr) {
            if (!Loco_Set_Matches(update->Get_Locomotor_Set()->Get_Valid_Surfaces(), surfaces)) {
                it.Advance();
                continue;
            }
        } else if (!Loco_Set_Matches(1, surfaces)) {
            it.Advance();
            continue;
        }

        if (!it.Cur()->Is_Effectively_Dead()) {
            if (!it.Cur()->Is_KindOf(KINDOF_INERT)) {
                if (it.Cur()->Did_Exit(poly)) {
                    did_exit = true;
                } else {
                    if (it.Cur()->Is_Inside(poly)) {
                        is_inside = true;
                    }
                }

                can_move = true;
            }
        }

        it.Advance();
    }

    return can_move && did_exit && !is_inside;
}

bool Team::All_Inside(PolygonTrigger *poly, unsigned int surfaces) const
{
    if (!m_enteredOrExited) {
        return false;
    }

    bool can_move = false;
    bool is_outside = false;
    DLINK_ITERATOR<Object> it = Iterate_Team_Member_List();

    while (!it.Done()) {
        AIUpdateInterface *update = it.Cur()->Get_AI_Update_Interface();

        if (update != nullptr) {
            if (!Loco_Set_Matches(update->Get_Locomotor_Set()->Get_Valid_Surfaces(), surfaces)) {
                it.Advance();
                continue;
            }
        } else if (!Loco_Set_Matches(1, surfaces)) {
            it.Advance();
            continue;
        }

        if (!it.Cur()->Is_Effectively_Dead()) {
            if (!it.Cur()->Is_KindOf(KINDOF_INERT)) {
                if (!it.Cur()->Is_Inside(poly)) {
                    is_outside = true;
                }

                can_move = true;
            }
        }

        it.Advance();
    }

    return can_move && !is_outside;
}

bool Team::None_Inside(PolygonTrigger *poly, unsigned int surfaces) const
{
    if (!m_enteredOrExited) {
        return false;
    }

    bool can_move = false;
    bool is_inside = false;
    DLINK_ITERATOR<Object> it = Iterate_Team_Member_List();

    while (!it.Done()) {
        AIUpdateInterface *update = it.Cur()->Get_AI_Update_Interface();

        if (update != nullptr) {
            if (!Loco_Set_Matches(update->Get_Locomotor_Set()->Get_Valid_Surfaces(), surfaces)) {
                it.Advance();
                continue;
            }
        } else if (!Loco_Set_Matches(1, surfaces)) {
            it.Advance();
            continue;
        }

        if (!it.Cur()->Is_Effectively_Dead()) {
            if (!it.Cur()->Is_KindOf(KINDOF_INERT)) {
                if (it.Cur()->Is_Inside(poly)) {
                    is_inside = true;
                }

                can_move = true;
            }
        }

        it.Advance();
    }

    return can_move && !is_inside;
}

bool Team::Some_Inside_Some_Outside(PolygonTrigger *poly, unsigned int surfaces) const
{
    bool can_move = false;
    bool is_inside = false;
    bool is_outside = false;
    DLINK_ITERATOR<Object> it = Iterate_Team_Member_List();

    while (!it.Done()) {
        AIUpdateInterface *update = it.Cur()->Get_AI_Update_Interface();

        if (update != nullptr) {
            if (!Loco_Set_Matches(update->Get_Locomotor_Set()->Get_Valid_Surfaces(), surfaces)) {
                it.Advance();
                continue;
            }
        } else if (!Loco_Set_Matches(1, surfaces)) {
            it.Advance();
            continue;
        }

        if (!it.Cur()->Is_Effectively_Dead()) {
            if (!it.Cur()->Is_KindOf(KINDOF_INERT)) {
                if (it.Cur()->Is_Inside(poly)) {
                    is_inside = true;
                } else {
                    is_outside = true;
                }

                can_move = true;
            }
        }

        it.Advance();
    }

    return can_move && is_inside && is_outside;
}

const Coord3D *Team::Get_Estimate_Team_Position() const
{
    DLINK_ITERATOR<Object> it = Iterate_Team_Member_List();

    if (it.Cur() == nullptr) {
        return nullptr;
    }

    const Coord3D *pos = it.Cur()->Get_Position();

    if (pos == nullptr) {
        return nullptr;
    }

    return pos;
}

void Team::Delete_Team(bool ignore_dead)
{
    if (this == this->Get_Controlling_Player()->Get_Default_Team()) {
        std::list<Object *> list;
        DLINK_ITERATOR<Object> it = Iterate_Team_Member_List();

        while (!it.Done()) {
            Object *obj = it.Cur();

            if (obj != nullptr && obj->Get_Contain() != nullptr) {
                if (obj->Get_Contain()->Get_Contain_Count() != 0) {
                    list.push_back(obj);
                }
            }

            it.Advance();
        }

        for (auto it2 = list.begin(); it2 != list.end(); it2++) {
            if ((*it2)->Get_Contain() != nullptr) {
                (*it2)->Get_Contain()->Remove_All_Contained(false);
            }
        }
    }

    DLINK_ITERATOR<Object> it = Iterate_Team_Member_List();

    while (!it.Done()) {
        Object *obj = it.Cur();

        if (obj != nullptr && (!ignore_dead || !obj->Is_Effectively_Dead())) {
            g_theGameLogic->Destroy_Object(obj);
        }

        it.Advance();
    }
}

void Team::Transfer_Units_To(Team *team)
{
    if (this != team && team != nullptr) {
        for (;;) {
            Object *obj = Get_First_Item_In_Team_Member_List();

            if (obj == nullptr) {
                break;
            }

            obj->Set_Team(team);
        }
    }
}

bool Is_In_Build_Variations(const ThingTemplate *tmplate, const ThingTemplate *tmplate2)
{
    const std::vector<Utf8String> &variations = tmplate->Get_Build_Variations();

    if (variations.empty()) {
        return false;
    }

    for (auto it = variations.begin(); it != variations.end(); it++) {
        if (tmplate2->Get_Name() == (*it)) {
            return true;
        }
    }

    return false;
}

Object *Team::Try_To_Recruit(const ThingTemplate *tmplate, const Coord3D *pos, float max_dist)
{
    Player *player = Get_Controlling_Player();
    float dist_sqr = max_dist * max_dist;
    Object *recruit = nullptr;

    for (Object *obj = g_theGameLogic->Get_First_Object(); obj != nullptr; obj = obj->Get_Next_Object()) {
        if (!obj->Get_Template()->Is_Equivalent_To(tmplate)) {
            if (!Is_In_Build_Variations(tmplate, obj->Get_Template())) {
                continue;
            }
        }

        if (obj->Get_Controlling_Player() == player) {
            Team *team = obj->Get_Team();
            bool is_default = team == player->Get_Default_Team();

            if (team->Is_Active()) {
                if (team->Get_Prototype()->Get_Template_Info()->m_productionPriority
                    < Get_Prototype()->Get_Template_Info()->m_productionPriority) {
                    bool do_recruit = is_default;

                    if (team->Get_Prototype()->Get_Template_Info()->m_isAIRecruitable) {
                        do_recruit = true;
                    }

                    if (team->m_canRecruit) {
                        do_recruit = team->m_availableForRecruitment;
                    }

                    if (do_recruit) {
                        if (obj->Get_AI_Update_Interface() == nullptr || obj->Get_AI_Update_Interface()->Is_Recruitable()) {
                            if (!obj->Get_Disabled_State(DISABLED_TYPE_DISABLED_HELD)) {
                                float x = pos->x - obj->Get_Position()->x;
                                float y = pos->y - obj->Get_Position()->y;

                                if (is_default && recruit == nullptr) {
                                    recruit = obj;
                                    dist_sqr = x * x + y * y;
                                }

                                if (x * x + y * y <= dist_sqr) {
                                    recruit = obj;
                                    dist_sqr = x * x + y * y;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (recruit != nullptr) {
        return recruit;
    }

    return nullptr;
}

void Team::Evacuate_Team()
{
    std::list<Object *> list;
    DLINK_ITERATOR<Object> it = Iterate_Team_Member_List();

    while (!it.Done()) {
        Object *obj = it.Cur();

        if (obj != nullptr && !obj->Is_Destroyed() && !obj->Is_Effectively_Dead()) {
            ContainModuleInterface *contain = obj->Get_Contain();
            int count = 0;

            if (contain != nullptr) {
                count = contain->Get_Contain_Count();
            }

            if (count != 0) {
                list.push_back(obj);
            }
        }

        it.Advance();
    }

    for (auto it2 = list.begin(); it2 != list.end(); it2++) {
        if ((*it2)->Get_Contain() != nullptr) {
            (*it2)->Get_Contain()->Remove_All_Contained(false);
        }
    }

    list.clear();
}

void Team::Kill_Team()
{
    std::list<Object *> list;
    Evacuate_Team();
    Utf8String name = Get_Controlling_Player()->Get_Player_Template()->Get_Beacon_Name();
    ThingTemplate *tmplate = g_theThingFactory->Find_Template(name, true);
    DLINK_ITERATOR<Object> it = Iterate_Team_Member_List();

    while (!it.Done()) {
        Object *obj = it.Cur();

        if (obj != nullptr) {
            if (!obj->Is_Destroyed()) {
                if (!obj->Is_Effectively_Dead() || obj->Get_Template()->Is_Equivalent_To(tmplate)) {
                    if (obj->Get_Team() == this) {
                        list.push_back(obj);
                    }
                }
            }
        }

        it.Advance();
    }

    for (auto it2 = list.begin(); it2 != list.end(); it2++) {
        Object *obj = (*it2);

        if (obj->Is_KindOf(KINDOF_TECH_BUILDING)) {
            obj->Set_Team(g_thePlayerList->Get_Neutral_Player()->Get_Default_Team());
        } else {
            obj->Kill(DAMAGE_UNRESISTABLE, DEATH_NORMAL);
        }
    }

    list.clear();
}

void Team::Update_Generic_Scripts()
{
    for (int i = 0; i < ARRAY_SIZE(m_genericScriptActive); i++) {
        if (m_genericScriptActive[i]) {
            Script *script = m_proto->Get_Generic_Script(i);

            if (script != nullptr) {
                if (g_theScriptEngine->Evaluate_Conditions(script, this, nullptr)) {
                    if (script->Is_One_Shot()) {
                        m_genericScriptActive[i] = false;
                    }

                    g_theScriptEngine->Friend_Execute_Action(script->Get_Action(), this);
                    Utf8String str("Generic script '");
                    str += script->Get_Name();
                    str += "' run on team ";
                    str += Get_Name();
                    g_theScriptEngine->Append_Debug_Message(str, false);
                }
            } else {
                m_genericScriptActive[i] = false;
            }
        }
    }
}
