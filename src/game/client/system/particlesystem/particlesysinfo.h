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

class ParticleSystemInfo : public SnapShot
{
    enum
    {
        KEYFRAME_COUNT = 8,
    };

    struct RandomKeyframe
    {
        GameClientRandomVariable var;
        uint32_t frame;
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

public:
    ParticleSystemInfo();

    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    void Tint_All_Colors(int tint);

protected:
    bool m_isOneShot;
    ParticleShaderType m_shaderType;
    ParticleType m_particleType;
    AsciiString m_particleTypeName;
    GameClientRandomVariable m_angleZ;
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
    AsciiString m_slaveSystemName;
    Coord3D m_slavePosOffset;
    AsciiString m_attachedSystemName;
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
};
