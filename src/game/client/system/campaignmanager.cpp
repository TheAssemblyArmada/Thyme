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
#include "campaignmanager.h"
#include "challengegenerals.h"
#include "ini.h"

CampaignManager::CampaignManager() :
    m_currentCampaign(nullptr),
    m_currentMission(nullptr),
    m_isVictory(false),
    m_skillPoints(0),
    m_difficulty(DIFFICULTY_NORMAL),
    m_playerFaction(0)
{
}

CampaignManager::~CampaignManager()
{
    m_currentCampaign = nullptr;
    m_currentMission = nullptr;

    for (auto it = m_campaignList.begin(); it != m_campaignList.end(); it = m_campaignList.erase(it)) {
        delete *it;
    }
}

void CampaignManager::Init()
{
    INI ini;
    ini.Load("Data\\INI\\Campaign.ini", INI_LOAD_OVERWRITE, nullptr);
}

void CampaignManager::Xfer_Snapshot(Xfer *xfer)
{
#ifdef GAME_DLL
    Call_Method<void, SnapShot, Xfer *>(PICK_ADDRESS(0x00518680, 0x008CFCFB), this, xfer);
#endif
}

void CampaignManager::Load_Post_Process()
{
    if (g_theChallengeGenerals != nullptr) {
        g_theChallengeGenerals->Set_Player_Faction(m_playerFaction);
    } else {
        captainslog_dbgassert(false,
            "TheChallengeGenerals singleton does not exist. This loaded game will not have a working Continue button for GC "
            "mode.");
    }
}

#ifndef GAME_DLL
CampaignManager *g_theCampaignManager;
#endif
