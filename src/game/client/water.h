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
#pragma once

#include "always.h"
#include "asciistring.h"
#include "color.h"
#include "gametype.h"
#include "ini.h"
#include "overridable.h"

class WaterSetting
{
public:
    virtual ~WaterSetting() {}

    static void Parse_Water_Setting_Definition(INI *ini);

private:
    Utf8String m_skyTextureFile;
    Utf8String m_waterTextureFile;
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

    static const FieldParse m_waterSettingFieldParseTable[];
    friend class WaterRenderObjClass;
};

class WaterTransparencySetting : public Overridable
{
    IMPLEMENT_POOL(WaterTransparencySetting);

protected:
    virtual ~WaterTransparencySetting() override {}

public:
    WaterTransparencySetting();

    static void Parse_Water_Transparency_Definition(INI *ini);

private:
    float m_transparentWaterDepth;
    float m_transparentWaterMinOpacity;
    RGBColor m_standingWaterColor;
    RGBColor m_radarWaterColor;
    bool m_additiveBlending;
    Utf8String m_standingWaterTexture;
    Utf8String m_skyboxTextureN;
    Utf8String m_skyboxTextureE;
    Utf8String m_skyboxTextureS;
    Utf8String m_skyboxTextureW;
    Utf8String m_skyboxTextureT;

    static const FieldParse m_waterTransparencySettingFieldParseTable[];
    friend class W3DShaderManager;
    friend class WaterRenderObjClass;
    friend class WaterTracksRenderSystem;
    friend class BaseHeightMapRenderObjClass;
    friend class W3DDisplay;
    friend class W3DView;
};

#ifdef GAME_DLL
extern ARRAY_DEC(WaterSetting, g_waterSettings, TIME_OF_DAY_COUNT);
extern Override<WaterTransparencySetting> &g_theWaterTransparency;
#else
extern WaterSetting g_waterSettings[TIME_OF_DAY_COUNT];
extern Override<WaterTransparencySetting> g_theWaterTransparency;
#endif