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
#include "gamedebug.h"
#include "gametype.h"
#include <cstddef>

#ifndef GAME_DLL
WaterSetting g_waterSettings[TIME_OF_DAY_COUNT];
WaterTransparencySetting *g_theWaterTransparency = nullptr;
#endif

FieldParse WaterSetting::m_waterSettingFieldParseTable[] = {
    {"SkyTexture", &INI::Parse_AsciiString, nullptr, offsetof(WaterSetting, m_skyTextureFile)},
    {"WaterTexture", &INI::Parse_AsciiString, nullptr, offsetof(WaterSetting, m_waterTextureFile)},
    {"Vertex00Color", &INI::Parse_RGBA_Color_Int, nullptr, offsetof(WaterSetting, m_vertex00Diffuse)},
    {"Vertex10Color", &INI::Parse_RGBA_Color_Int, nullptr, offsetof(WaterSetting, m_vertex10Diffuse)},
    {"Vertex01Color", &INI::Parse_RGBA_Color_Int, nullptr, offsetof(WaterSetting, m_vertex01Diffuse)},
    {"Vertex11Color", &INI::Parse_RGBA_Color_Int, nullptr, offsetof(WaterSetting, m_vertex11Diffuse)},
    {"DiffuseColor", &INI::Parse_RGBA_Color_Int, nullptr, offsetof(WaterSetting, m_waterDiffuseColor)},
    {"TransparentDiffuseColor", &INI::Parse_RGBA_Color_Int, nullptr, offsetof(WaterSetting, m_transparentWaterDiffuse)},
    {"UScrollPerMS", &INI::Parse_Real, nullptr, offsetof(WaterSetting, m_uScrollPerMs)},
    {"VScrollPerMS", &INI::Parse_Real, nullptr, offsetof(WaterSetting, m_vScrollPerMs)},
    {"SkyTexelsPerUnit", &INI::Parse_Real, nullptr, offsetof(WaterSetting, m_skyTexelsPerUnit)},
    {"WaterRepeatCount", &INI::Parse_Int, nullptr, offsetof(WaterSetting, m_waterRepeatCount)},
    {nullptr, nullptr, nullptr, 0}};

FieldParse WaterTransparencySetting::m_waterTransparencySettingFieldParseTable[] = {
    {"TransparentWaterDepth", &INI::Parse_Real, nullptr, offsetof(WaterTransparencySetting, m_transparentWaterDepth)},
    {"TransparentWaterMinOpacity",
        &INI::Parse_Real,
        nullptr,
        offsetof(WaterTransparencySetting, m_transparentWaterMinOpacity)},
    {"StandingWaterColor", &INI::Parse_RGB_Color, nullptr, offsetof(WaterTransparencySetting, m_standingWaterColor)},
    {"StandingWaterTexture", &INI::Parse_AsciiString, nullptr, offsetof(WaterTransparencySetting, m_standingWaterTexture)},
    {"AdditiveBlending", &INI::Parse_Bool, nullptr, offsetof(WaterTransparencySetting, m_additiveBlending)},
    {"RadarWaterColor", &INI::Parse_RGB_Color, nullptr, offsetof(WaterTransparencySetting, m_radarWaterColor)},
    {"SkyboxTextureN", &INI::Parse_AsciiString, nullptr, offsetof(WaterTransparencySetting, m_skyboxTextureN)},
    {"SkyboxTextureE", &INI::Parse_AsciiString, nullptr, offsetof(WaterTransparencySetting, m_skyboxTextureE)},
    {"SkyboxTextureS", &INI::Parse_AsciiString, nullptr, offsetof(WaterTransparencySetting, m_skyboxTextureS)},
    {"SkyboxTextureW", &INI::Parse_AsciiString, nullptr, offsetof(WaterTransparencySetting, m_skyboxTextureW)},
    {"SkyboxTextureT", &INI::Parse_AsciiString, nullptr, offsetof(WaterTransparencySetting, m_skyboxTextureT)},
    {nullptr, nullptr, nullptr, 0}};

void WaterSetting::Parse_Water_Setting(INI *ini)
{
    Utf8String token = ini->Get_Next_Token();
    int tod;

    for (tod = 0; tod < TIME_OF_DAY_COUNT; ++tod) {
        if (strcasecmp(g_timeOfDayNames[tod], token.Str()) == 0) {
            break;
        }
    }

    DEBUG_ASSERT_THROW(tod < TIME_OF_DAY_COUNT, 0xDEAD0006, "Failed to parse a valid time of day.\n");
    ini->Init_From_INI(&g_waterSettings[tod], m_waterSettingFieldParseTable);
}

WaterTransparencySetting::WaterTransparencySetting() :
    m_transparentWaterDepth(3.0f),
    m_transparentWaterMinOpacity(1.0f),
    m_standingWaterColor{1.0f, 1.0f, 1.0f},
    m_radarWaterColor{140.0f, 140.0f, 255.0f},
    m_additiveBlending(false),
    m_standingWaterTexture("TWWater01.tga"),
    m_skyboxTextureN("TSMorningN.tga"),
    m_skyboxTextureE("TSMorningE.tga"),
    m_skyboxTextureS("TSMorningS.tga"),
    m_skyboxTextureW("TSMorningW.tga"),
    m_skyboxTextureT("TSMorningT.tga")
{
}

void WaterTransparencySetting::Parse_Water_Transparency(INI *ini)
{
    if (g_theWaterTransparency == nullptr) {
        g_theWaterTransparency = new WaterTransparencySetting;
    } else {
        DEBUG_ASSERT_THROW(ini->Get_Load_Type() == INI_LOAD_CREATE_OVERRIDES,
            0xDEAD0006,
            "g_theWaterTransparency is not null, but m_loadType is not INI_LOAD_CREATE_OVERRIDES.\n");
        WaterTransparencySetting *new_wts = new WaterTransparencySetting;
        *new_wts = *g_theWaterTransparency;
        new_wts->m_isAllocated = true;
        g_theWaterTransparency->Add_Override(new_wts);
    }

    ini->Init_From_INI(g_theWaterTransparency->Get_Override(), m_waterTransparencySettingFieldParseTable);

    if (ini->Get_Load_Type() == INI_LOAD_CREATE_OVERRIDES) {
        if (g_theWaterTransparency != g_theWaterTransparency->Get_Override()) {
            // TODO requires TerrainVisual virtual table layout implementing.
        }
    }
}
