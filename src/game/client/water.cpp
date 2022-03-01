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
#include <captainslog.h>
#include <cstddef>

#ifndef GAME_DLL
WaterSetting g_waterSettings[TIME_OF_DAY_COUNT];
Override<WaterTransparencySetting> g_theWaterTransparency;
#endif

// clang-format off
const FieldParse WaterSetting::m_waterSettingFieldParseTable[] = {
    FIELD_PARSE_ASCIISTRING("SkyTexture", WaterSetting, m_skyTextureFile),
    FIELD_PARSE_ASCIISTRING("WaterTexture", WaterSetting, m_waterTextureFile),
    FIELD_PARSE_RGBA_COLOR_INT("Vertex00Color", WaterSetting, m_vertex00Diffuse),
    FIELD_PARSE_RGBA_COLOR_INT("Vertex10Color", WaterSetting, m_vertex10Diffuse),
    FIELD_PARSE_RGBA_COLOR_INT("Vertex01Color", WaterSetting, m_vertex01Diffuse),
    FIELD_PARSE_RGBA_COLOR_INT("Vertex11Color", WaterSetting, m_vertex11Diffuse),
    FIELD_PARSE_RGBA_COLOR_INT("DiffuseColor", WaterSetting, m_waterDiffuseColor),
    FIELD_PARSE_RGBA_COLOR_INT("TransparentDiffuseColor", WaterSetting, m_transparentWaterDiffuse),
    FIELD_PARSE_REAL("UScrollPerMS", WaterSetting, m_uScrollPerMs),
    FIELD_PARSE_REAL("VScrollPerMS", WaterSetting, m_vScrollPerMs),
    FIELD_PARSE_REAL("SkyTexelsPerUnit", WaterSetting, m_skyTexelsPerUnit),
    FIELD_PARSE_INT("WaterRepeatCount", WaterSetting, m_waterRepeatCount),
    FIELD_PARSE_LAST
};

const FieldParse WaterTransparencySetting::m_waterTransparencySettingFieldParseTable[] = {
    FIELD_PARSE_REAL("TransparentWaterDepth", WaterTransparencySetting, m_transparentWaterDepth),
    FIELD_PARSE_REAL("TransparentWaterMinOpacity", WaterTransparencySetting, m_transparentWaterMinOpacity),
    FIELD_PARSE_RGB_COLOR("StandingWaterColor", WaterTransparencySetting, m_standingWaterColor),
    FIELD_PARSE_ASCIISTRING("StandingWaterTexture", WaterTransparencySetting, m_standingWaterTexture),
    FIELD_PARSE_BOOL("AdditiveBlending", WaterTransparencySetting, m_additiveBlending),
    FIELD_PARSE_RGB_COLOR("RadarWaterColor", WaterTransparencySetting, m_radarWaterColor),
    FIELD_PARSE_ASCIISTRING("SkyboxTextureN", WaterTransparencySetting, m_skyboxTextureN),
    FIELD_PARSE_ASCIISTRING("SkyboxTextureE", WaterTransparencySetting, m_skyboxTextureE),
    FIELD_PARSE_ASCIISTRING("SkyboxTextureS", WaterTransparencySetting, m_skyboxTextureS),
    FIELD_PARSE_ASCIISTRING("SkyboxTextureW", WaterTransparencySetting, m_skyboxTextureW),
    FIELD_PARSE_ASCIISTRING("SkyboxTextureT", WaterTransparencySetting, m_skyboxTextureT),
    FIELD_PARSE_LAST
};
// clang-format on

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
            // TODO requires TerrainVisual virtual table layout implementing.
            // g_theTerrainVisual->Replace_Skybox_Textures(oldtex, newtex);
        }
    }
}
