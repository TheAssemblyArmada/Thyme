/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Object holding a single particle instance.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */ 
#include "particle.h"
#include "gameclient.h"
#include "particlesys.h"
#include "particlesysmanager.h"
#include "xfer.h"

/**
 * 0x004CCC30
 */
Particle::Particle(ParticleSystem *system, const ParticleInfo &info) :
    m_systemNext(nullptr),
    m_systemPrev(nullptr),
    m_overallNext(nullptr),
    m_overallPrev(nullptr),
    m_system(system),
    m_particleID(0),
    m_accel(),
    m_lastPos(),
    m_lifetimeLeft(info.m_lifetime),
    m_createTimestamp(g_theGameClient->Get_Frame()),
    m_alpha(info.m_alphaKey[0].value),
    m_alphaTargetKey(true),
    m_color(info.m_colorKey[0].color),
    m_colorTargetKey(true),
    m_drawable(false),
    m_inSystemList(false),
    m_inOverallList(false),
    m_systemUnderControl(nullptr)
{
    m_vel = info.m_vel;
    m_pos = info.m_pos;
    m_emitterPos = info.m_emitterPos;
    m_velDamping = info.m_velDamping;
#ifdef THYME_STANDALONE
    m_angleX = info.m_angleX;
    m_angleY = info.m_angleY;
#endif
    m_angleZ = info.m_angleZ;
#ifdef THYME_STANDALONE
    m_angularRateX = info.m_angularRateX;
    m_angularRateY = info.m_angularRateY;
#endif
    m_angularRateZ = info.m_angularRateZ;
    m_angularDamping = info.m_angularDamping;
    m_lifetime = info.m_lifetime;
    m_size = info.m_size;
    m_sizeRate = info.m_sizeRate;
    m_sizeRateDamping = info.m_sizeRateDamping;
    m_colorScale = info.m_colorScale;
    m_windRandomness = info.m_windRandomness;
    m_particleUpTowardsEmitter = info.m_particleUpTowardsEmitter;

    for (int i = 0; i < KEYFRAME_COUNT; ++i) {
        m_alphaKey[i] = info.m_alphaKey[i];
    }

    Compute_Alpha_Rate();

    for (int i = 0; i < KEYFRAME_COUNT; ++i) {
        m_colorKey[i] = info.m_colorKey[i];
    }

    Compute_Color_Rate();
    g_theParticleSystemManager->Add_Particle(this, m_system->Get_Priority());
    m_system->Add_Particle(this);
}

/**
 * 0x004CD040
 */
Particle::~Particle()
{
    m_system->Remove_Particle(this);

    if (m_systemUnderControl != nullptr) {
        m_systemUnderControl->Set_Control_Particle(nullptr);
        m_systemUnderControl->Destroy();
    }

    m_systemUnderControl = nullptr;
    g_theParticleSystemManager->Remove_Particle(this);
}

/**
 * @brief Transfer the object data.
 *
 * 0x004CD2E0
 */
void Particle::Xfer_Snapshot(Xfer *xfer) 
{
#define PARTICLE_XFER_VERSION 1
    uint8_t version = PARTICLE_XFER_VERSION;
    xfer->xferVersion(&version, PARTICLE_XFER_VERSION);
    ParticleInfo::Xfer_Snapshot(xfer);
    xfer->xferUnsignedInt(&m_particleID);
    xfer->xferCoord3D(&m_accel);
    xfer->xferCoord3D(&m_lastPos);
    xfer->xferUnsignedInt(&m_lifetimeLeft);
    xfer->xferUnsignedInt(&m_createTimestamp);
    xfer->xferReal(&m_alpha);
    xfer->xferReal(&m_alphaRate);
    xfer->xferInt(&m_alphaTargetKey);
    xfer->xferRGBColor(&m_color);
    xfer->xferRGBColor(&m_colorRate);
    xfer->xferInt(&m_colorTargetKey);
    
    DrawableID drawable_id_fake = DRAWABLE_UNK; // Looks like this was removed and a fake entry added to avoid version bump.
    xfer->xferDrawableID(&drawable_id_fake);

    // This looks like it will save/process the value, but not restore it?
    ParticleSystemID id = m_systemUnderControl != nullptr ? m_systemUnderControl->System_ID() : PARTSYS_ID_NONE;
    xfer->xferInt(reinterpret_cast<int32_t *>(&id));
}

/**
 * @brief Performs additional post data load house keeping.
 *
 * 0x004CD3F0
 */
void Particle::Load_Post_Process()
{
    // When is the system ID loaded to the memory for the pointer? Some unswizzling scheme?
    if (m_systemUnderControl != nullptr) {
        ParticleSystemID id = ParticleSystemID(reinterpret_cast<uintptr_t>(m_systemUnderControl));
        ParticleSystem *sys = g_theParticleSystemManager->Find_Particle_System(id);
        DEBUG_ASSERT_THROW(sys != nullptr, 6, "Failed to find controlled particle system for ID %d.\n", id);
        sys->Set_Control_Particle(this);
        m_systemUnderControl = sys;
    }
}

/**
 * @brief Computes rate based on current target key.
 *
 * 0x004CCB50
 */
void Particle::Compute_Alpha_Rate()
{
    if (m_alphaKey[m_alphaTargetKey].frame != 0) {
        float val_diff = m_alphaKey[m_alphaTargetKey].value - m_alphaKey[m_alphaTargetKey - 1].value;
        float frame_diff = float(int(m_alphaKey[m_alphaTargetKey].frame - m_alphaKey[m_alphaTargetKey - 1].frame));
        m_alphaRate = val_diff / frame_diff;
    } else {
        m_alphaRate = 0.0f;
    }
}

/**
 * @brief Computes rate based on current target key.
 *
 * 0x004CCBA0
 */
void Particle::Compute_Color_Rate()
{
    if (m_colorKey[m_colorTargetKey].frame != 0) {
        float frame_diff = float(int(m_colorKey[m_colorTargetKey].frame - m_colorKey[m_colorTargetKey - 1].frame));
        m_colorRate.red =
            float(m_colorKey[m_colorTargetKey].color.red - m_colorKey[m_colorTargetKey - 1].color.red) / frame_diff;
        m_colorRate.green =
            float(m_colorKey[m_colorTargetKey].color.green - m_colorKey[m_colorTargetKey - 1].color.green) / frame_diff;
        m_colorRate.blue =
            float(m_colorKey[m_colorTargetKey].color.blue - m_colorKey[m_colorTargetKey - 1].color.blue) / frame_diff;
    } else {
        m_colorRate = {0.0f, 0.0f, 0.0f};
    }
}

/**
 * @brief Applies some force to the accelration vector.
 */
void Particle::Apply_Force(const Coord3D &force)
{
    m_accel += force;
}

/**
 * @brief Applies wind motion logic to the particle.
 *
 * 0x004CD160
 */
void Particle::Do_Wind_Motion()
{
    // TODO requires GameLogic and GameClient global.
#ifndef THYME_STANDALONE
    Call_Method<void, Particle>(0x004CD160, this);
#endif
}

/**
 * @brief Gets the priority of the system this particle belongs to.
 */
ParticlePriorityType Particle::Get_Priority() const
{
    return m_system->Get_Priority();
}
