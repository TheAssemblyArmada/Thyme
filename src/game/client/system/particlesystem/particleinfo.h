/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Base class for particles.
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
#include "color.h"
#include "coord.h"
#include "snapshot.h"

struct Keyframe
{
    float value;
    uint32_t frame;
};

class ParticleInfo : public SnapShot
{
    enum
    {
        KEYFRAME_COUNT = 8,
    };
public:
    ParticleInfo();

    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

protected:
    Coord3D m_vel;
    Coord3D m_pos;
    Coord3D m_emitterPos;
    float m_velDamping;
#ifdef THYME_STANDALONE
    float m_angleX;
    float m_angleY;
#endif
    float m_angleZ;
#ifdef THYME_STANDALONE
    float m_angularRateX;
    float m_angularRateY;
#endif
    float m_angularRateZ;
    float m_angularDamping;
    uint32_t m_lifetime;
    float m_size;
    float m_sizeRate;
    float m_sizeRateDamping;
    Keyframe m_alphaKey[KEYFRAME_COUNT];
    RGBColorKeyframe m_colorKey[KEYFRAME_COUNT];
    float m_colorScale;
    float m_windRandomness;
    bool m_particleUpTowardsEmitter;
};
