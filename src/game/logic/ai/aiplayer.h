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

class TeamInQueue;
class WorkOrder;

// member names are a best guess at this point
class AIPlayer : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_POOL(AIPlayer)

public:
    virtual ~AIPlayer() override;
    virtual bool Compute_Superweapon_Target(
        const SpecialPowerTemplate *special_power, Coord3D *pos, int player_index, float f);
    virtual void Update();
    virtual void New_Map();
    virtual void On_Unit_Produced(Object *factory, Object *unit);
    virtual void On_Structure_Produced(Object *factory, Object *bldg);
    virtual void Build_Specific_AI_Team(TeamPrototype *team_proto, bool priorty_build);
    virtual void Build_AI_Base_Defense(bool b);
    virtual void Build_AI_Base_Defense_Structure(const Utf8String &name, bool b);
    virtual void Build_Specific_AI_Building(const Utf8String &name);
    virtual void Recruit_Specific_AI_Team(TeamPrototype *team_proto, float recruit_radius);
    virtual bool Is_Skirmish_AI();
    virtual Player *Get_Ai_Enemy();
    virtual bool Check_Bridges(Object *obj, Waypoint *waypoint);
    virtual void Repair_Structure(ObjectID id);
    virtual void Select_Skillset(int);
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
    virtual bool Is_A_Good_Idea_To_Build_Team(TeamPrototype *proto);
    virtual void Process_Base_Building();
    virtual void Process_Team_Building();

    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    bool Get_AI_Base_Center(Coord3D *center)
    {
        *center = m_baseCenter;
        return m_baseCenterValid;
    }

private:
    struct DLINKHEAD_TeamBuildQueue
    {
        TeamInQueue *m_head;
    };

    struct DLINKHEAD_TeamReadyQueue
    {
        TeamInQueue *m_head;
    };

    DLINKHEAD_TeamBuildQueue m_dlinkhead_TeamBuildQueue;
    DLINKHEAD_TeamReadyQueue m_dlinkhead_TeamReadyQueue;
    bool m_readyToBuildTeam;
    bool m_readyToBuildStructure;
    int m_teamTimer;
    int m_structureTimer;
    int m_teamSeconds;
    int m_buildDelay;
    int m_teamDelay;
    int m_frameLastBuildingBuilt;
    GameDifficulty m_difficulty;
    int m_skillSet;
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
