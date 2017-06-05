////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: OPTIONPREFERENCES.CPP
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Class holding preferences from options.ini.
//
//       License:: Thyme is free software: you can redistribute it and/or 
//                 modify it under the terms of the GNU General Public License 
//                 as published by the Free Software Foundation, either version 
//                 2 of the License, or (at your option) any later version.
//
//                 A full copy of the GNU General Public License can be found in
//                 LICENSE
//
////////////////////////////////////////////////////////////////////////////////
#include "always.h"
#include "optionpreferences.h"
#include "gamelod.h"
#include "globaldata.h"
#include "hooker.h"
#include "minmax.h"
#include <cstdio>

OptionPreferences::OptionPreferences()
{
    DEBUG_LOG("Loading option preferences file.\n");
    Load("Options.ini");
}

int OptionPreferences::Get_Campaign_Difficulty()
{
    // TODO, needs script engine member.
    return Call_Method<int, OptionPreferences>(0x00462BA0, this);
}

void OptionPreferences::Set_Campaign_Difficulty(int difficulty)
{
    // TODO, needs script engine member.
    return Call_Method<void, OptionPreferences, int>(0x00462CB0, this, difficulty);
}

bool OptionPreferences::Get_Alternate_Mouse_Mode_Enabled()
{
    auto it = find("UseAlternateMouse");

    if ( it == end() ) {
        return g_theWriteableGlobalData->m_alternateMouseEnabled;
    }

    return strcasecmp(it->second.Str(), "yes") == 0;
}

bool OptionPreferences::Get_Retaliation_Mode_Enabled()
{
    auto it = find("Retaliation");

    if ( it == end() ) {
        return g_theWriteableGlobalData->m_retaliationModeEnabled;
    }

    return strcasecmp(it->second.Str(), "yes") == 0;
}

bool OptionPreferences::Get_Double_Click_Attack_Move_Enabled()
{
    auto it = find("UseDoubleClickAttackMove");

    if ( it == end() ) {
        return g_theWriteableGlobalData->m_doubleClickAttackMovesEnabled;
    }

    return strcasecmp(it->second.Str(), "yes") == 0;
}

float OptionPreferences::Get_Scroll_Factor()
{
    auto it = find("ScrollFactor");

    if ( it == end() ) {
        return g_theWriteableGlobalData->m_keyboardDefaultScrollFactor;
    }

    int factor = Clamp(atoi(it->second.Str()), 0, 100);

    return (float)(factor / 100.0f);
}

bool OptionPreferences::Uses_System_Map_Dir()
{
    auto it = find("UseSystemMapDir");

    if ( it == end() ) {
        return true;
    }

    return strcasecmp(it->second.Str(), "yes") == 0;
}

bool OptionPreferences::Save_Camera_In_Replays()
{
    auto it = find("SaveCameraInReplays");

    if ( it == end() ) {
        return true;
    }

    return strcasecmp(it->second.Str(), "yes") == 0;
}

bool OptionPreferences::Use_Camera_In_Replays()
{
    auto it = find("UseCameraInReplays");

    if ( it == end() ) {
        return true;
    }

    return strcasecmp(it->second.Str(), "yes") == 0;
}

int OptionPreferences::Get_Ideal_Static_Game_Detail()
{
    auto it = find("IdealStaticGameLOD");

    if ( it == end() ) {
        return STATLOD_INVALID;
    }

    return g_theGameLODManager->Get_Static_LOD_Index(it->second);
}

int OptionPreferences::Get_Static_Game_Detail()
{
    auto it = find("StaticGameLOD");

    if ( it == end() ) {
        return g_theGameLODManager->Get_Static_LOD_Level();
    }

    return g_theGameLODManager->Get_Static_LOD_Index(it->second);
}

AsciiString OptionPreferences::Get_Preferred_3D_Provider()
{
    // TODO needs AudioInterface
    return Call_Method<AsciiString, OptionPreferences>(0x00464720, this);
}

AsciiString OptionPreferences::Get_Speaker_Type()
{
    // TODO needs AudioInterface
    return Call_Method<AsciiString, OptionPreferences>(0x00464800, this);
}

float OptionPreferences::Get_Sound_Volume()
{
    // TODO needs AudioInterface
    return Call_Method<float, OptionPreferences>(0x004648F0, this);
}

float OptionPreferences::Get_3DSound_Volume()
{
    // TODO needs AudioInterface
    return Call_Method<float, OptionPreferences>(0x00464A40, this);
}

float OptionPreferences::Get_Speech_Volume()
{
    // TODO needs AudioInterface
    return Call_Method<float, OptionPreferences>(0x00464B90, this);
}

float OptionPreferences::Get_Music_Volume()
{
    // TODO needs AudioInterface
    return Call_Method<float, OptionPreferences>(0x00465AC0, this);
}

bool OptionPreferences::Get_Cloud_Shadows_Enabled()
{
    auto it = find("UseCloudMap");

    if ( it == end() ) {
        return g_theWriteableGlobalData->m_useCloudMap;
    }

    return strcasecmp(it->second.Str(), "yes") == 0;
}

bool OptionPreferences::Get_Lightmap_Enabled()
{
    auto it = find("UseLightMap");

    if ( it == end() ) {
        return g_theWriteableGlobalData->m_useLightMap;
    }

    return strcasecmp(it->second.Str(), "yes") == 0;
}

bool OptionPreferences::Get_Smooth_Water_Enabled()
{
    auto it = find("ShowSoftWaterEdge");

    if ( it == end() ) {
        return g_theWriteableGlobalData->m_showSoftWaterEdge;
    }

    return strcasecmp(it->second.Str(), "yes") == 0;
}

bool OptionPreferences::Get_Trees_Enabled()
{
    auto it = find("ShowTrees");

    if ( it == end() ) {
        return g_theWriteableGlobalData->m_useTrees;
    }

    return strcasecmp(it->second.Str(), "yes") == 0;
}

bool OptionPreferences::Get_Extra_Animations_Disabled()
{
    auto it = find("ExtraAnimations");

    if ( it == end() ) {
        return g_theWriteableGlobalData->m_extraAnimations;
    }

    return strcasecmp(it->second.Str(), "yes") == 0;
}

bool OptionPreferences::Get_Use_Heat_Effects()
{
    auto it = find("HeatEffects");

    if ( it == end() ) {
        return g_theWriteableGlobalData->m_useHeatEffects;
    }

    return strcasecmp(it->second.Str(), "yes") == 0;
}

bool OptionPreferences::Get_Dynamic_LOD_Enabled()
{
    auto it = find("DynamicLOD");

    if ( it == end() ) {
        return g_theWriteableGlobalData->m_dynamicLOD;
    }

    return strcasecmp(it->second.Str(), "yes") == 0;
}

bool OptionPreferences::Get_FPSLimit_Enabled()
{
    auto it = find("FPSLimit");

    if ( it == end() ) {
        return g_theWriteableGlobalData->m_useFPSLimit;
    }

    return strcasecmp(it->second.Str(), "yes") == 0;
}

bool OptionPreferences::Get_3DShadows_Enabled()
{
    auto it = find("UseShadowVolumes");

    if ( it == end() ) {
        return g_theWriteableGlobalData->m_shadowVolumes;
    }

    return strcasecmp(it->second.Str(), "yes") == 0;
}

bool OptionPreferences::Get_2DShadows_Enabled()
{
    auto it = find("UseShadowDecals");

    if ( it == end() ) {
        return g_theWriteableGlobalData->m_shadowDecals;
    }

    return strcasecmp(it->second.Str(), "yes") == 0;
}

bool OptionPreferences::Get_Building_Occlusion_Enabled()
{
    auto it = find("BuildingOcclusion");

    if ( it == end() ) {
        return g_theWriteableGlobalData->m_useBehindBuildingMarker;
    }

    return strcasecmp(it->second.Str(), "yes") == 0;
}

int OptionPreferences::Get_Particle_Cap()
{
    auto it = find("MaxParticleCount");

    if ( it == end() ) {
        return g_theWriteableGlobalData->m_maxParticleCount;
    }

    int ret = atoi(it->second.Str());

    return MIN(ret, 100);
}

int OptionPreferences::Get_Texture_Reduction()
{
    auto it = find("TextureReduction");

    if ( it == end() ) {
        return -1;
    }

    int ret = atoi(it->second.Str());

    return MAX(ret, 2);
}

float OptionPreferences::Get_Gamma_Value()
{
    auto it = find("Gamma");

    if ( it == end() ) {
        return 50.0f;
    }

    return (float)atoi(it->second.Str());
}

void OptionPreferences::Get_Resolution(int *x, int *y)
{
    *x = g_theWriteableGlobalData->m_xResolution;
    *y = g_theWriteableGlobalData->m_yResolution;

    auto it = find("Resolution");

    if ( it == end() ) {
        return;
    }

    int tmpx;
    int tmpy;

    if ( sscanf(it->second.Str(), "%d%d", &tmpx, &tmpy) != 2 ) {
        return;
    }

    *x = tmpx;
    *y = tmpy;
}

bool OptionPreferences::Get_Send_Delay()
{
    auto it = find("SendDelay");

    if ( it == end() ) {
        return g_theWriteableGlobalData->m_sendDelay;
    }

    return strcasecmp(it->second.Str(), "yes") == 0;
}

int OptionPreferences::Get_Firewall_Behavior()
{
    auto it = find("FirewallBehaviour");

    if ( it == end() ) {
        return g_theWriteableGlobalData->m_firewallBehaviour;
    }

    int ret = atoi(it->second.Str());

    return MAX(ret, 0);
}

int16_t OptionPreferences::Get_Firewall_Port_Allocation_Delta()
{
    auto it = find("FirewallPortAllocationDelta");

    if ( it == end() ) {
        return g_theWriteableGlobalData->m_firewallPortAllocationDelta;
    }

    return atoi(it->second.Str());
}

uint16_t OptionPreferences::Get_Firewall_Port_Override()
{
    auto it = find("FirewallPortOverride");

    if ( it == end() ) {
        return g_theWriteableGlobalData->m_firewallPortOverrides;
    }

    return atoi(it->second.Str());
}

bool OptionPreferences::Get_Firewall_Need_Refresh()
{
    // Original creates a second OptionPreferences instance
    // to query for this string, not sure what purpose that serves.
    auto it = find("FirewallNeedToRefresh");

    if ( it != end() ) {
        if ( strcasecmp(it->second.Str(), "true") == 0 ) {
            g_theWriteableGlobalData->m_firewallBehaviour = 0;
        }
    }

    if ( g_theWriteableGlobalData->m_firewallBehaviour != 0 ) {
        return true;
    }

    return false;
}

uint32_t OptionPreferences::Get_LAN_IPAddress()
{
    // TODO needs IPEnumeration
    return Call_Method<uint32_t, OptionPreferences>(0x00462EA0, this);
}

void OptionPreferences::Set_LAN_IPAddress(AsciiString address)
{
    // TODO needs IPEnumeration
    return Call_Method<void, OptionPreferences, AsciiString>(0x004630E0, this, address);
}

void OptionPreferences::Set_LAN_IPAddress(uint32_t address)
{
    // TODO needs IPEnumeration
    return Call_Method<void, OptionPreferences, uint32_t>(0x004632B0, this, address);
}

uint32_t OptionPreferences::Get_Online_IPAddress()
{
    // TODO needs IPEnumeration
    return Call_Method<uint32_t, OptionPreferences>(0x004634B0, this);
}

void OptionPreferences::Set_Online_IPAddress(AsciiString address)
{
    // TODO needs IPEnumeration
    return Call_Method<void, OptionPreferences, AsciiString>(0x004636F0, this, address);
}

void OptionPreferences::Set_Online_IPAddress(uint32_t address)
{
    // TODO needs IPEnumeration
    return Call_Method<void, OptionPreferences, uint32_t>(0x004638C0, this, address);
}
