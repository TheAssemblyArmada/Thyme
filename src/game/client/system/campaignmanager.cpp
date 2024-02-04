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

#ifndef GAME_DLL
CampaignManager *g_theCampaignManager;
#endif

// clang-format off
const FieldParse CampaignManager::s_campaignFieldParseTable[] = {
    {"Mission", &CampaignManager::Parse_Mission_Part, nullptr, 0},
    {"FirstMission", &INI::Parse_AsciiString, nullptr, offsetof(Campaign, m_firstMission)},
    {"CampaignNameLabel", &INI::Parse_AsciiString, nullptr, offsetof(Campaign, m_campaignNameLabel)},
    {"FinalVictoryMovie", &INI::Parse_AsciiString, nullptr, offsetof(Campaign, m_finalVictoryMovie)},
    {"IsChallengeCampaign", &INI::Parse_Bool, nullptr, offsetof(Campaign, m_isChallengeCampaign)},
    {"PlayerFaction", &INI::Parse_AsciiString, nullptr, offsetof(Campaign, m_playerFaction)},
    {nullptr, nullptr, nullptr, 0}
};
// clang-format on

Campaign::Campaign() : m_isChallengeCampaign(false) {}

Campaign::~Campaign()
{
    for (auto it = m_missionList.begin(); it != m_missionList.end(); it = m_missionList.erase(it)) {
        Mission *mission = *it;

        if (mission != nullptr) {
            mission->Delete_Instance();
        }
    }
}

Mission *Campaign::New_Mission(Utf8String mission_name)
{
    mission_name.To_Lower();

    for (auto it = m_missionList.begin(); it != m_missionList.end(); it++) {
        Mission *mission = *it;

        if (mission->m_name.Compare(mission_name) == 0) {
            m_missionList.erase(it);
            mission->Delete_Instance();
            break;
        }
    }

    Mission *mission = new Mission();
    mission->m_name.Set(mission_name);
    m_missionList.push_back(mission);
    return mission;
}

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
        (*it)->Delete_Instance();
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

void CampaignManager::Parse(INI *ini)
{
    Utf8String name;
    name.Set(ini->Get_Next_Token());
    captainslog_dbgassert(g_theCampaignManager != nullptr, "CampaignManager::Parse: Unable to Get TheCampaignManager");

    if (g_theCampaignManager != nullptr) {
        Campaign *campaign = g_theCampaignManager->New_Campaign(name);
        captainslog_dbgassert(campaign != nullptr, "CampaignManager::Parse: Unable to allocate campaign '%s'");
        ini->Init_From_INI(campaign, Get_Field_Parse());
    }
}

Campaign *CampaignManager::New_Campaign(Utf8String campaign_name)
{
    campaign_name.To_Lower();

    for (auto it = m_campaignList.begin(); it != m_campaignList.end(); it++) {
        Campaign *campaign = *it;

        if (campaign->m_name.Compare(campaign_name) == 0) {
            m_campaignList.erase(it);
            campaign->Delete_Instance();
            break;
        }
    }

    Campaign *campaign = new Campaign();
    campaign->m_name.Set(campaign_name);
    m_campaignList.push_back(campaign);
    return campaign;
}

void CampaignManager::Parse_Mission_Part(INI *ini, void *formal, void *store, const void *user_data)
{
    // clang-format off
    static const FieldParse myFieldParse[] = 
    {
        {"Map", &INI::Parse_AsciiString, nullptr, offsetof(Mission, m_map)},
        {"NextMission", &INI::Parse_AsciiString, nullptr, offsetof(Mission, m_nextMission)},
        {"IntroMovie", &INI::Parse_AsciiString, nullptr, offsetof(Mission, m_introMovie)},
        {"ObjectiveLine0", &INI::Parse_AsciiString, nullptr, offsetof(Mission, m_objectiveLines[0])},
        {"ObjectiveLine1", &INI::Parse_AsciiString, nullptr, offsetof(Mission, m_objectiveLines[1])},
        {"ObjectiveLine2", &INI::Parse_AsciiString, nullptr, offsetof(Mission, m_objectiveLines[2])},
        {"ObjectiveLine3", &INI::Parse_AsciiString, nullptr, offsetof(Mission, m_objectiveLines[3])},
        {"BriefingVoice", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(Mission, m_briefingVoice)},
        {"UnitNames0", &INI::Parse_AsciiString, nullptr, offsetof(Mission, m_unitNames[0])},
        {"UnitNames1", &INI::Parse_AsciiString, nullptr, offsetof(Mission, m_unitNames[1])},
        {"UnitNames2", &INI::Parse_AsciiString, nullptr, offsetof(Mission, m_unitNames[2])},
        {"GeneralName", &INI::Parse_AsciiString, nullptr, offsetof(Mission, m_generalName)},
        {"LocationNameLabel", &INI::Parse_AsciiString, nullptr, offsetof(Mission, m_locationNameLabel)},
        {"VoiceLength", &INI::Parse_Int, nullptr, offsetof(Mission, m_voiceLength)},
        {nullptr, nullptr, nullptr, 0}
    };
    // clang-format on

    Utf8String name;
    name.Set(ini->Get_Next_Token());
    Mission *mission = static_cast<Campaign *>(formal)->New_Mission(name);
    ini->Init_From_INI(mission, myFieldParse);
}
