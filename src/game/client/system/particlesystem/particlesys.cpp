/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Object holding a particle system instance.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "particlesys.h"
#include "particlesystemplate.h"

ParticleSystem::ParticleSystem(const ParticleSystemTemplate &temp, ParticleSystemID id, bool create_slaves) :
    m_systemParticlesHead(nullptr),
    m_systemParticlesTail(nullptr),
    m_particleCount(),
    m_systemID(id),
    m_attachedToDrawableID(0),
    m_attachedToObjectID(0),
    m_localTransform(true),
    m_transform(true),
    m_burstDelayLeft(),
    m_delayLeft(),
    m_startTimestamp(),
    m_systemLifetimeLeft(),
    m_nextParticleIDMaybe(),
    m_accumulatedSizeBonus(0),
    m_velCoefficient(1.0f, 1.0f, 1.0f),
    m_countCoefficient(1.0f),
    m_delayCoefficient(1.0f),
    m_sizeCoefficient(1.0f),
    m_pos(),
    m_lastPos(),
    m_slaveSystem(nullptr),
    m_slaveID(PARTSYS_ID_NONE),
    m_masterSystem(nullptr),
    m_masterID(PARTSYS_ID_NONE),
    m_template(&temp),
    m_controlParticle(),
    m_isLocalIdentity(true),
    m_isIdentity(true),
    m_isForever(false),
    m_isStopped(false),
    m_isDestroyed(false),
    m_isFirstPos(true),
    m_saveable(true),
    m_unkBool1(false)
{
    m_slavePosOffset = temp.m_slavePosOffset;
    m_unkValue = 0;
    m_driftVelocity = temp.m_driftVelocity;
    m_gravity = temp.m_gravity;
    m_lifetime = temp.m_lifetime;
    m_startSize = temp.m_startSize;
    m_startSizeRate = temp.m_startSizeRate;
    m_sizeRate = temp.m_sizeRate;
    m_sizeRateDamping = temp.m_sizeRateDamping;

    for (int i = 0; i < KEYFRAME_COUNT; ++i) {
        m_alphaKey[i] = temp.m_alphaKey[i];
        m_colorKey[i] = temp.m_colorKey[i];
    }

    m_colorScale.Set_Range(temp.m_colorScale.Get_Min() / 255.0f, temp.m_colorScale.Get_Max() / 255.0f, GameClientRandomVariable::UNIFORM);
    m_burstDelay = temp.m_burstDelay;
    m_burstDelayLeft = 0;
    m_burstCount = temp.m_burstCount;
    m_isOneShot = temp.m_isOneShot;
    m_delayLeft = m_initialDelay;
    /*m_startTimestamp = g_theGameClient->*/ // TODO Needs GameClient
    m_systemLifetimeLeft = temp.m_systemLifetime;
    m_isForever = m_systemLifetime == 0;
    m_velDamping = temp.m_velDamping;
#ifdef THYME_STANDALONE
    m_angleX = temp.m_angleX;
    m_angularRateX = temp.m_angularRateX;
    m_angleY = temp.m_angleY;
    m_angularRateY = temp.m_angularRateY;
#endif
    m_angleZ = temp.m_angleZ;
    m_angularRateZ = temp.m_angularRateZ;
    m_angularDamping = temp.m_angularDamping;
    m_priority = temp.m_priority;
    m_emissionVelocityType = temp.m_emissionVelocityType;
    memcpy(&m_emissionVelocity, &temp.m_emissionVelocity, sizeof(m_emissionVelocity));
    m_emissionVolumeType = temp.m_emissionVolumeType;
    memcpy(&m_emissionVolume, &temp.m_emissionVolume, sizeof(m_emissionVelocity));
    m_isEmissionVolumeHollow = temp.m_isEmissionVolumeHollow;
    m_isGroundAligned = temp.m_isGroundAligned;
    m_isEmitAboveGroundOnly = temp.m_isEmitAboveGroundOnly;
    m_isParticleUpTowardsEmitter = temp.m_isParticleUpTowardsEmitter;
    m_windMotion = temp.m_windMotion;
    m_windAngleChange = temp.m_windAngleChange;
    m_windAngleChangeMin = temp.m_windAngleChangeMin;
    m_windAngleChangeMax = temp.m_windAngleChangeMax;
    m_windMotionStartAngleMin = temp.m_windMotionStartAngleMin;
    m_windMotionStartAngleMax = temp.m_windMotionStartAngleMax;
    m_windMotionEndAngleMin = temp.m_windMotionEndAngleMin;
    m_windMotionEndAngleMax = temp.m_windMotionEndAngleMax;
    m_windMotionMovingToEndAngle = temp.m_windMotionMovingToEndAngle;
    m_windMotionStartAngle = Get_Client_Random_Value_Real(temp.m_windMotionStartAngleMin, temp.m_windMotionStartAngleMax);
    m_windMotionEndAngle = Get_Client_Random_Value_Real(temp.m_windMotionEndAngleMin, temp.m_windMotionEndAngleMax);
    m_windAngle = Get_Client_Random_Value_Real(m_windMotionStartAngle, m_windMotionEndAngle);
    m_shaderType = temp.m_shaderType;
    m_particleType = temp.m_particleType;
    m_particleTypeName = temp.m_particleTypeName;
}

ParticleSystem::~ParticleSystem()
{

}

void ParticleSystem::Xfer_Snapshot(Xfer *xfer)
{

}

void ParticleSystem::Load_Post_Process()
{

}

void ParticleSystem::Remove_Master()
{
    if (m_masterSystem != nullptr) {
        m_masterSystem->m_slaveSystem = nullptr;
        m_masterSystem->m_slaveID = PARTSYS_ID_NONE;
        m_masterSystem = nullptr;
        m_masterID = PARTSYS_ID_NONE;
    }
}

void ParticleSystem::Remove_Slave()
{
    if (m_slaveSystem != nullptr) {
        m_slaveSystem->m_masterSystem = nullptr;
        m_slaveSystem->m_masterID = PARTSYS_ID_NONE;
        m_slaveSystem = nullptr;
        m_slaveID = PARTSYS_ID_NONE;
    }
}
