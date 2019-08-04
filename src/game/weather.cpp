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
#include "weather.h"

#ifdef GAME_DLL
WeatherSetting *&g_theWeatherSetting = Make_Global<WeatherSetting *>(0x00A2BF00);
#else
WeatherSetting *g_theWeatherSetting;
#endif

FieldParse WeatherSetting::s_weatherSettingParseTable[] = {
    {"SnowTexture", &INI::Parse_AsciiString, nullptr, 12},
    {"SnowFrequencyScaleX", &INI::Parse_Real, nullptr, 16},
    {"SnowFrequencyScaleY", &INI::Parse_Real, nullptr, 20},
    {"SnowAmplitude", &INI::Parse_Real, nullptr, 24},
    {"SnowPointSize", &INI::Parse_Real, nullptr, 28},
    {"SnowMaxPointSize", &INI::Parse_Real, nullptr, 32},
    {"SnowMinPointSize", &INI::Parse_Real, nullptr, 36},
    {"SnowQuadSize", &INI::Parse_Real, nullptr, 40},
    {"SnowBoxDimensions", &INI::Parse_Real, nullptr, 44},
    {"SnowBoxDensity", &INI::Parse_Real, nullptr, 48},
    {"SnowVelocity", &INI::Parse_Real, nullptr, 52},
    {"SnowPointSprites", &INI::Parse_Bool, nullptr, 56},
    {"SnowEnabled", &INI::Parse_Bool, nullptr, 57},
    {nullptr, nullptr, nullptr, 0}
};

WeatherSetting::WeatherSetting() :
    m_snowTexture("EXSnowFlake.tga"),
    m_snowFreqScaleX(0.0533f),
    m_snowFreqScaleY(0.0275f),
    m_snowAmplitude(5.0f),
    m_snowPointSize(1.0f),
    m_snowMaxPointSize(64.0f),
    m_snowMinPointSize(0.0f),
    m_snowQuadSize(0.5f),
    m_snowBoxDimensions(200.0f),
    m_snowBoxDensity(1.0f),
    m_snowVelocity(4.0f),
    m_snowPointSprites(true),
    m_snowEnabled(false)
{
}

void WeatherSetting::Parse_Weather_Definition(INI *ini)
{
    if (g_theWeatherSetting == nullptr) {
        g_theWeatherSetting = new WeatherSetting;
    } else {
        DEBUG_ASSERT_THROW(ini->Get_Load_Type() == INI_LOAD_CREATE_OVERRIDES, 0xDEAD0006, "g_theWeatherSetting is not null, but m_loadType is not INI_LOAD_CREATE_OVERRIDES.\n");
        WeatherSetting *new_ws = new WeatherSetting;
        *new_ws = *g_theWeatherSetting;
        new_ws->m_isAllocated = true;
        g_theWeatherSetting->Add_Override(new_ws);
    }

    ini->Init_From_INI(g_theWeatherSetting->Get_Override(), s_weatherSettingParseTable);

    if (ini->Get_Load_Type() == INI_LOAD_CREATE_OVERRIDES) {
        if (g_theWeatherSetting != g_theWeatherSetting->Get_Override()) {
            // TODO requires SnowManager virtual table layout implementing.
        }
    }
}
