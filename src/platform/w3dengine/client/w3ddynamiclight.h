/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Dynamic Light
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
#include "light.h"

class W3DDynamicLight : public LightClass
{
public:
    W3DDynamicLight() : LightClass(LightClass::POINT), m_priorEnable(false), m_enabled(true) {}
    virtual ~W3DDynamicLight() override {}
    virtual void On_Frame_Update() override;

    void Set_Frame_Fade(unsigned int frame_increase_time, unsigned int decay_frame_time);

    bool Is_Enabled() { return m_enabled; }
    void Set_Enabled(bool enabled)
    {
        m_enabled = enabled;
        m_decayRange = false;
        m_decayFrameCount = false;
        m_decayColor = false;
        m_increaseFrameCount = 0;
    }

    void Set_Decay_Range() { m_decayRange = true; }
    void Set_Decay_Color() { m_decayColor = true; }

private:
    bool m_priorEnable;
    bool m_processMe;
    int m_prevMinX;
    int m_prevMinY;
    int m_prevMaxX;
    int m_prevMaxY;
    int m_minX;
    int m_minY;
    int m_maxX;
    int m_maxY;
    bool m_enabled;
    bool m_decayRange;
    bool m_decayColor;
    int m_curDecayFrameCount;
    int m_curIncreaseFrameCount;
    int m_decayFrameCount;
    int m_increaseFrameCount;
    float m_targetRange;
    Vector3 m_targetAmbient;
    Vector3 m_targetDiffuse;
};
