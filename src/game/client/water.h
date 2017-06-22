////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: WATER.H
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
#pragma once

#ifndef WATER_H
#define WATER_H

#ifndef THYME_STANDALONE
#include "hooker.h"
#endif

#include "always.h"
#include "overridable.h"
#include "asciistring.h"
#include "color.h"
#include "ini.h"

class WaterSetting
{
public:
    virtual ~WaterSetting() {}

    static void Parse_Water_Setting(INI *ini);

private:
    AsciiString m_skyTextureFile;
    AsciiString m_waterTextureFile;
    int m_waterRepeatCount;
    float m_skyTexelsPerUnit;
    RGBAColorInt m_vertex00Diffuse;
    RGBAColorInt m_vertex10Diffuse;
    RGBAColorInt m_vertex11Diffuse;
    RGBAColorInt m_vertex01Diffuse;
    RGBAColorInt m_waterDiffuseColor;
    RGBAColorInt m_transparentWaterDiffuse;
    float m_uScrollPerMs;
    float m_vScrollPerMs;

    static FieldParse m_waterSettingFieldParseTable[];
};

class WaterTransparencySetting : public Overridable
{
    IMPLEMENT_POOL(WaterTransparencySetting);
public:
    WaterTransparencySetting();
    virtual ~WaterTransparencySetting() {}

    WaterTransparencySetting *Get_Override()
    {
        return m_next == nullptr ? this : (WaterTransparencySetting*)Get_Final_Override();
    }

    static void Parse_Water_Transparency(INI *ini);

private:
    float m_transparentWaterDepth;
    float m_transparentWaterMinOpacity;
    RGBColor m_standingWaterColor;
    RGBColor m_radarWaterColor;
    bool m_additiveBlending;
    AsciiString m_standingWaterTexture;
    AsciiString m_skyboxTextureN;
    AsciiString m_skyboxTextureE;
    AsciiString m_skyboxTextureS;
    AsciiString m_skyboxTextureW;
    AsciiString m_skyboxTextureT;

    static FieldParse m_waterTransparencySettingFieldParseTable[];
};

#ifdef THYME_STANDALONE
extern WaterSetting g_waterSettings[TIME_OF_DAY_COUNT];
extern WaterTrasparencySetting *g_theWaterTransparency;
#else
#define g_waterSettings (Make_Pointer<WaterSetting>(0x00A2F0B8))
#define g_theWaterTransparency (Make_Global<WaterTransparencySetting*>(0x00A2F0B0))
#endif

#endif // WATER_H
