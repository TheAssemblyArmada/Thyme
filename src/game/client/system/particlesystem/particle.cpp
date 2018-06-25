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
#include "particlesys.h"
#include "particlesysmanager.h"
#include "xfer.h"

Particle::Particle(ParticleSystem *system, const ParticleInfo &info) :
    ParticleInfo(info),
    m_system(system),
    m_particleIDMaybe(0),
    m_createTimestamp(),
    m_drawable(false)
{
    //TODO
}

Particle::~Particle()
{
    
}

void Particle::Xfer_Snapshot(Xfer *xfer) 
{
#define PARTICLE_XFER_VERSION 1
    uint8_t version = PARTICLE_XFER_VERSION;
    xfer->xferVersion(&version, PARTICLE_XFER_VERSION);
    ParticleInfo::Xfer_Snapshot(xfer);
    xfer->xferUnsignedInt(&m_particleIDMaybe);
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
    
    DrawableID drawable_id_fake = 0; // Looks like this was removed and a fake entry added to avoid version bump.
    xfer->xferDrawableID(&drawable_id_fake);

    ParticleSystemID id = m_systemUnderControl != nullptr ? m_systemUnderControl->System_ID() : PARTSYS_ID_NONE;
    xfer->xferUnsignedInt(reinterpret_cast<uint32_t *>(&id));
}

void Particle::Load_Post_Process()
{
    // When is the system ID loaded to the memory for the pointer? Some unswizzling scheme?
    if (m_systemUnderControl != nullptr) {
        ParticleSystemID id = ParticleSystemID(reinterpret_cast<uintptr_t>(m_systemUnderControl));
        ParticleSystem *sys = g_theParticleSystemManager->Find_Particle_System(id);
        ASSERT_THROW_PRINT(sys != nullptr, 6, "Failed to find controlled particle system for ID %d.\n", id);
        sys->Set_Control_Particle(this);
        m_systemUnderControl = sys;
    }
}

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

void Particle::Apply_Force(const Coord3D &force)
{
    m_accel += force;
}

void Particle::Do_Wind_Motion()
{
    // TODO requires GameLogic and GameClient global.
#ifndef THYME_STANDALONE
    Call_Method<void, Particle>(0x004CD160, this);
#endif
}

