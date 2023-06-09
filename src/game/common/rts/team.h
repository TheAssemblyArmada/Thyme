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
#pragma once
#include "always.h"
#include "aiupdate.h"
#include "bitflags.h"
#include "coord.h"
#include "dict.h"
#include "gametype.h"
#include "mempoolobj.h"
#include "namekeygenerator.h"
#include "object.h"
#include "snapshot.h"
#include <list>
#include <map>

#ifdef THYME_USE_STLPORT
#include <hash_map>
#else
#include <unordered_map>
#endif

class TeamPrototype;
class SidesList;
class Team;
class Script;
class Player;
class Object;
class Waypoint;
class PlayerRelationMap;
class ThingTemplate;

template<class T> class DLINK_ITERATOR
{
public:
    typedef T *(T::*functype)() const;
    DLINK_ITERATOR(T *obj, functype func, int offset) : m_cur(obj), m_getNextFunc(func), m_offset(offset) {}

    void Advance()
    {
        if (m_cur != nullptr) {
            T *cur = reinterpret_cast<T *>(reinterpret_cast<char *>(m_cur) + m_offset);
            m_cur = (cur->*m_getNextFunc)();
        }
    }

    bool Done() const { return m_cur == nullptr; }
    T *Cur() const { return m_cur; }

private:
    T *m_cur;
    functype m_getNextFunc;
    int m_offset;
};

class TeamRelationMap : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_NAMED_POOL(TeamRelationMap, TeamRelationMapPool);

public:
    TeamRelationMap() {}

    virtual ~TeamRelationMap() override;
    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

#ifdef GAME_DLL
    TeamRelationMap *Hook_Ctor() { return new (this) TeamRelationMap(); }
#endif

private:
#ifdef THYME_USE_STLPORT
    std::hash_map<unsigned int, Relationship> m_relationships;
#else
    std::unordered_map<unsigned int, Relationship> m_relationships;
#endif
    friend class Team;
    friend class Player;
};

class TeamFactory : public SubsystemInterface, public SnapShot
{
public:
    TeamFactory();
    virtual ~TeamFactory() override;
    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override {}

    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    void Clear();
    void Init_From_Sides(SidesList *sides);
    void Init_Team(const Utf8String &name, const Utf8String &owner, bool is_singleton, Dict *d);
    void Add_Team_Prototype_To_List(TeamPrototype *team);
    void Remove_Team_Prototype_From_List(TeamPrototype *team);
    TeamPrototype *Find_Team_Prototype(const Utf8String &name);
    TeamPrototype *Find_Team_Prototype_By_ID(unsigned int id);
    Team *Find_Team_By_ID(unsigned int id);
    Team *Create_Inactive_Team(const Utf8String &name);
    Team *Create_Team(const Utf8String &name);
    Team *Create_Team_On_Prototype(TeamPrototype *proto);
    Team *Find_Team(const Utf8String &name);
    void Team_About_To_Be_Deleted(Team *team);

#ifdef GAME_DLL
    TeamFactory *Hook_Ctor() { return new (this) TeamFactory(); }
#endif

private:
    std::map<NameKeyType, TeamPrototype *> m_prototypes;
    unsigned int m_nextPrototypeID;
    unsigned int m_nextTeamID;
};

struct TCreateUnitsInfo
{
    int min_units;
    int max_units;
    Utf8String unit_thing_name;
};

class TeamTemplateInfo : public SnapShot
{
public:
    enum
    {
        MAX_UNIT_TYPES = 7,
    };

    TeamTemplateInfo(Dict *d);
    ~TeamTemplateInfo() {}
    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

private:
    TCreateUnitsInfo m_unitsInfo[MAX_UNIT_TYPES];
    int m_numUnitsInfo;
    Coord3D m_homeLocation;
    char m_hasHomeLocation;
    Utf8String m_scriptOnCreate;
    Utf8String m_scriptOnIdle;
    int m_initialIdleFrames;
    Utf8String m_scriptOnEnemySighted;
    Utf8String m_scriptOnAllClear;
    Utf8String m_scriptOnUnitDestroyed;
    Utf8String m_scriptOnDestroyed;
    float m_destroyedThreshold;
    bool m_isAIRecruitable;
    bool m_isBaseDefense;
    bool m_isPerimeterDefense;
    bool m_automaticallyReinforce;
    bool m_transportsReturn;
    bool m_avoidThreats;
    bool m_attackCommonTarget;
    int m_maxInstances;
    int m_productionPriority;
    int m_productionPrioritySuccessIncrease;
    int m_productionPriorityFailureDecrease;
    AttitudeType m_initialTeamAttitude;
    Utf8String m_transportUnitType;
    Utf8String m_startReinforceWaypoint;
    bool m_teamStartsFull;
    bool m_transportsExit;
    int m_veterancy;
    Utf8String m_productionCondition;
    char m_executesActionsOnCreate;
    Utf8String m_genericScripts[16];
    friend class TeamFactory;
    friend class Team;
    friend class TeamPrototype;
    friend class Object;
    friend class ScriptEngine;
};

class Team : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_NAMED_POOL(Team, TeamPool);

public:
    Team(TeamPrototype *proto, unsigned int id);
    virtual ~Team() override;
    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    Player *Get_Controlling_Player() const;
    void Get_Team_As_AI_Group(AIGroup *aigroup);
    int Get_Targetable_Count() const;
    Relationship Get_Relationship(const Team *that) const;
    Object *Get_Team_Target_Object();
    const Coord3D *Get_Estimate_Team_Position() const;
    void Delete_Team(bool ignore_dead);
    void Transfer_Units_To(Team *team);
    Object *Try_To_Recruit(const ThingTemplate *tmplate, const Coord3D *pos, float distance);
    void Evacuate_Team();
    void Kill_Team();
    bool Damage_Team_Members(float amount);
    void Move_Team_To(Coord3D destination);
    bool Has_Any_Build_Facility() const;
    void Update_Generic_Scripts();

    void Set_Controlling_Player(Player *new_controller);
    void Set_Attack_Priority_Name(Utf8String name);
    void Set_Team_Target_Object(const Object *obj);
    void Set_Override_Team_Relationship(unsigned int id, Relationship relationship);
    void Set_Override_Player_Relationship(int id, Relationship relationship);

    bool Has_Any_Buildings() const;
    bool Has_Any_Buildings(BitFlags<KINDOF_COUNT> must_be_set) const;
    bool Has_Any_Units() const;
    bool Has_Any_Objects() const;

    bool Remove_Override_Team_Relationship(unsigned int id);
    bool Remove_Override_Player_Relationship(int id);
    void Heal_All_Objects();
    void Iterate_Objects(void (*func)(Object *, void *), void *data);
    bool Is_Idle() const;
    void Update_State();
    void Notify_Team_Of_Object_Death();
    bool Did_All_Enter(PolygonTrigger *poly, unsigned int surfaces) const;
    bool Did_Partial_Enter(PolygonTrigger *poly, unsigned int surfaces) const;
    bool Did_Partial_Exit(PolygonTrigger *poly, unsigned int surfaces) const;
    bool Did_All_Exit(PolygonTrigger *poly, unsigned int surfaces) const;
    bool All_Inside(PolygonTrigger *poly, unsigned int surfaces) const;
    bool None_Inside(PolygonTrigger *poly, unsigned int surfaces) const;
    bool Some_Inside_Some_Outside(PolygonTrigger *poly, unsigned int surfaces) const;

    void Count_Objects_By_Thing_Template(int num_tmplates,
        const ThingTemplate *const *things,
        bool ignore_dead,
        int *counts,
        bool ignore_under_construction) const;
    int Count_Buildings();
    int Count_Objects(BitFlags<KINDOF_COUNT> must_be_set, BitFlags<KINDOF_COUNT> must_be_clear);

    Team *Dlink_Next_Team_Instance_List() const { return m_dlink_TeamInstanceList.m_next; }

    void DLink_Remove_From_Team_Instance_List(Team **head)
    {
        captainslog_dbgassert(DLink_Is_In_List_Team_Instance_List(head), "not in listTeamInstanceList");
        captainslog_dbgassert(((uintptr_t)head & 1) == 0 && ((uintptr_t)m_dlink_TeamInstanceList.m_next & 1) == 0
                && ((uintptr_t)m_dlink_TeamInstanceList.m_prev & 1) == 0,
            "bogus ptrs");

        if (m_dlink_TeamInstanceList.m_next != nullptr) {
            m_dlink_TeamInstanceList.m_next->m_dlink_TeamInstanceList.m_prev = m_dlink_TeamInstanceList.m_prev;
        }

        if (m_dlink_TeamInstanceList.m_prev != nullptr) {
            m_dlink_TeamInstanceList.m_prev->m_dlink_TeamInstanceList.m_next = m_dlink_TeamInstanceList.m_next;
        } else {
            *head = m_dlink_TeamInstanceList.m_next;
        }

        m_dlink_TeamInstanceList.m_prev = nullptr;
        m_dlink_TeamInstanceList.m_next = nullptr;
        captainslog_dbgassert(((uintptr_t)head & 1) == 0 && ((uintptr_t)m_dlink_TeamInstanceList.m_next & 1) == 0
                && ((uintptr_t)m_dlink_TeamInstanceList.m_prev & 1) == 0,
            "bogus ptrs");
    }

    void DLink_Prepend_To_Team_Instance_List(Team **head)
    {
        captainslog_dbgassert(!DLink_Is_In_List_Team_Instance_List(head), "already in listTeamInstanceList");
        captainslog_dbgassert(((uintptr_t)head & 1) == 0 && ((uintptr_t)m_dlink_TeamInstanceList.m_next & 1) == 0
                && ((uintptr_t)m_dlink_TeamInstanceList.m_prev & 1) == 0,
            "bogus ptrs");
        m_dlink_TeamInstanceList.m_next = *head;

        if (*head != nullptr) {
            (*head)->m_dlink_TeamInstanceList.m_prev = this;
        }

        *head = this;
        captainslog_dbgassert(((uintptr_t)head & 1) == 0 && ((uintptr_t)m_dlink_TeamInstanceList.m_next & 1) == 0
                && ((uintptr_t)m_dlink_TeamInstanceList.m_prev & 1) == 0,
            "bogus ptrs");
    }

    bool DLink_Is_In_List_Team_Instance_List(Team *const *head) const
    {
        captainslog_dbgassert(((uintptr_t)head & 1) == 0 && ((uintptr_t)m_dlink_TeamInstanceList.m_next & 1) == 0
                && ((uintptr_t)m_dlink_TeamInstanceList.m_prev & 1) == 0,
            "bogus ptrs");
        return *head == this || m_dlink_TeamInstanceList.m_prev != nullptr || m_dlink_TeamInstanceList.m_next != nullptr;
    }

    bool Is_In_List_Team_Member_List(Object *object) const
    {
        captainslog_dbgassert(((uintptr_t)m_dlinkhead_TeamMemberList.m_head & 1) == 0, "bogus head ptr");
        return object->DLink_Is_In_List_Team_Member_List(&m_dlinkhead_TeamMemberList.m_head);
    }

    void Prepend_To_Team_Member_List(Object *object)
    {
        captainslog_dbgassert(((uintptr_t)m_dlinkhead_TeamMemberList.m_head & 1) == 0, "bogus head ptr");
        if (!Is_In_List_Team_Member_List(object)) {
            object->DLink_Prepend_To_Team_Member_List(&m_dlinkhead_TeamMemberList.m_head);
        }
    }

    void Remove_From_Team_Member_List(Object *object)
    {
        captainslog_dbgassert(((uintptr_t)m_dlinkhead_TeamMemberList.m_head & 1) == 0, "bogus head ptr");
        if (Is_In_List_Team_Member_List(object)) {
            object->DLink_Remove_From_Team_Member_List(&m_dlinkhead_TeamMemberList.m_head);
        }
    }

    DLINK_ITERATOR<Object> Iterate_Team_Member_List() const
    {
        captainslog_dbgassert(((uintptr_t)m_dlinkhead_TeamMemberList.m_head & 1) == 0, "bogus head ptr");
        return DLINK_ITERATOR<Object>(m_dlinkhead_TeamMemberList.m_head, &Object::Dlink_Next_Team_Member_List, 0);
    }

    Object *Get_First_Item_In_Team_Member_List() const
    {
        captainslog_dbgassert(((uintptr_t)m_dlinkhead_TeamMemberList.m_head & 1) == 0, "bogus head ptr");
        return m_dlinkhead_TeamMemberList.m_head;
    }

    bool Is_Active() const { return m_active; }
    const Utf8String &Get_Name() const;
    unsigned int Get_Team_ID() const { return m_teamID; }
    const TeamPrototype *Get_Prototype() const { return m_proto; }
    const Waypoint *Get_Current_Waypoint() { return m_currentWaypoint; }
    bool Did_Enter_Or_Exit() { return m_enteredOrExited; }
    Utf8String &Get_State() { return m_state; }
    bool Is_Created() const { return m_created; }

    void Set_Entered_Exited() { m_enteredOrExited = true; }
    void Set_Current_Waypoint(Waypoint *waypoint) { m_currentWaypoint = waypoint; }
    void Set_ID(int id) { m_teamID = id; }
    void Set_State(Utf8String &state) { m_state = state; }

    void Set_Available_For_Recruitment(bool available)
    {
        m_canRecruit = true;
        m_availableForRecruitment = available;
    }

    void Set_Active()
    {
        if (!m_active) {
            m_created = true;
            m_active = true;
        }
    }

private:
    struct DLINKHEAD_TeamMemberList
    {
        DLINKHEAD_TeamMemberList() : m_head(nullptr) {}
        ~DLINKHEAD_TeamMemberList()
        {
            captainslog_dbgassert(m_head == nullptr, "destroying dlinkhead still in a list TeamMemberList");
        }
        Object *m_head;
    };

    struct DLINK_TeamInstanceList
    {
        DLINK_TeamInstanceList() : m_prev(nullptr), m_next(nullptr) {}
        ~DLINK_TeamInstanceList()
        {
            captainslog_dbgassert(
                m_prev == nullptr && m_next == nullptr, "destroying dlink still in a list TeamInstanceList");
        }
        Team *m_prev;
        Team *m_next;
    };

    TeamPrototype *m_proto;
    unsigned int m_teamID;
    DLINKHEAD_TeamMemberList m_dlinkhead_TeamMemberList;
    DLINK_TeamInstanceList m_dlink_TeamInstanceList;
    Utf8String m_state;
    bool m_enteredOrExited;
    bool m_active;
    bool m_created;
    bool m_checkEnemySighted;
    bool m_seeEnemy;
    bool m_prevSeeEnemy;
    bool m_wasIdle;
    int m_destroyThreshold;
    int m_curUnits;
    Waypoint *m_currentWaypoint;
    bool m_genericScriptActive[16];
    bool m_canRecruit;
    bool m_availableForRecruitment;
    ObjectID m_targetObjectID;
    TeamRelationMap *m_teamRelations;
    PlayerRelationMap *m_playerRelations;
    std::list<ObjectID> m_objectIDList;
};

class TeamPrototype : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_NAMED_POOL(TeamPrototype, TeamPrototypePool);

public:
    TeamPrototype(
        TeamFactory *tf, const Utf8String &name, Player *owner_player, bool is_singleton, Dict *d, unsigned int id);
    virtual ~TeamPrototype() override;
    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    Player *Get_Controlling_Player() const;
    Team *Find_Team_By_ID(unsigned int id);
    void Set_Controlling_Player(Player *new_controller);
    void Count_Objects_By_Thing_Template(int num_tmplates,
        const ThingTemplate *const *things,
        bool ignore_dead,
        int *counts,
        bool ignore_under_construction) const;
    void Team_About_To_Be_Deleted(Team *team);
    Script *Get_Generic_Script(int script);
    void Increase_AI_Priority_For_Success();
    void Decrease_AI_Priority_For_Failure();
    int Count_Buildings();
    int Count_Objects(BitFlags<KINDOF_COUNT> must_be_set, BitFlags<KINDOF_COUNT> must_be_clear);
    void Heal_All_Objects();
    void Iterate_Objects(void (*func)(Object *, void *), void *data);
    int Count_Team_Instances();
    bool Has_Any_Buildings() const;
    bool Has_Any_Buildings(BitFlags<KINDOF_COUNT> must_be_set) const;
    bool Has_Any_Units() const;
    bool Has_Any_Objects() const;
    void Update_State();
    bool Has_Any_Build_Facility() const;
    void Damage_Team_Members(float amount);
    void Move_Team_To(Coord3D destination);
    bool Evaluate_Production_Condition();

    Team *Get_First_Item_In_Team_Instance_List() const
    {
        captainslog_dbgassert(((uintptr_t)m_dlinkhead_TeamInstanceList.m_head & 1) == 0, "bogus head ptr");
        return m_dlinkhead_TeamInstanceList.m_head;
    }

    DLINK_ITERATOR<Team> Iterate_Team_Instance_List() const
    {
        captainslog_dbgassert(((uintptr_t)m_dlinkhead_TeamInstanceList.m_head & 1) == 0, "bogus head ptr");
        return DLINK_ITERATOR<Team>(m_dlinkhead_TeamInstanceList.m_head, &Team::Dlink_Next_Team_Instance_List, 0);
    }

    void Remove_All_Team_Instance_List(void (*func)(Team *))
    {
        while (m_dlinkhead_TeamInstanceList.m_head != nullptr) {
            captainslog_dbgassert(((uintptr_t)m_dlinkhead_TeamInstanceList.m_head & 1) == 0, "bogus head ptr");
            Team *team = m_dlinkhead_TeamInstanceList.m_head;
            Remove_From_Team_Instance_List(team);

            if (func != nullptr) {
                func(team);
            }
        }
    }

    bool Is_In_List_Team_Instance_List(Team *team) const
    {
        captainslog_dbgassert(((uintptr_t)m_dlinkhead_TeamInstanceList.m_head & 1) == 0, "bogus head ptr");
        return team->DLink_Is_In_List_Team_Instance_List(&m_dlinkhead_TeamInstanceList.m_head);
    }

    void Prepend_To_Team_Instance_List(Team *team)
    {
        captainslog_dbgassert(((uintptr_t)m_dlinkhead_TeamInstanceList.m_head & 1) == 0, "bogus head ptr");
        if (!Is_In_List_Team_Instance_List(team)) {
            team->DLink_Prepend_To_Team_Instance_List(&m_dlinkhead_TeamInstanceList.m_head);
        }
    }

    void Remove_From_Team_Instance_List(Team *team)
    {
        captainslog_dbgassert(((uintptr_t)m_dlinkhead_TeamInstanceList.m_head & 1) == 0, "bogus head ptr");
        if (Is_In_List_Team_Instance_List(team)) {
            team->DLink_Remove_From_Team_Instance_List(&m_dlinkhead_TeamInstanceList.m_head);
        }
    }

    bool Get_Singleton() const { return (m_flags & TEAM_SINGLETON) != 0; }
    const Utf8String &Get_Name() const { return m_name; }
    const TeamTemplateInfo *Get_Template_Info() const { return &m_teamTemplate; }
    Utf8String Get_Attack_Priority_Name() const { return m_attackPriorityName; }
    unsigned int Get_ID() const { return m_ID; }
    void Set_Attack_Priority_Name(Utf8String &name) { m_attackPriorityName = name; }
    void Set_Owning_Player(Player *player) { m_owningPlayer = player; }

private:
    enum TeamPrototypeFlags
    {
        TEAM_SINGLETON = 1,
    };

    struct DLINKHEAD_TeamInstanceList
    {
        DLINKHEAD_TeamInstanceList() : m_head(nullptr) {}
        ~DLINKHEAD_TeamInstanceList()
        {
            captainslog_dbgassert(m_head == nullptr, "destroying dlinkhead still in a list TeamInstanceList");
        }
        Team *m_head;
    };

    TeamFactory *m_factory;
    Player *m_owningPlayer;
    unsigned int m_ID;
    Utf8String m_name;
    int m_flags;
    bool m_productionConditionAlwaysFalse;
    Script *m_productionConditionScript;
    bool m_genericScriptsInitalized;
    Script *m_genericScripts[16];
    TeamTemplateInfo m_teamTemplate;
    Utf8String m_attackPriorityName;
    DLINKHEAD_TeamInstanceList m_dlinkhead_TeamInstanceList;
};

inline const Utf8String &Team::Get_Name() const
{
    return m_proto->Get_Name();
}

#ifdef GAME_DLL
extern TeamFactory *&g_theTeamFactory;
#else
extern TeamFactory *g_theTeamFactory;
#endif
