/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Base class for particle systems containing common info.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "particlesysinfo.h"
#include "xfer.h"

/**
 * 0x004CD460;
 */
ParticleSystemInfo::ParticleSystemInfo() :
    m_isOneShot(false),
    m_shaderType(PARTICLE_SHADER_NONE),
    m_particleType(PARTICLE_TYPE_NONE),
    m_particleTypeName(),
#ifndef GAME_DLL
    m_angleX(),
    m_angleY(),
#endif
    m_angleZ(),
#ifndef GAME_DLL
    m_angularRateX(),
    m_angularRateY(),
#endif
    m_angularRateZ(),
    m_angularDamping(),
    m_velDamping(),
    m_lifetime(),
    m_systemLifetime(0),
    m_startSize(),
    m_startSizeRate(),
    m_sizeRate(),
    m_sizeRateDamping(),
    m_unkValue(0),
    m_colorScale(),
    m_burstDelay(),
    m_burstCount(),
    m_initialDelay(),
    m_driftVelocity(),
    m_gravity(0.0f),
    m_slaveSystemName(),
    m_slavePosOffset(),
    m_attachedSystemName(),
    m_emissionVelocityType(EMISSION_VELOCITY_NONE),
    m_priority(),
    m_emissionVolumeType(EMISSION_VOLUME_NONE),
    m_isEmissionVolumeHollow(false),
    m_isGroundAligned(false),
    m_isEmitAboveGroundOnly(false),
    m_isParticleUpTowardsEmitter(false),
    m_windMotion(WIND_MOTION_UNUSED),
    m_windAngle(0.000000f),
    m_windAngleChange(0.150000f),
    m_windAngleChangeMin(0.150000f),
    m_windAngleChangeMax(0.450000f),
    m_windMotionStartAngle(0.000000f),
    m_windMotionStartAngleMin(0.000000f),
    m_windMotionStartAngleMax(0.785398f),
    m_windMotionEndAngle(5.497787f),
    m_windMotionEndAngleMin(5.497787f),
    m_windMotionEndAngleMax(6.283185f),
    m_windMotionMovingToEndAngle(true)
{
    m_driftVelocity.Zero();
    m_slavePosOffset.Zero();

    // #BUGFIX Initialize all members
    for (int i = 0; i < KEYFRAME_COUNT; ++i) {
        m_alphaKey[i] = RandomKeyframe{};
        m_colorKey[i] = RGBColorKeyframe{};
    }
}

/**
 * @brief Transfers object data.
 *
 * 0x004CD5D0;
 */
void ParticleSystemInfo::Xfer_Snapshot(Xfer *xfer)
{
#define PARTICLESYS_XFER_VERSION 1
    uint8_t version = PARTICLESYS_XFER_VERSION;
    xfer->xferVersion(&version, PARTICLESYS_XFER_VERSION);
    xfer->xferBool(&m_isOneShot);
    xfer->xferInt(reinterpret_cast<int32_t *>(&m_shaderType)); // Original calls xferUser, endianness issues.
    xfer->xferInt(reinterpret_cast<int32_t *>(&m_particleType)); // Original calls xferUser, endianness issues.
    xfer->xferAsciiString(&m_particleTypeName);
#ifndef GAME_DLL
    xfer->Xfer_Client_Random_Var(&m_angleX);
    xfer->Xfer_Client_Random_Var(&m_angleY);
#else
    GameClientRandomVariable fake; // Used to keep version number after angleX and angleY members removed.
    xfer->Xfer_Client_Random_Var(
        &fake); // angleX in Generals. // These xfers used xferUser to transfer as opaque objects, but has endian issues.
    xfer->Xfer_Client_Random_Var(&fake); // angleY in Generals.
#endif
    xfer->Xfer_Client_Random_Var(&m_angleZ);
#ifndef GAME_DLL
    xfer->Xfer_Client_Random_Var(&m_angularRateX);
    xfer->Xfer_Client_Random_Var(&m_angularRateY);
#else
    xfer->Xfer_Client_Random_Var(&fake); // angularRateX in Generals.
    xfer->Xfer_Client_Random_Var(&fake); // angularRateY in Generals.
#endif
    xfer->Xfer_Client_Random_Var(&m_angularRateZ);
    xfer->Xfer_Client_Random_Var(&m_angularDamping);
    xfer->Xfer_Client_Random_Var(&m_velDamping);
    xfer->xferUnsignedInt(&m_systemLifetime);
    xfer->Xfer_Client_Random_Var(&m_startSize);
    xfer->Xfer_Client_Random_Var(&m_startSizeRate);
    xfer->Xfer_Client_Random_Var(&m_sizeRate);
    xfer->Xfer_Client_Random_Var(&m_sizeRateDamping);

    for (int i = 0; i < KEYFRAME_COUNT; ++i) {
        xfer->Xfer_Client_Random_Var(&m_alphaKey[i].var);
        xfer->xferUnsignedInt(&m_alphaKey[i].frame);
    }

    for (int i = 0; i < KEYFRAME_COUNT; ++i) {
        xfer->xferRGBColor(&m_colorKey[i].color);
        xfer->xferUnsignedInt(&m_colorKey[i].frame);
    }

    xfer->Xfer_Client_Random_Var(&m_colorScale);
    xfer->Xfer_Client_Random_Var(&m_burstDelay);
    xfer->Xfer_Client_Random_Var(&m_burstCount);
    xfer->Xfer_Client_Random_Var(&m_initialDelay);
    xfer->xferCoord3D(&m_driftVelocity);
    xfer->xferReal(&m_gravity);
    xfer->xferAsciiString(&m_slaveSystemName);
    xfer->xferCoord3D(&m_slavePosOffset);
    xfer->xferAsciiString(&m_attachedSystemName);
    xfer->xferInt(reinterpret_cast<int32_t *>(&m_emissionVelocityType));
    xfer->xferInt(reinterpret_cast<int32_t *>(&m_priority));

    switch (m_emissionVelocityType) {
        case EMISSION_VELOCITY_ORTHO:
            xfer->Xfer_Client_Random_Var(&m_emissionVelocity.ortho.x);
            xfer->Xfer_Client_Random_Var(&m_emissionVelocity.ortho.y);
            xfer->Xfer_Client_Random_Var(&m_emissionVelocity.ortho.z);
            break;
        case EMISSION_VELOCITY_SPHERICAL:
            xfer->Xfer_Client_Random_Var(&m_emissionVelocity.spherical);
            break;
        case EMISSION_VELOCITY_HEMISPHERICAL:
            xfer->Xfer_Client_Random_Var(&m_emissionVelocity.hemispherical);
            break;
        case EMISSION_VELOCITY_CYLINDRICAL:
            xfer->Xfer_Client_Random_Var(&m_emissionVelocity.cylindrical.radial);
            xfer->Xfer_Client_Random_Var(&m_emissionVelocity.cylindrical.normal);
            break;
        case EMISSION_VELOCITY_OUTWARD:
            xfer->Xfer_Client_Random_Var(&m_emissionVelocity.outward.outward);
            xfer->Xfer_Client_Random_Var(&m_emissionVelocity.outward.other);
            break;
        default:
            break;
    }

    xfer->xferInt(reinterpret_cast<int32_t *>(&m_emissionVolumeType));

    switch (m_emissionVolumeType) {
        case EMISSION_VOLUME_LINE:
            xfer->xferCoord3D(&m_emissionVolume.line.start);
            xfer->xferCoord3D(&m_emissionVolume.line.end);
            break;
        case EMISSION_VOLUME_BOX:
            xfer->xferCoord3D(&m_emissionVolume.box);
            break;
        case EMISSION_VOLUME_SPHERE:
            xfer->xferReal(&m_emissionVolume.sphere);
            break;
        case EMISSION_VOLUME_CYLINDER:
            xfer->xferReal(&m_emissionVolume.cylinder.radius);
            xfer->xferReal(&m_emissionVolume.cylinder.length);
            break;
        default:
            break;
    }

    xfer->xferBool(&m_isEmissionVolumeHollow);
    xfer->xferBool(&m_isGroundAligned);
    xfer->xferBool(&m_isEmitAboveGroundOnly);
    xfer->xferBool(&m_isParticleUpTowardsEmitter);
    xfer->xferInt(reinterpret_cast<int32_t *>(&m_windMotion));
    xfer->xferReal(&m_windAngle);
    xfer->xferReal(&m_windAngleChange);
    xfer->xferReal(&m_windAngleChangeMin);
    xfer->xferReal(&m_windAngleChangeMax);
    xfer->xferReal(&m_windMotionStartAngle);
    xfer->xferReal(&m_windMotionStartAngleMin);
    xfer->xferReal(&m_windMotionStartAngleMax);
    xfer->xferReal(&m_windMotionEndAngle);
    xfer->xferReal(&m_windMotionEndAngleMin);
    xfer->xferReal(&m_windMotionEndAngleMax);
    xfer->xferBool(&m_windMotionMovingToEndAngle); // TODO verify this is a bool and not a Byte as original does xferByte
#undef PARTICLESYS_XFER_VERSION
}

/**
 * @brief Tints all colors in the color key array. Tint is treated as 32bit RGBA data.
 *
 * 0x004CD540;
 */
void ParticleSystemInfo::Tint_All_Colors(int tint)
{
    for (int i = 1; i < KEYFRAME_COUNT; ++i) {
        m_colorKey[i].color.red *= float(float((tint >> 16) & 0xFF) / 256.0f);
        m_colorKey[i].color.green *= float(float((tint >> 8) & 0xFF) / 256.0f);
        m_colorKey[i].color.blue *= float(float(tint & 0xFF) / 256.0f);
    }
}
