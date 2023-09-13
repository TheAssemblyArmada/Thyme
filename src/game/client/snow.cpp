/**
 * @file
 *
 * @author tomsons26
 *
 * @brief Implements interface for drawing snow/rain effects.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */

#include "snow.h"

#ifndef GAME_DLL
SnowManager *g_theSnowManager;
Override<WeatherSetting> g_theWeatherSetting;
#endif

SnowManager::SnowManager() :
    m_dimensionsBuffer(nullptr),
    m_elapsedTime(0.0f),
    m_velocity(0.0f),
    m_lifetime(0.0f),
    m_freqScaleX(0.0f),
    m_freqScaleY(0.0f),
    m_amplitude(0.0f),
    m_pointSize(0.0f),
    m_maxPointSize(0.0f),
    m_minPointSize(0.0f),
    m_particleSize(0.0f),
    m_boxDimensions(0.0f),
    m_boxDensity(0.0f),
    m_isVisible(true)
{
}

SnowManager::~SnowManager()
{
    delete[] m_dimensionsBuffer;
    m_dimensionsBuffer = nullptr;
}

void SnowManager::Init()
{
    m_dimensionsBuffer = new float[DIMENSION_BUFFER_SIZE];
    m_elapsedTime = 0.0f;
    Update_INI_Settings();
}

void SnowManager::Update_INI_Settings()
{
    float *buffer = m_dimensionsBuffer;
    int dimensions = g_theWeatherSetting->m_snowBoxDimensions;

    for (int i = 0; i < DIMENSION_BUFFER_RES; ++i) {
        for (int j = 0; j < DIMENSION_BUFFER_RES; ++j) {
            *buffer = (rand() % dimensions);
            ++buffer;
        }
    }

    m_velocity = g_theWeatherSetting->m_snowVelocity;
    m_freqScaleX = g_theWeatherSetting->m_snowFreqScaleX;
    m_freqScaleY = g_theWeatherSetting->m_snowFreqScaleY;
    m_amplitude = g_theWeatherSetting->m_snowAmplitude;
    m_pointSize = g_theWeatherSetting->m_snowPointSize;
    m_particleSize = g_theWeatherSetting->m_snowQuadSize;
    m_boxDimensions = g_theWeatherSetting->m_snowBoxDimensions;
    m_boxDensity = 1.0f / g_theWeatherSetting->m_snowBoxDensity;
    m_maxPointSize = g_theWeatherSetting->m_snowMaxPointSize;
    m_minPointSize = g_theWeatherSetting->m_snowMinPointSize;
    m_lifetime = m_boxDimensions / m_velocity;
}

void SnowManager::Set_Visible(bool state)
{
    m_isVisible = state;
}

void SnowManager::Reset()
{
    m_isVisible = true;
}

const FieldParse WeatherSetting::s_weatherSettingParseTable[] = {
    { "SnowTexture", &INI::Parse_AsciiString, nullptr, offsetof(WeatherSetting, m_snowTexture) },
    { "SnowFrequencyScaleX", &INI::Parse_Real, nullptr, offsetof(WeatherSetting, m_snowFreqScaleX) },
    { "SnowFrequencyScaleY", &INI::Parse_Real, nullptr, offsetof(WeatherSetting, m_snowFreqScaleY) },
    { "SnowAmplitude", &INI::Parse_Real, nullptr, offsetof(WeatherSetting, m_snowAmplitude) },
    { "SnowPointSize", &INI::Parse_Real, nullptr, offsetof(WeatherSetting, m_snowPointSize) },
    { "SnowMaxPointSize", &INI::Parse_Real, nullptr, offsetof(WeatherSetting, m_snowMaxPointSize) },
    { "SnowMinPointSize", &INI::Parse_Real, nullptr, offsetof(WeatherSetting, m_snowMinPointSize) },
    { "SnowQuadSize", &INI::Parse_Real, nullptr, offsetof(WeatherSetting, m_snowQuadSize) },
    { "SnowBoxDimensions", &INI::Parse_Real, nullptr, offsetof(WeatherSetting, m_snowBoxDimensions) },
    { "SnowBoxDensity", &INI::Parse_Real, nullptr, offsetof(WeatherSetting, m_snowBoxDensity) },
    { "SnowVelocity", &INI::Parse_Real, nullptr, offsetof(WeatherSetting, m_snowVelocity) },
    { "SnowPointSprites", &INI::Parse_Bool, nullptr, offsetof(WeatherSetting, m_snowPointSprites) },
    { "SnowEnabled", &INI::Parse_Bool, nullptr, offsetof(WeatherSetting, m_snowEnabled) },
    { nullptr, nullptr, nullptr, 0 }
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

// Was originally INI::parseWeatherDefinition
void WeatherSetting::Parse_Weather_Definition(INI *ini)
{
    if (*g_theWeatherSetting) {
        if (ini->Get_Load_Type() == INI_LOAD_CREATE_OVERRIDES) {
            WeatherSetting *old_ws = g_theWeatherSetting;
            WeatherSetting *new_ws = NEW_POOL_OBJ(WeatherSetting);
            *new_ws = *old_ws;
            new_ws->Set_Is_Allocated();
            old_ws->Friend_Get_Final_Override()->Set_Next(new_ws);
        } else {
            throw CODE_06;
        }
    } else {
        g_theWeatherSetting = NEW_POOL_OBJ(WeatherSetting);
    }

    Overridable *setting = g_theWeatherSetting;
    setting = setting->Friend_Get_Final_Override();
    ini->Init_From_INI(setting, s_weatherSettingParseTable);

    if (g_theSnowManager) {
        g_theSnowManager->Update_INI_Settings();
    }
}
