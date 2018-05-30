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
#include "particleinfo.h"
#include "xfer.h"

ParticleInfo::ParticleInfo() :
    m_vel(),
    m_pos(),
    m_emitterPos(),
    m_velDamping(0.0f),
#ifdef THYME_STANDALONE
    m_angleX(0.0f),
    m_angleY(0.0f),
#endif
    m_angleZ(0.0f),
#ifdef THYME_STANDALONE
    m_angularRateX(0.0f),
    m_angularRateY(0.0f),
#endif
    m_angularRateZ(0.0f),
    m_angularDamping(0.0f),
    m_lifetime(0),
    m_size(0.0f),
    m_sizeRate(0.0f),
    m_sizeRateDamping(0.0f),
    m_colorScale(0.0f),
    m_windRandomness(0.0f),
    m_particleUpTowardsEmitter(false)
{
}

void ParticleInfo::Xfer_Snapshot(Xfer *xfer)
{
#define PARTICLEINFO_XFER_VERSION 1
    uint8_t version = PARTICLEINFO_XFER_VERSION;
    xfer->xferVersion(&version, PARTICLEINFO_XFER_VERSION);
    xfer->xferCoord3D(&m_vel);
    xfer->xferCoord3D(&m_pos);
    xfer->xferCoord3D(&m_emitterPos);
    xfer->xferReal(&m_velDamping);
#ifdef THYME_STANDALONE
    xfer->xferReal(&m_angleX);
    xfer->xferReal(&m_angleY);
#else
    float fake; // Used to keep version number after angleX and angleY members removed.
    xfer->xferReal(&fake); // angleX in Generals.
    xfer->xferReal(&fake); // angleY in Generals.
#endif
    xfer->xferReal(&m_angleZ);
#ifdef THYME_STANDALONE
    xfer->xferReal(&m_angularRateX);
    xfer->xferReal(&m_angularRateY);
#else
    xfer->xferReal(&fake); // angularRateX in Generals.
    xfer->xferReal(&fake); // angularRateY in Generals.
#endif
    xfer->xferReal(&m_angularRateZ);
    xfer->xferUnsignedInt(&m_lifetime);
    xfer->xferReal(&m_size);
    xfer->xferReal(&m_sizeRate);
    xfer->xferReal(&m_sizeRateDamping);

    for (int i = 0; i < KEYFRAME_COUNT; ++i) {
        xfer->xferReal(&m_alphaKey[i].value);
        xfer->xferUnsignedInt(&m_alphaKey[i].frame);
    }

    for (int i = 0; i < KEYFRAME_COUNT; ++i) {
        xfer->xferRGBColor(&m_colorKey[i].color);
        xfer->xferUnsignedInt(&m_colorKey[i].frame);
    }

    xfer->xferReal(&m_colorScale);
    xfer->xferBool(&m_particleUpTowardsEmitter);
    xfer->xferReal(&m_windRandomness);
}
