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
#include "weather.h"

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
    int dimensions = (int)g_theWeatherSetting->m_snowBoxDimensions;
    for (int i = 0; i < DIMENSION_BUFFER_RES; ++i) {
        for (int j = 0; j < DIMENSION_BUFFER_RES; ++j) {
            *buffer = (double)(rand() % dimensions);
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
