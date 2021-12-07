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
#include "w3ddynamiclight.h"
void W3DDynamicLight::On_Frame_Update()
{
    if (m_enabled) {
        float k = 1.0f;

        if (m_curIncreaseFrameCount && m_increaseFrameCount) {
            m_curIncreaseFrameCount--;
            k = (float)(unsigned int)(m_increaseFrameCount - m_curIncreaseFrameCount) / (float)m_increaseFrameCount;
        } else if (!m_decayFrameCount) {
            k = 1.0f;
        } else {
            if (!--m_curDecayFrameCount) {
                m_enabled = false;
                return;
            }

            k = (float)(unsigned int)m_curDecayFrameCount / (float)m_decayFrameCount;
        }

        if (m_decayRange) {
            m_farAttenEnd = k * m_targetRange;

            if (m_farAttenEnd < m_farAttenStart) {
                m_farAttenEnd = m_farAttenStart;
            }
        }

        if (m_decayColor) {
            m_ambient = m_targetAmbient * k;
            m_diffuse = m_targetDiffuse * k;
        }
    }
}

void W3DDynamicLight::Set_Frame_Fade(unsigned int frame_increase_time, unsigned int decay_frame_time)
{
    m_decayFrameCount = decay_frame_time;
    m_curDecayFrameCount = decay_frame_time;
    m_curIncreaseFrameCount = frame_increase_time;
    m_increaseFrameCount = frame_increase_time;
    m_targetAmbient = m_ambient;
    m_targetDiffuse = m_diffuse;
    m_targetRange = m_farAttenEnd;
}
