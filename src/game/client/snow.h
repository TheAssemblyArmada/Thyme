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
#pragma once

#include "always.h"
#include "subsysteminterface.h"

class SnowManager : SubsystemInterface
{
public:
    enum
    {
        DIMENSION_BUFFER_RES = 64,
        DIMENSION_BUFFER_SIZE = DIMENSION_BUFFER_RES * DIMENSION_BUFFER_RES,
    };

    SnowManager();

    virtual ~SnowManager();
    virtual void Init() override;
    virtual void Reset() override;

    virtual void Update_INI_Settings();

    void Set_Visible(bool state);

    static int Get_Dimension_Buffer_Index(int x, int y)
    {
        // return ((((unsigned char)y - 96) & 63) << 6) + (((unsigned char)x - 96) & 63);
        return (((unsigned)x + 10000) % DIMENSION_BUFFER_RES)
            + ((((unsigned)y + 10000) % DIMENSION_BUFFER_RES) * DIMENSION_BUFFER_RES);
    }

protected:
    float *m_dimensionsBuffer;
    float m_elapsedTime;
    float m_velocity; // speed at which snow falls
    float m_lifetime;
    float m_freqScaleX; // speed of side-to-side wave movement
    float m_freqScaleY; // speed of side-to-side wave movement
    float m_amplitude; // amount of side-to-side movement
    float m_pointSize; // scale the size of snow particles
    float m_maxPointSize; // maximum pixel size of point sprite particles (min-spec requires <= 64)
    float m_minPointSize; // minimum pixel size of point sprite particles
    float m_particleSize; // scale of snow particles when using point-sprite emulation
    float m_boxDimensions; // width/height of box around camera containing snow
    float m_boxDensity; // amount of snow particles per world-unit. Raise to increase particle count
    bool m_isVisible; // is it currently visible
};