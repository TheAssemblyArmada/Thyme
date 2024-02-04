/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Campaign Manager
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
#include "asciistring.h"
#include "audioeventrts.h"
#include "gametype.h"
#include "mempoolobj.h"
#include "snapshot.h"
#include <list>

class INI;

class Mission : public MemoryPoolObject
{
    IMPLEMENT_POOL(Mission);

public:
    Mission() : m_voiceLength(0) {}
    virtual ~Mission() override {}

private:
    Utf8String m_name;
    Utf8String m_map;
    Utf8String m_nextMission;
    Utf8String m_introMovie;
    Utf8String m_objectiveLines[5];
    AudioEventRTS m_briefingVoice;
    Utf8String m_locationNameLabel;
    Utf8String m_unitNames[3];
    int m_voiceLength;
    Utf8String m_generalName;
    friend class CampaignManager;
    friend class Campaign;
};

class Campaign : public MemoryPoolObject
{
    IMPLEMENT_POOL(Campaign)

public:
    Campaign();
    virtual ~Campaign() override;
    Utf8String Get_Final_Victory_Movie();
    Mission *New_Mission(Utf8String mission_name);
    Mission *Get_Mission(Utf8String mission_name);
    Mission *Get_Next_Mission(Mission *mission_name);

    bool Is_Challenge_Campaign() const { return m_isChallengeCampaign; }

private:
    Utf8String m_name;
    Utf8String m_firstMission;
    Utf8String m_campaignNameLabel;
    std::list<Mission *> m_missionList;
    Utf8String m_finalVictoryMovie;
    bool m_isChallengeCampaign;
    Utf8String m_playerFaction;
    friend class ScriptEngine;
    friend class GameLogic;
    friend class CampaignManager;
};

class CampaignManager : public SnapShot
{
public:
    CampaignManager();
    ~CampaignManager();
    void Init();

    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    Mission *Goto_Next_Mission();
    void Set_Campaign_And_Mission(Utf8String campaign_name, Utf8String mission);
    void Set_Campaign(Utf8String campaign_name);
    Utf8String Get_Current_Map();
    int Get_Current_Mission_Number();
    Campaign *New_Campaign(Utf8String campaign_name);

    static void Parse_Mission_Part(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse(INI *ini);
    static const FieldParse *Get_Field_Parse() { return s_campaignFieldParseTable; }
    static const FieldParse s_campaignFieldParseTable[];

    Mission *Get_Current_Mission() const { return m_currentMission; }
    Campaign *Get_Current_Campaign() const { return m_currentCampaign; }
    bool Is_Challenge_Campaign() const { return false; }
    GameDifficulty Get_Difficulty() const { return m_difficulty; }
    bool Is_Victory() const { return m_isVictory; }

    void Set_Is_Victory(bool set) { m_isVictory = set; }
    void Set_Difficulty(GameDifficulty difficulty) { m_difficulty = difficulty; }
    void Set_Skill_Points(int points) { m_skillPoints = points; }

private:
    std::list<Campaign *> m_campaignList;
    Campaign *m_currentCampaign;
    Mission *m_currentMission;
    bool m_isVictory;
    int m_skillPoints;
    GameDifficulty m_difficulty;
    int m_playerFaction;
};

#ifdef GAME_DLL
extern CampaignManager *&g_theCampaignManager;
#else
extern CampaignManager *g_theCampaignManager;
#endif
