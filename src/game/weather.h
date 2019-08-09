/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Weather control device.
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
#include "ini.h"
#include "overridable.h"

class WeatherSetting : public Overridable
{
    IMPLEMENT_POOL(WeatherSetting);
public:
    WeatherSetting();
    virtual ~WeatherSetting() {}

    WeatherSetting *Get_Override()
    {
        return m_next == nullptr ? this : (WeatherSetting*)Get_Final_Override();
    }

    static void Parse_Weather_Definition(INI *ini);

private:
    Utf8String m_snowTexture;
    float m_snowFreqScaleX;
    float m_snowFreqScaleY;
    float m_snowAmplitude;
    float m_snowPointSize;
    float m_snowMaxPointSize;
    float m_snowMinPointSize;
    float m_snowQuadSize;
    float m_snowBoxDimensions;
    float m_snowBoxDensity;
    float m_snowVelocity;
    bool m_snowPointSprites;
    bool m_snowEnabled;

    static FieldParse s_weatherSettingParseTable[];
};

#ifdef GAME_DLL
extern WeatherSetting *&g_theWeatherSetting;
#else
extern WeatherSetting *g_theWeatherSetting;
#endif
