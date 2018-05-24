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
#pragma once

#include "always.h"
#include "matrix3d.h"
#include "mempoolobj.h"
#include "particlesysinfo.h"
#include "particlesysmanager.h"

class Particle;
class ParticleSystemTemplate;

class ParticleSystem : public MemoryPoolObject, public ParticleSystemInfo
{
    IMPLEMENT_POOL(ParticleSystem);
public:
    ParticleSystem(const ParticleSystemTemplate &temp, ParticleSystemID id, bool create_slaves);
    virtual ~ParticleSystem();

    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

private:
    void Remove_Master();
    void Remove_Slave();

private:
    Particle *m_systemParticlesHead;
    Particle *m_systemParticlesTail;
    uint32_t m_particleCount;
    ParticleSystemID m_systemID;
    uint32_t m_attachedToDrawableID;
    uint32_t m_attachedToObjectID;
    Matrix3D m_localTransform;
    Matrix3D m_transform;
    uint32_t m_burstDelayLeft;
    uint32_t m_delayLeft;
    uint32_t m_startTimestamp;
    uint32_t m_systemLifetimeLeft;
    uint32_t m_nextParticleIDMaybe;
    uint32_t m_accumulatedSizeBonus;
    Coord3D m_velCoefficient;
    float m_countCoefficient;
    float m_delayCoefficient;
    float m_sizeCoefficient;
    Coord3D m_pos;
    Coord3D m_lastPos;
    ParticleSystem *m_slaveSystem;
    ParticleSystemID m_slaveID;
    ParticleSystem *m_masterSystem;
    ParticleSystemID m_masterID;
    const ParticleSystemTemplate *m_template;
    Particle *m_controlParticle;
    bool m_isLocalIdentity;
    bool m_isIdentity;
    bool m_isForever;
    bool m_isStopped;
    bool m_isDestroyed;
    bool m_isFirstPos;
    bool m_saveable;
    bool m_unkBool1;
};