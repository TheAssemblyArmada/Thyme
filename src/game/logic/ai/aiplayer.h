/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief AI Player
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
#include "mempoolobj.h"
#include "player.h"
#include "snapshot.h"
#include "team.h"

class WorkOrder;
class Player;

enum SkillSetSelection
{
    INVALID_SKILLSET_SELECTION = -1,
};

class TeamInQueue : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_POOL(TeamInQueue)

public:
    virtual ~TeamInQueue() override;
    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    TeamInQueue *Dlink_Next_Team_Build_Queue() const { return m_dlink_TeamBuildQueue.m_next; }

    void DLink_Remove_From_Team_Build_Queue(TeamInQueue **head)
    {
        captainslog_dbgassert(DLink_Is_In_List_Team_Build_Queue(head), "not in listTeamBuildQueue");
        captainslog_dbgassert(((uintptr_t)head & 1) == 0 && ((uintptr_t)m_dlink_TeamBuildQueue.m_next & 1) == 0
                && ((uintptr_t)m_dlink_TeamBuildQueue.m_prev & 1) == 0,
            "bogus ptrs");

        if (m_dlink_TeamBuildQueue.m_next != nullptr) {
            m_dlink_TeamBuildQueue.m_next->m_dlink_TeamBuildQueue.m_prev = m_dlink_TeamBuildQueue.m_prev;
        }

        if (m_dlink_TeamBuildQueue.m_prev != nullptr) {
            m_dlink_TeamBuildQueue.m_prev->m_dlink_TeamBuildQueue.m_next = m_dlink_TeamBuildQueue.m_next;
        } else {
            *head = m_dlink_TeamBuildQueue.m_next;
        }

        m_dlink_TeamBuildQueue.m_prev = nullptr;
        m_dlink_TeamBuildQueue.m_next = nullptr;
        captainslog_dbgassert(((uintptr_t)head & 1) == 0 && ((uintptr_t)m_dlink_TeamBuildQueue.m_next & 1) == 0
                && ((uintptr_t)m_dlink_TeamBuildQueue.m_prev & 1) == 0,
            "bogus ptrs");
    }

    void DLink_Prepend_To_Team_Build_Queue(TeamInQueue **head)
    {
        captainslog_dbgassert(!DLink_Is_In_List_Team_Build_Queue(head), "already in listTeamBuildQueue");
        captainslog_dbgassert(((uintptr_t)head & 1) == 0 && ((uintptr_t)m_dlink_TeamBuildQueue.m_next & 1) == 0
                && ((uintptr_t)m_dlink_TeamBuildQueue.m_prev & 1) == 0,
            "bogus ptrs");
        m_dlink_TeamBuildQueue.m_next = *head;

        if (*head != nullptr) {
            (*head)->m_dlink_TeamBuildQueue.m_prev = this;
        }

        *head = this;
        captainslog_dbgassert(((uintptr_t)head & 1) == 0 && ((uintptr_t)m_dlink_TeamBuildQueue.m_next & 1) == 0
                && ((uintptr_t)m_dlink_TeamBuildQueue.m_prev & 1) == 0,
            "bogus ptrs");
    }

    bool DLink_Is_In_List_Team_Build_Queue(TeamInQueue *const *head) const
    {
        captainslog_dbgassert(((uintptr_t)head & 1) == 0 && ((uintptr_t)m_dlink_TeamBuildQueue.m_next & 1) == 0
                && ((uintptr_t)m_dlink_TeamBuildQueue.m_prev & 1) == 0,
            "bogus ptrs");
        return *head == this || m_dlink_TeamBuildQueue.m_prev != nullptr || m_dlink_TeamBuildQueue.m_next != nullptr;
    }

    void Dlink_Swap_Links_Team_Build_Queue()
    {
        TeamInQueue *next = m_dlink_TeamBuildQueue.m_next;
        m_dlink_TeamBuildQueue.m_next = m_dlink_TeamBuildQueue.m_prev;
        m_dlink_TeamBuildQueue.m_prev = next;
    }

    TeamInQueue *Dlink_Next_Team_Ready_Queue() const { return m_dlink_TeamReadyQueue.m_next; }

    void DLink_Remove_From_Team_Ready_Queue(TeamInQueue **head)
    {
        captainslog_dbgassert(DLink_Is_In_List_Team_Ready_Queue(head), "not in listTeamReadyQueue");
        captainslog_dbgassert(((uintptr_t)head & 1) == 0 && ((uintptr_t)m_dlink_TeamReadyQueue.m_next & 1) == 0
                && ((uintptr_t)m_dlink_TeamReadyQueue.m_prev & 1) == 0,
            "bogus ptrs");

        if (m_dlink_TeamReadyQueue.m_next != nullptr) {
            m_dlink_TeamReadyQueue.m_next->m_dlink_TeamReadyQueue.m_prev = m_dlink_TeamReadyQueue.m_prev;
        }

        if (m_dlink_TeamReadyQueue.m_prev != nullptr) {
            m_dlink_TeamReadyQueue.m_prev->m_dlink_TeamReadyQueue.m_next = m_dlink_TeamReadyQueue.m_next;
        } else {
            *head = m_dlink_TeamReadyQueue.m_next;
        }

        m_dlink_TeamReadyQueue.m_prev = nullptr;
        m_dlink_TeamReadyQueue.m_next = nullptr;
        captainslog_dbgassert(((uintptr_t)head & 1) == 0 && ((uintptr_t)m_dlink_TeamReadyQueue.m_next & 1) == 0
                && ((uintptr_t)m_dlink_TeamReadyQueue.m_prev & 1) == 0,
            "bogus ptrs");
    }

    void DLink_Prepend_To_Team_Ready_Queue(TeamInQueue **head)
    {
        captainslog_dbgassert(!DLink_Is_In_List_Team_Ready_Queue(head), "already in listTeamReadyQueue");
        captainslog_dbgassert(((uintptr_t)head & 1) == 0 && ((uintptr_t)m_dlink_TeamReadyQueue.m_next & 1) == 0
                && ((uintptr_t)m_dlink_TeamReadyQueue.m_prev & 1) == 0,
            "bogus ptrs");
        m_dlink_TeamReadyQueue.m_next = *head;

        if (*head != nullptr) {
            (*head)->m_dlink_TeamReadyQueue.m_prev = this;
        }

        *head = this;
        captainslog_dbgassert(((uintptr_t)head & 1) == 0 && ((uintptr_t)m_dlink_TeamReadyQueue.m_next & 1) == 0
                && ((uintptr_t)m_dlink_TeamReadyQueue.m_prev & 1) == 0,
            "bogus ptrs");
    }

    bool DLink_Is_In_List_Team_Ready_Queue(TeamInQueue *const *head) const
    {
        captainslog_dbgassert(((uintptr_t)head & 1) == 0 && ((uintptr_t)m_dlink_TeamReadyQueue.m_next & 1) == 0
                && ((uintptr_t)m_dlink_TeamReadyQueue.m_prev & 1) == 0,
            "bogus ptrs");
        return *head == this || m_dlink_TeamReadyQueue.m_prev != nullptr || m_dlink_TeamReadyQueue.m_next != nullptr;
    }

    void Dlink_Swap_Links_Team_Ready_Queue()
    {
        TeamInQueue *next = m_dlink_TeamReadyQueue.m_next;
        m_dlink_TeamReadyQueue.m_next = m_dlink_TeamReadyQueue.m_prev;
        m_dlink_TeamReadyQueue.m_prev = next;
    }

private:
    struct DLINK_TeamBuildQueue
    {
        TeamInQueue *m_prev;
        TeamInQueue *m_next;
        DLINK_TeamBuildQueue() : m_prev(nullptr), m_next(nullptr) {}
        ~DLINK_TeamBuildQueue()
        {
            captainslog_dbgassert(m_prev == nullptr && m_next == nullptr, "destroying dlink still in a list TeamBuildQueue");
        }
    };

    struct DLINK_TeamReadyQueue
    {
        TeamInQueue *m_prev;
        TeamInQueue *m_next;
        DLINK_TeamReadyQueue() : m_prev(nullptr), m_next(nullptr) {}
        ~DLINK_TeamReadyQueue()
        {
            captainslog_dbgassert(m_prev == nullptr && m_next == nullptr, "destroying dlink still in a list TeamReadyQueue");
        }
    };

    DLINK_TeamBuildQueue m_dlink_TeamBuildQueue;
    DLINK_TeamReadyQueue m_dlink_TeamReadyQueue;
    WorkOrder *m_workorders;
    bool m_priorityBuild;
    Team *m_team;
    TeamInQueue *m_nextTeamInQueue;
    int m_frameStarted;
    bool m_sentToStartLocation;
    bool m_stopQueueing;
    bool m_reinforcement;
    ObjectID m_reinforcementID;
};

class AIPlayer : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_POOL(AIPlayer)

public:
    AIPlayer(Player *p);
    virtual ~AIPlayer() override;
    virtual bool Compute_Superweapon_Target(
        const SpecialPowerTemplate *special_power, Coord3D *pos, int player_index, float f);
    virtual void Update();
    virtual void New_Map();
    virtual void On_Unit_Produced(Object *factory, Object *unit);
    virtual void On_Structure_Produced(Object *factory, Object *bldg);
    virtual void Build_Specific_AI_Team(TeamPrototype *team_proto, bool priority_build);
    virtual void Build_AI_Base_Defense(bool b);
    virtual void Build_AI_Base_Defense_Structure(const Utf8String &name, bool b);
    virtual void Build_Specific_AI_Building(const Utf8String &name);
    virtual void Recruit_Specific_AI_Team(TeamPrototype *team_proto, float recruit_radius);
    virtual bool Is_Skirmish_AI() { return false; }
    virtual Player *Get_Ai_Enemy() { return nullptr; }
    virtual bool Check_Bridges(Object *obj, Waypoint *waypoint) { return false; }
    virtual void Repair_Structure(ObjectID id);
    virtual void Select_Skillset(int skillset);
    virtual void Do_Base_Building();
    virtual void Check_Ready_Teams();
    virtual void Check_Queued_Teams();
    virtual void Do_Team_Building();
    virtual void Do_Upgrades_And_Skills();
    virtual Object *Find_Dozer(const Coord3D *pos);
    virtual void Queue_Dozer();
    virtual bool Select_Team_To_Build();
    virtual bool Select_Team_To_Reinforce(int min_priority);
    virtual bool Start_Training(WorkOrder *order, bool busy_ok, Utf8String team_name);
    virtual bool Is_A_Good_Idea_To_Build_Team(TeamPrototype *team_proto);
    virtual void Process_Base_Building();
    virtual void Process_Team_Building();

    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    void Set_Team_Delay_Seconds(int seconds) { m_teamDelaySeconds = seconds; }

    void Clear_Teams_In_Queue();
    bool Is_Supply_Source_Safe(int source);
    bool Is_Supply_Source_Attacked();
    void AI_Pre_Team_Destroy(const Team *team);
    void Guard_Supply_Center(Team *team, int center);
    void Build_By_Supplies(int supplies, const Utf8String &name);
    void Build_Specific_Building_Nearest_Team(const Utf8String &name, const Team *team);
    void Build_Upgrade(const Utf8String &name);
    bool Calc_Closest_Construction_Zone_Location(const ThingTemplate *tmplate, Coord3D *pos);

    TeamInQueue *Get_First_Item_In_Team_Build_Queue() const
    {
        captainslog_dbgassert(((uintptr_t)m_dlinkhead_TeamBuildQueue.m_head & 1) == 0, "bogus head ptr");
        return m_dlinkhead_TeamBuildQueue.m_head;
    }

    DLINK_ITERATOR<TeamInQueue> Iterate_Team_Build_Queue() const
    {
        captainslog_dbgassert(((uintptr_t)m_dlinkhead_TeamBuildQueue.m_head & 1) == 0, "bogus head ptr");
        return DLINK_ITERATOR<TeamInQueue>(m_dlinkhead_TeamBuildQueue.m_head, &TeamInQueue::Dlink_Next_Team_Build_Queue, 0);
    }

    void Remove_All_Team_Build_Queue(void (*func)(TeamInQueue *))
    {
        while (m_dlinkhead_TeamBuildQueue.m_head != nullptr) {
            captainslog_dbgassert(((uintptr_t)m_dlinkhead_TeamBuildQueue.m_head & 1) == 0, "bogus head ptr");
            TeamInQueue *team = m_dlinkhead_TeamBuildQueue.m_head;
            Remove_From_Team_Build_Queue(team);

            if (func != nullptr) {
                func(team);
            }
        }
    }

    bool Is_In_List_Team_Build_Queue(TeamInQueue *team) const
    {
        captainslog_dbgassert(((uintptr_t)m_dlinkhead_TeamBuildQueue.m_head & 1) == 0, "bogus head ptr");
        return team->DLink_Is_In_List_Team_Build_Queue(&m_dlinkhead_TeamBuildQueue.m_head);
    }

    void Prepend_To_Team_Build_Queue(TeamInQueue *team)
    {
        captainslog_dbgassert(((uintptr_t)m_dlinkhead_TeamBuildQueue.m_head & 1) == 0, "bogus head ptr");
        if (!Is_In_List_Team_Build_Queue(team)) {
            team->DLink_Prepend_To_Team_Build_Queue(&m_dlinkhead_TeamBuildQueue.m_head);
        }
    }

    void Remove_From_Team_Build_Queue(TeamInQueue *team)
    {
        captainslog_dbgassert(((uintptr_t)m_dlinkhead_TeamBuildQueue.m_head & 1) == 0, "bogus head ptr");
        if (Is_In_List_Team_Build_Queue(team)) {
            team->DLink_Remove_From_Team_Build_Queue(&m_dlinkhead_TeamBuildQueue.m_head);
        }
    }

    void Reverse_Team_Build_Queue()
    {
        TeamInQueue *head = m_dlinkhead_TeamBuildQueue.m_head;
        TeamInQueue *tail = nullptr;

        while (head != nullptr) {
            TeamInQueue *next = head->Dlink_Next_Team_Build_Queue();
            head->Dlink_Swap_Links_Team_Build_Queue();
            tail = head;
            head = next;
        }

        m_dlinkhead_TeamBuildQueue.m_head = tail;
    }

    TeamInQueue *Get_First_Item_In_Team_Ready_Queue() const
    {
        captainslog_dbgassert(((uintptr_t)m_dlinkhead_TeamReadyQueue.m_head & 1) == 0, "bogus head ptr");
        return m_dlinkhead_TeamReadyQueue.m_head;
    }

    DLINK_ITERATOR<TeamInQueue> Iterate_Team_Ready_Queue() const
    {
        captainslog_dbgassert(((uintptr_t)m_dlinkhead_TeamReadyQueue.m_head & 1) == 0, "bogus head ptr");
        return DLINK_ITERATOR<TeamInQueue>(m_dlinkhead_TeamReadyQueue.m_head, &TeamInQueue::Dlink_Next_Team_Ready_Queue, 0);
    }

    void Remove_All_Team_Ready_Queue(void (*func)(TeamInQueue *))
    {
        while (m_dlinkhead_TeamReadyQueue.m_head != nullptr) {
            captainslog_dbgassert(((uintptr_t)m_dlinkhead_TeamReadyQueue.m_head & 1) == 0, "bogus head ptr");
            TeamInQueue *team = m_dlinkhead_TeamReadyQueue.m_head;
            Remove_From_Team_Ready_Queue(team);

            if (func != nullptr) {
                func(team);
            }
        }
    }

    bool Is_In_List_Team_Ready_Queue(TeamInQueue *team) const
    {
        captainslog_dbgassert(((uintptr_t)m_dlinkhead_TeamReadyQueue.m_head & 1) == 0, "bogus head ptr");
        return team->DLink_Is_In_List_Team_Ready_Queue(&m_dlinkhead_TeamReadyQueue.m_head);
    }

    void Prepend_To_Team_Ready_Queue(TeamInQueue *team)
    {
        captainslog_dbgassert(((uintptr_t)m_dlinkhead_TeamReadyQueue.m_head & 1) == 0, "bogus head ptr");
        if (!Is_In_List_Team_Ready_Queue(team)) {
            team->DLink_Prepend_To_Team_Ready_Queue(&m_dlinkhead_TeamReadyQueue.m_head);
        }
    }

    void Remove_From_Team_Ready_Queue(TeamInQueue *team)
    {
        captainslog_dbgassert(((uintptr_t)m_dlinkhead_TeamReadyQueue.m_head & 1) == 0, "bogus head ptr");
        if (Is_In_List_Team_Ready_Queue(team)) {
            team->DLink_Remove_From_Team_Ready_Queue(&m_dlinkhead_TeamReadyQueue.m_head);
        }
    }

    void Reverse_Team_Ready_Queue()
    {
        TeamInQueue *head = m_dlinkhead_TeamReadyQueue.m_head;
        TeamInQueue *tail = nullptr;

        while (head != nullptr) {
            TeamInQueue *next = head->Dlink_Next_Team_Ready_Queue();
            head->Dlink_Swap_Links_Team_Ready_Queue();
            tail = head;
            head = next;
        }

        m_dlinkhead_TeamReadyQueue.m_head = tail;
    }

    bool Get_AI_Base_Center(Coord3D *center) const
    {
        *center = m_baseCenter;
        return m_baseCenterValid;
    }

    void Set_Difficulty(GameDifficulty difficulty) { m_difficulty = difficulty; }
    GameDifficulty Get_Difficulty() const { return m_difficulty; }

private:
    struct DLINKHEAD_TeamBuildQueue
    {
        TeamInQueue *m_head;
        DLINKHEAD_TeamBuildQueue() : m_head(nullptr) {}
        ~DLINKHEAD_TeamBuildQueue()
        {
            captainslog_dbgassert(m_head == nullptr, "destroying dlinkhead still in a list TeamBuildQueue");
        }
    };

    struct DLINKHEAD_TeamReadyQueue
    {
        TeamInQueue *m_head;
        DLINKHEAD_TeamReadyQueue() : m_head(nullptr) {}
        ~DLINKHEAD_TeamReadyQueue()
        {
            captainslog_dbgassert(m_head == nullptr, "destroying dlinkhead still in a list TeamReadyQueue");
        }
    };

    DLINKHEAD_TeamBuildQueue m_dlinkhead_TeamBuildQueue;
    DLINKHEAD_TeamReadyQueue m_dlinkhead_TeamReadyQueue;
    Player *m_player;
    bool m_readyToBuildTeam;
    bool m_readyToBuildStructure;
    int m_teamTimer;
    int m_structureTimer;
    int m_teamDelaySeconds;
    int m_buildDelay;
    int m_teamDelay;
    int m_frameLastBuildingBuilt;
    GameDifficulty m_difficulty;
    int m_skillsetSelector;
    Coord3D m_baseCenter;
    bool m_baseCenterValid;
    float m_baseRadius;
    ObjectID m_structureRepairQueue[2];
    ObjectID m_dozerID;
    Coord3D m_dozerPosition;
    int m_structureRepairCount;
    bool m_dozerQueued;
    bool m_dozerRepairing;
    int m_bridgeRepairTimer;
    int m_supplySourceFrame;
    ObjectID m_supplySourceID;
    ObjectID m_supplyCenterID;
};

class AISkirmishPlayer : public AIPlayer
{
    IMPLEMENT_POOL(AISkirmishPlayer)

public:
    AISkirmishPlayer(Player *p);
    virtual ~AISkirmishPlayer() override;

    virtual bool Compute_Superweapon_Target(
        const SpecialPowerTemplate *special_power, Coord3D *pos, int player_index, float f) override;
    virtual void Update() override;
    virtual void New_Map() override;
    virtual void On_Unit_Produced(Object *factory, Object *unit) override;
    virtual void Build_Specific_AI_Team(TeamPrototype *team_proto, bool priority_build) override;
    virtual void Build_AI_Base_Defense(bool b) override;
    virtual void Build_AI_Base_Defense_Structure(const Utf8String &name, bool b) override;
    virtual void Build_Specific_AI_Building(const Utf8String &name) override;
    virtual void Recruit_Specific_AI_Team(TeamPrototype *team_proto, float recruit_radius) override;
    virtual bool Is_Skirmish_AI() override { return true; }
    virtual Player *Get_Ai_Enemy() override;
    virtual bool Check_Bridges(Object *obj, Waypoint *waypoint) override;
    virtual void Do_Base_Building() override;
    virtual void Check_Ready_Teams() override;
    virtual void Check_Queued_Teams() override;
    virtual void Do_Team_Building() override;
    virtual Object *Find_Dozer(const Coord3D *pos) override;
    virtual void Queue_Dozer() override;
    virtual bool Select_Team_To_Build() override;
    virtual bool Select_Team_To_Reinforce(int min_priority) override;
    virtual bool Start_Training(WorkOrder *order, bool busy_ok, Utf8String team_name) override;
    virtual bool Is_A_Good_Idea_To_Build_Team(TeamPrototype *team_proto) override;
    virtual void Process_Base_Building() override;
    virtual void Process_Team_Building() override;

    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

private:
    int m_flags1;
    int m_flags2;
    float m_angle1;
    float m_angle2;
    float m_angle3;
    float m_angle4;
    float m_angle5;
    float m_angle6;
    int m_nextGetEnemyFrame;
    Player *m_enemyPlayer;
};
