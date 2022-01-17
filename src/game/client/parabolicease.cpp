/**
 * @file
 *
 * @author xezon
 *
 * @brief Parabolic Ease class
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "parabolicease.h"
#include <algorithm>
#include <captainslog.h>

ParabolicEase::ParabolicEase(float in, float out)
{
    Set_Ease_Times(in, out);
}

void ParabolicEase::Set_Ease_Times(float in, float out)
{
    m_easeIn = in;

    if (m_easeIn < 0.0f || m_easeIn > 1.0f) {
        captainslog_dbgassert(0, "Ease-in out of range (in = %g)", m_easeIn);
        m_easeIn = std::clamp(m_easeIn, 0.0f, 1.0f);
    }

    m_easeOut = 1.0f - out;

    if (m_easeOut < 0.0f || m_easeOut > 1.0f) {
        captainslog_dbgassert(0, "Ease-out out of range (out = %g)", m_easeOut);
        m_easeOut = std::clamp(m_easeOut, 0.0f, 1.0f);
    }

    if (m_easeIn > m_easeOut) {
        captainslog_dbgassert(0, "Ease-in and ease-out overlap (in = %g, out = %g)", m_easeIn, m_easeOut);
        m_easeIn = m_easeOut;
    }
}

float ParabolicEase::operator()(float param)
{
    if (param < 0.0f || param > 1.0f) {
        captainslog_dbgassert(0, "Ease-in/ease-out parameter out of range (param = %g)", param);
        param = std::clamp(param, 0.0f, 1.0f);
    }

    float scale = m_easeOut + 1.0f - m_easeIn;

    if (param < m_easeIn) {
        return (param * param) / (scale * m_easeIn);
    }

    if (param <= m_easeOut) {
        return (m_easeIn + param - m_easeIn + param - m_easeIn) / scale;
    }

    float d = m_easeOut - m_easeIn;
    float b = m_easeIn + d + d;
    float r = param - m_easeOut;
    return (b + (r + r + m_easeOut * m_easeOut - param * param) / (1.0f - m_easeOut)) / scale;
}
