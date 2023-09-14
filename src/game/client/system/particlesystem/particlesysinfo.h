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
#pragma once

#include "always.h"
#include "asciistring.h"
#include "color.h"
#include "coord.h"
#include "gametype.h"
#include "randomvalue.h"
#include "snapshot.h"

#ifdef GAME_DLL
#include <new>
#endif

class ParticleSystemInfo : public SnapShot
{
    friend class ParticleSystemManager;
    friend class Particle;

protected:
    enum
    {
        KEYFRAME_COUNT = 8,
    };

    enum EmissionVelocityType : int32_t
    {
        EMISSION_VELOCITY_NONE,
        EMISSION_VELOCITY_ORTHO,
        EMISSION_VELOCITY_SPHERICAL,
        EMISSION_VELOCITY_HEMISPHERICAL,
        EMISSION_VELOCITY_CYLINDRICAL,
        EMISSION_VELOCITY_OUTWARD,
    };

    union EmissionVelocityUnion
    {
        EmissionVelocityUnion() {}
        EmissionVelocityUnion &operator=(const EmissionVelocityUnion &that)
        {
            if (this != &that) {
                memcpy(this, &that, sizeof(that));
            }

            return *this;
        }

        struct
        {
            GameClientRandomVariable x;
            GameClientRandomVariable y;
            GameClientRandomVariable z;
        } ortho;
        GameClientRandomVariable spherical;
        GameClientRandomVariable hemispherical;
        struct
        {
            GameClientRandomVariable radial;
            GameClientRandomVariable normal;
        } cylindrical;
        struct
        {
            GameClientRandomVariable outward;
            GameClientRandomVariable other;
        } outward;
    };

    enum EmissionVolumeType : int32_t
    {
        EMISSION_VOLUME_NONE,
        EMISSION_VOLUME_POINT,
        EMISSION_VOLUME_LINE,
        EMISSION_VOLUME_BOX,
        EMISSION_VOLUME_SPHERE,
        EMISSION_VOLUME_CYLINDER,
    };

    union EmissionVolumeUnion
    {
        EmissionVolumeUnion() {}

        EmissionVolumeUnion &operator=(const EmissionVolumeUnion &that)
        {
            if (this != &that) {
                memcpy(this, &that, sizeof(that));
            }

            return *this;
        }

        struct
        {
            Coord3D start;
            Coord3D end;
        } line;
        Coord3D box;
        float sphere;
        struct
        {
            float radius;
            float length;
        } cylinder;
    };

    enum WindMotion : int32_t
    {
        WIND_MOTION_NONE,
        WIND_MOTION_UNUSED,
        WIND_MOTION_PING_PONG,
        WIND_MOTION_CIRCULAR,
    };

    enum ParticleShaderType : int32_t
    {
        PARTICLE_SHADER_NONE,
        PARTICLE_SHADER_ADDITIVE,
        PARTICLE_SHADER_ALPHA,
        PARTICLE_SHADER_ALPHA_TEST,
        PARTICLE_SHADER_MULTIPLY,
    };

    enum ParticleType : int32_t
    {
        PARTICLE_TYPE_NONE,
        PARTICLE_TYPE_PARTICLE,
        PARTICLE_TYPE_DRAWABLE,
        PARTICLE_TYPE_STREAK,
        PARTICLE_TYPE_VOLUME_PARTICLE,
        PARTICLE_TYPE_SMUDGE,
    };

    struct RandomKeyframe
    {
        GameClientRandomVariable var;
        uint32_t frame;
    };

public:
    ParticleSystemInfo();
    ParticleSystemInfo(const ParticleSystemInfo &that) = delete;

    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    void Tint_All_Colors(int tint);

#ifdef GAME_DLL
    ParticleSystemInfo *Hook_Ctor() { return new (this) ParticleSystemInfo; }
#endif

protected:
    bool m_isOneShot;
    ParticleShaderType m_shaderType;
    ParticleType m_particleType;
    Utf8String m_particleTypeName;
#ifndef GAME_DLL
    GameClientRandomVariable m_angleX;
    GameClientRandomVariable m_angleY;
#endif
    GameClientRandomVariable m_angleZ;
#ifndef GAME_DLL
    GameClientRandomVariable m_angularRateX;
    GameClientRandomVariable m_angularRateY;
#endif
    GameClientRandomVariable m_angularRateZ;
    GameClientRandomVariable m_angularDamping;
    GameClientRandomVariable m_velDamping;
    GameClientRandomVariable m_lifetime;
    uint32_t m_systemLifetime;
    GameClientRandomVariable m_startSize;
    GameClientRandomVariable m_startSizeRate;
    GameClientRandomVariable m_sizeRate;
    GameClientRandomVariable m_sizeRateDamping;
    int32_t m_unkValue;
    RandomKeyframe m_alphaKey[KEYFRAME_COUNT];
    RGBColorKeyframe m_colorKey[KEYFRAME_COUNT];
    GameClientRandomVariable m_colorScale;
    GameClientRandomVariable m_burstDelay;
    GameClientRandomVariable m_burstCount;
    GameClientRandomVariable m_initialDelay;
    Coord3D m_driftVelocity;
    float m_gravity;
    Utf8String m_slaveSystemName;
    Coord3D m_slavePosOffset;
    Utf8String m_attachedSystemName;
    EmissionVelocityType m_emissionVelocityType;
    ParticlePriorityType m_priority;
    EmissionVelocityUnion m_emissionVelocity;
    EmissionVolumeType m_emissionVolumeType;
    EmissionVolumeUnion m_emissionVolume;
    bool m_isEmissionVolumeHollow;
    bool m_isGroundAligned;
    bool m_isEmitAboveGroundOnly;
    bool m_isParticleUpTowardsEmitter;
    WindMotion m_windMotion;
    float m_windAngle;
    float m_windAngleChange;
    float m_windAngleChangeMin;
    float m_windAngleChangeMax;
    float m_windMotionStartAngle;
    float m_windMotionStartAngleMin;
    float m_windMotionStartAngleMax;
    float m_windMotionEndAngle;
    float m_windMotionEndAngleMin;
    float m_windMotionEndAngleMax;
    bool m_windMotionMovingToEndAngle;
    friend class ScriptEngine;
    friend class W3DParticleSystemManager;
};

// no evidence this function exists so we can't be sure its accurate
#if 0
inline ParticleSystemInfo::ParticleSystemInfo(const ParticleSystemInfo &that) :
    m_isOneShot(that.m_isOneShot),
    m_shaderType(that.m_shaderType),
    m_particleType(that.m_particleType),
    m_particleTypeName(that.m_particleTypeName),
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
    m_velDamping(that.m_velDamping),
    m_lifetime(that.m_lifetime),
    m_systemLifetime(that.m_systemLifetime),
    m_startSize(that.m_startSize),
    m_startSizeRate(that.m_startSizeRate),
    m_sizeRate(that.m_sizeRate),
    m_sizeRateDamping(that.m_sizeRateDamping),
    m_unkValue(that.m_unkValue),
    m_colorScale(that.m_colorScale),
    m_burstDelay(that.m_burstDelay),
    m_burstCount(that.m_burstCount),
    m_initialDelay(that.m_initialDelay),
    m_driftVelocity(that.m_driftVelocity),
    m_gravity(that.m_gravity),
    m_slaveSystemName(that.m_slaveSystemName),
    m_slavePosOffset(that.m_slavePosOffset),
    m_attachedSystemName(that.m_attachedSystemName),
    m_emissionVelocityType(that.m_emissionVelocityType),
    m_priority(that.m_priority),
    m_emissionVelocity(that.m_emissionVelocity),
    m_emissionVolumeType(that.m_emissionVolumeType),
    m_emissionVolume(that.m_emissionVolume),
    m_isEmissionVolumeHollow(that.m_isEmissionVolumeHollow),
    m_isGroundAligned(that.m_isGroundAligned),
    m_isEmitAboveGroundOnly(that.m_isEmitAboveGroundOnly),
    m_isParticleUpTowardsEmitter(that.m_isParticleUpTowardsEmitter),
    m_windMotion(that.m_windMotion),
    m_windAngle(that.m_windAngle),
    m_windAngleChange(that.m_windAngleChange),
    m_windAngleChangeMin(that.m_windAngleChangeMin),
    m_windAngleChangeMax(that.m_windAngleChangeMax),
    m_windMotionStartAngle(that.m_windMotionStartAngle),
    m_windMotionStartAngleMin(that.m_windMotionStartAngleMin),
    m_windMotionStartAngleMax(that.m_windMotionStartAngleMax),
    m_windMotionEndAngle(that.m_windMotionEndAngle),
    m_windMotionEndAngleMin(that.m_windMotionEndAngleMin),
    m_windMotionEndAngleMax(that.m_windMotionEndAngleMax),
    m_windMotionMovingToEndAngle(that.m_windMotionMovingToEndAngle)
{
    for (int i = 0; i < KEYFRAME_COUNT; ++i) {
        m_alphaKey[i] = that.m_alphaKey[i];
        m_colorKey[i] = that.m_colorKey[i];
    }
}
#endif
