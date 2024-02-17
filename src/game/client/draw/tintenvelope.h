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
#pragma once

#include "always.h"
#include "mempoolobj.h"
#include "snapshot.h"
#include "vector3.h"

class RGBColor;

class TintEnvelope : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_POOL(TintEnvelope);

public:
    enum TintState : int8_t
    {
        STATE_IDLE,
        STATE_ATTACK, // Fade in from idle to peak color.
        STATE_DECAY, // Fade out from peak to idle color.
        STATE_PEAK, // Tint peak color reached.
    };

    TintEnvelope();
    virtual ~TintEnvelope() override {}

    void CRC_Snapshot(Xfer *xfer) override {}
    void Xfer_Snapshot(Xfer *xfer) override;
    void Load_Post_Process() override {}

    void Play(RGBColor const *peak_color, unsigned int attack_frames, unsigned int decay_frames, unsigned int peak_frames);

    void Update();

    void Set_Idle_State() { m_state = STATE_IDLE; }
    void Set_Decay_State() { m_state = STATE_DECAY; }
    bool Is_Tinted() { return m_isTinted; }
    Vector3 *Get_Tint_Color() { return &m_tintColor; }

private:
    void Set_Attack_Frames(unsigned int attack_frames);
    void Set_Decay_Frames(unsigned int decay_frames);
    void Set_Peak_Color(RGBColor const *color);

private:
    Vector3 m_attackColor;
    Vector3 m_decayColor;
    Vector3 m_peakColor;
    Vector3 m_tintColor;
    unsigned int m_peakWaitFrames;
    TintState m_state;
    bool m_isTinted;
};
