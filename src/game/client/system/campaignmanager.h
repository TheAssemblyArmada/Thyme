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
#include "gametype.h"
#include "mempoolobj.h"
#include "snapshot.h"
#include <list>

class Mission;

class Campaign : public MemoryPoolObject
{
    IMPLEMENT_POOL(Campaign)

public:
    ~Campaign() override;

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
};

class CampaignManager : public SnapShot
{
public:
    void CRC_Snapshot(Xfer *xfer) override;
    void Xfer_Snapshot(Xfer *xfer) override;
    void Load_Post_Process() override;

    Campaign *Get_Current_Campaign() { return m_currentCampaign; }
    void Set_Is_Victory(bool set) { m_isVictory = set; }

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
