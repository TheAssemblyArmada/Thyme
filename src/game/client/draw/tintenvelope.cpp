/**
 * @file
 *
 * @author tomsons26
 *
 * @brief
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "tintenvelope.h"
#include "color.h"
#include "xfer.h"

TintEnvelope::TintEnvelope() :
    m_attackColor(0.0f, 0.0f, 0.0f),
    m_decayColor(0.0f, 0.0f, 0.0f),
    m_peakColor(0.0f, 0.0f, 0.0f),
    m_tintColor(0.0f, 0.0f, 0.0f),
    m_peakWaitFrames(0),
    m_state(STATE_IDLE),
    m_isTinted(false)
{
}

void TintEnvelope::Play(
    RGBColor const *peak_color, unsigned int attack_frames, unsigned int decay_frames, unsigned int peak_frames)
{
    Set_Peak_Color(peak_color);

    Set_Attack_Frames(attack_frames);
    Set_Decay_Frames(decay_frames);

    m_state = STATE_ATTACK;

    m_peakWaitFrames = peak_frames;

    m_isTinted = true;

    Vector3 delta;
    Vector3::Subtract(m_tintColor, m_peakColor, &delta);
    if (delta.Length() <= 0.001f) {
        m_state = STATE_PEAK;
    }
}

void TintEnvelope::Set_Attack_Frames(unsigned int attack_frames)
{
    if (attack_frames < 1) {
        attack_frames = 1;
    }

    float f1 = 1.0f / (float)attack_frames;

    m_attackColor = m_tintColor;

    Vector3::Subtract(m_peakColor, m_attackColor, &m_attackColor);

    m_attackColor.Scale(Vector3(f1, f1, f1));
}

void TintEnvelope::Set_Decay_Frames(unsigned int decay_frames)
{
    if (decay_frames < 1) {
        decay_frames = 1;
    }

    float f1 = -1.0f / (float)decay_frames;

    m_decayColor = m_peakColor;

    m_decayColor.Scale(Vector3(f1, f1, f1));
}

void TintEnvelope::Update()
{
    switch (m_state) {

        case STATE_IDLE: {
            m_tintColor.Set(0.0f, 0.0f, 0.0f);
            m_isTinted = false;
            break;
        }

        case STATE_ATTACK: {
            Vector3 v1;
            Vector3::Subtract(m_tintColor, m_peakColor, &v1);

            if (v1.Length() >= m_attackColor.Length() && v1.Length() > 0.001f) {
                Vector3::Add(m_attackColor, m_tintColor, &m_tintColor);
                m_isTinted = true;
            } else if (m_peakWaitFrames != 0) {
                m_state = STATE_PEAK;
            } else {
                m_state = STATE_DECAY;
            }
            break;
        }

        case STATE_DECAY: {
            if (m_tintColor.Length() >= m_decayColor.Length() && m_tintColor.Length() > 0.001f) {
                Vector3::Add(m_decayColor, m_tintColor, &m_tintColor);
                m_isTinted = true;
            } else {
                m_state = STATE_IDLE;
                m_isTinted = false;
            }
            break;
        }

        case STATE_PEAK: {
            if (m_peakWaitFrames > 0) {
                --m_peakWaitFrames;
            } else {
                m_state = STATE_DECAY;
            }
            break;
        }

        default:
            break;
    }
}

void TintEnvelope::Set_Peak_Color(RGBColor const *color)
{
    m_peakColor = Vector3(color->red, color->green, color->blue);
}

void TintEnvelope::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t version = 1;
    xfer->xferVersion(&version, 1);

    xfer->xferUser(&m_attackColor, sizeof(m_attackColor));
    xfer->xferUser(&m_decayColor, sizeof(m_decayColor));
    xfer->xferUser(&m_peakColor, sizeof(m_peakColor));
    xfer->xferUser(&m_tintColor, sizeof(m_tintColor));

    xfer->xferUnsignedInt(&m_peakWaitFrames);

    xfer->xferBool(&m_isTinted);

    static_assert(sizeof(TintState) == sizeof(int8_t));

    xfer->xferByte(reinterpret_cast<int8_t *>(&m_state));
}
