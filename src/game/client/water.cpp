/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Configuration for water effects.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "water.h"
#include "gametype.h"
#include "terrainvisual.h"
#include <captainslog.h>
#include <cstddef>

#ifndef GAME_DLL
WaterSetting g_waterSettings[TIME_OF_DAY_COUNT];
Override<WaterTransparencySetting> g_theWaterTransparency;
#endif

const FieldParse WaterSetting::m_waterSettingFieldParseTable[] = {
    { "SkyTexture", &INI::Parse_AsciiString, nullptr, offsetof(WaterSetting, m_skyTextureFile) },
    { "WaterTexture", &INI::Parse_AsciiString, nullptr, offsetof(WaterSetting, m_waterTextureFile) },
    { "Vertex00Color", &INI::Parse_RGBA_Color_Int, nullptr, offsetof(WaterSetting, m_vertex00Diffuse) },
    { "Vertex10Color", &INI::Parse_RGBA_Color_Int, nullptr, offsetof(WaterSetting, m_vertex10Diffuse) },
    { "Vertex01Color", &INI::Parse_RGBA_Color_Int, nullptr, offsetof(WaterSetting, m_vertex01Diffuse) },
    { "Vertex11Color", &INI::Parse_RGBA_Color_Int, nullptr, offsetof(WaterSetting, m_vertex11Diffuse) },
    { "DiffuseColor", &INI::Parse_RGBA_Color_Int, nullptr, offsetof(WaterSetting, m_waterDiffuseColor) },
    { "TransparentDiffuseColor", &INI::Parse_RGBA_Color_Int, nullptr, offsetof(WaterSetting, m_transparentWaterDiffuse) },
    { "UScrollPerMS", &INI::Parse_Real, nullptr, offsetof(WaterSetting, m_uScrollPerMs) },
    { "VScrollPerMS", &INI::Parse_Real, nullptr, offsetof(WaterSetting, m_vScrollPerMs) },
    { "SkyTexelsPerUnit", &INI::Parse_Real, nullptr, offsetof(WaterSetting, m_skyTexelsPerUnit) },
    { "WaterRepeatCount", &INI::Parse_Int, nullptr, offsetof(WaterSetting, m_waterRepeatCount) },
    { nullptr, nullptr, nullptr, 0 }
};

const FieldParse WaterTransparencySetting::m_waterTransparencySettingFieldParseTable[] = {
    { "TransparentWaterDepth", &INI::Parse_Real, nullptr, offsetof(WaterTransparencySetting, m_transparentWaterDepth) },
    { "TransparentWaterMinOpacity",
        &INI::Parse_Real,
        nullptr,
        offsetof(WaterTransparencySetting, m_transparentWaterMinOpacity) },
    { "StandingWaterColor", &INI::Parse_RGB_Color, nullptr, offsetof(WaterTransparencySetting, m_standingWaterColor) },
    { "StandingWaterTexture", &INI::Parse_AsciiString, nullptr, offsetof(WaterTransparencySetting, m_standingWaterTexture) },
    { "AdditiveBlending", &INI::Parse_Bool, nullptr, offsetof(WaterTransparencySetting, m_additiveBlending) },
    { "RadarWaterColor", &INI::Parse_RGB_Color, nullptr, offsetof(WaterTransparencySetting, m_radarWaterColor) },
    { "SkyboxTextureN", &INI::Parse_AsciiString, nullptr, offsetof(WaterTransparencySetting, m_skyboxTextureN) },
    { "SkyboxTextureE", &INI::Parse_AsciiString, nullptr, offsetof(WaterTransparencySetting, m_skyboxTextureE) },
    { "SkyboxTextureS", &INI::Parse_AsciiString, nullptr, offsetof(WaterTransparencySetting, m_skyboxTextureS) },
    { "SkyboxTextureW", &INI::Parse_AsciiString, nullptr, offsetof(WaterTransparencySetting, m_skyboxTextureW) },
    { "SkyboxTextureT", &INI::Parse_AsciiString, nullptr, offsetof(WaterTransparencySetting, m_skyboxTextureT) },
    { nullptr, nullptr, nullptr, 0 }
};

// Was originally INI::parseWaterSettingDefinition
void WaterSetting::Parse_Water_Setting_Definition(INI *ini)
{
    Utf8String token = ini->Get_Next_Token();
    int tod;

    for (tod = 0; tod < TIME_OF_DAY_COUNT; ++tod) {
        if (strcasecmp(g_timeOfDayNames[tod], token.Str()) == 0) {
            break;
        }
    }

    captainslog_relassert(tod < TIME_OF_DAY_COUNT, 0xDEAD0006, "Failed to parse a valid time of day.");
    ini->Init_From_INI(&g_waterSettings[tod], m_waterSettingFieldParseTable);
}

WaterTransparencySetting::WaterTransparencySetting() :
    m_transparentWaterDepth(3.0f),
    m_transparentWaterMinOpacity(1.0f),
    m_standingWaterColor{ 1.0f, 1.0f, 1.0f },
    m_radarWaterColor{ 140.0f, 140.0f, 255.0f },
    m_additiveBlending(false),
    m_standingWaterTexture("TWWater01.tga"),
    m_skyboxTextureN("TSMorningN.tga"),
    m_skyboxTextureE("TSMorningE.tga"),
    m_skyboxTextureS("TSMorningS.tga"),
    m_skyboxTextureW("TSMorningW.tga"),
    m_skyboxTextureT("TSMorningT.tga")
{
}

// Was originally INI::parseWaterTransparencyDefinition
void WaterTransparencySetting::Parse_Water_Transparency_Definition(INI *ini)
{
    if (*g_theWaterTransparency) {
        if (ini->Get_Load_Type() == INI_LOAD_CREATE_OVERRIDES) {
            WaterTransparencySetting *old_wts = g_theWaterTransparency;
            WaterTransparencySetting *new_wts = NEW_POOL_OBJ(WaterTransparencySetting);
            *new_wts = *old_wts;
            new_wts->Set_Is_Allocated();
            old_wts->Friend_Get_Final_Override()->Set_Next(new_wts);
        } else {
            throw CODE_06;
        }
    } else {
        g_theWaterTransparency = NEW_POOL_OBJ(WaterTransparencySetting);
    }

    Overridable *setting = g_theWaterTransparency;
    setting = setting->Friend_Get_Final_Override();
    ini->Init_From_INI(setting, m_waterTransparencySettingFieldParseTable);

    if (ini->Get_Load_Type() == INI_LOAD_CREATE_OVERRIDES) {
        WaterTransparencySetting *oldsetting = g_theWaterTransparency;
        Override<WaterTransparencySetting> newsetting(g_theWaterTransparency);
        if (oldsetting != *newsetting) {
            const Utf8String *oldtex[5];
            const Utf8String *newtex[5];
            oldtex[0] = &oldsetting->m_skyboxTextureN;
            newtex[0] = &newsetting->m_skyboxTextureN;
            oldtex[1] = &oldsetting->m_skyboxTextureE;
            newtex[1] = &newsetting->m_skyboxTextureE;
            oldtex[2] = &oldsetting->m_skyboxTextureS;
            newtex[2] = &newsetting->m_skyboxTextureS;
            oldtex[3] = &oldsetting->m_skyboxTextureW;
            newtex[3] = &newsetting->m_skyboxTextureW;
            oldtex[4] = &oldsetting->m_skyboxTextureT;
            newtex[4] = &newsetting->m_skyboxTextureT;
            g_theTerrainVisual->Replace_Skybox_Textures(oldtex, newtex);
        }
    }
}
