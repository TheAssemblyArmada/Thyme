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
    friend class ParticleSystem;
    friend class Particle;

    enum
    {
        KEYFRAME_COUNT = 8,
    };

public:
    ParticleInfo();
    ParticleInfo(const ParticleInfo &that);

    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

protected:
    Coord3D m_vel;
    Coord3D m_pos;
    Coord3D m_emitterPos;
    float m_velDamping;
#ifndef GAME_DLL
    float m_angleX;
    float m_angleY;
#endif
    float m_angleZ;
#ifndef GAME_DLL
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

inline ParticleInfo::ParticleInfo(const ParticleInfo &that) :
    m_vel(that.m_vel),
    m_pos(that.m_pos),
    m_emitterPos(that.m_emitterPos),
    m_velDamping(that.m_velDamping),
#ifndef GAME_DLL
    m_angleX(that.m_angleX),
    m_angleY(that.m_angleY),
#endif
    m_angleZ(that.m_angleZ),
#ifndef GAME_DLL
    m_angularRateX(that.m_angularRateX),
    m_angularRateY(that.m_angularRateY),
#endif
    m_angularRateZ(that.m_angularRateZ),
    m_angularDamping(that.m_angularDamping),
    m_lifetime(that.m_lifetime),
    m_size(that.m_size),
    m_sizeRate(that.m_sizeRate),
    m_sizeRateDamping(that.m_sizeRateDamping),
    m_colorScale(that.m_colorScale),
    m_windRandomness(that.m_windRandomness),
    m_particleUpTowardsEmitter(that.m_particleUpTowardsEmitter)
{
    for (int i = 0; i < KEYFRAME_COUNT; ++i) {
        m_alphaKey[i] = that.m_alphaKey[i];
        m_colorKey[i] = that.m_colorKey[i];
    }
}
