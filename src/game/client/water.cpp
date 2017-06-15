////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: WATER.CPP
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Configuration for water effects.
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
#include "water.h"
#include "gamedebug.h"
#include "gametype.h"

// WaterSetting g_waterSettings[TIME_OF_DAY_COUNT];

FieldParse WaterSetting::m_waterSettingFieldParseTable[] =
{
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

void WaterSetting::Parse_Water_Setting(INI *ini)
{
    AsciiString token = ini->Get_Next_Token();
    int tod;

    for ( tod = 0; tod < TIME_OF_DAY_COUNT; ++tod ) {
        if ( strcasecmp(g_timeOfDayNames[tod], token.Str()) == 0 ) {
            break;
        }
    }

    ASSERT_THROW(tod < TIME_OF_DAY_COUNT, 0xDEAD0006);
    ini->Init_From_INI(&g_waterSettings[tod], m_waterSettingFieldParseTable);
}
