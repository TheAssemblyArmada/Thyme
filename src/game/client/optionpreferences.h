/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Class holding preferences from options.ini.
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
#include "gamelod.h"
#include "userpreferences.h"

class OptionPreferences : public UserPreferences
{
public:
    OptionPreferences();
    virtual ~OptionPreferences() {}

    int Get_Campaign_Difficulty();
    void Set_Campaign_Difficulty(int difficulty);
    bool Get_Alternate_Mouse_Mode_Enabled();
    bool Get_Retaliation_Mode_Enabled();
    bool Get_Double_Click_Attack_Move_Enabled();
    float Get_Scroll_Factor();
    bool Uses_System_Map_Dir();
    bool Save_Camera_In_Replays();
    bool Use_Camera_In_Replays();
    int Get_Ideal_Static_Game_Detail();
    int Get_Static_Game_Detail();
    void Set_Ideal_Static_Game_Detail(int level) { (*this)["IdealStaticGameLOD"] = g_staticGameLODNames[level]; }
    void Set_Static_Game_Detail(int level) { (*this)["StaticGameLOD"] = g_staticGameLODNames[level]; }
    Utf8String Get_Preferred_3D_Provider();
    Utf8String Get_Speaker_Type();
    float Get_Sound_Volume();
    float Get_3DSound_Volume();
    float Get_Speech_Volume();
    float Get_Music_Volume();
    bool Get_Cloud_Shadows_Enabled();
    bool Get_Lightmap_Enabled();
    bool Get_Smooth_Water_Enabled();
    bool Get_Trees_Enabled();
    bool Get_Extra_Animations_Disabled();
    bool Get_Use_Heat_Effects();
    bool Get_Dynamic_LOD_Enabled();
    bool Get_FPSLimit_Enabled();
    bool Get_3DShadows_Enabled();
    bool Get_2DShadows_Enabled();
    bool Get_Building_Occlusion_Enabled();
    int Get_Particle_Cap();
    int Get_Texture_Reduction();
    float Get_Gamma_Value();
    void Get_Resolution(int *x, int *y);
    bool Get_Send_Delay();
    int Get_Firewall_Behavior();
    int16_t Get_Firewall_Port_Allocation_Delta();
    uint16_t Get_Firewall_Port_Override();
    bool Get_Firewall_Need_Refresh();
    uint32_t Get_LAN_IPAddress();
    void Set_LAN_IPAddress(Utf8String address);
    void Set_LAN_IPAddress(uint32_t address);
    uint32_t Get_Online_IPAddress();
    void Set_Online_IPAddress(Utf8String address);
    void Set_Online_IPAddress(uint32_t address);

private:
};

#ifdef GAME_DLL
#include "hooker.h"
#endif