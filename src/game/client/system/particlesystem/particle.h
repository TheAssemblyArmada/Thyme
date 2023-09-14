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
#pragma once

#include "always.h"
#include "gametype.h"
#include "mempoolobj.h"
#include "particleinfo.h"

class ParticleSystem;

class Particle : public MemoryPoolObject, public ParticleInfo
{
    IMPLEMENT_NAMED_POOL(Particle, ParticlePool);
    friend class ParticleSystem;
    friend class ParticleSystemManager;

protected:
    virtual ~Particle() override;

public:
    Particle(ParticleSystem *system, const ParticleInfo &info);

    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    void Compute_Alpha_Rate();
    void Compute_Color_Rate();
    void Apply_Force(const Coord3D &force);
    void Do_Wind_Motion();
    bool Update();
    bool Is_Invisible();

    const Coord3D *Get_Position() const { return &m_pos; }
    float Get_Size() const { return m_size; }
    float Get_Angle() const { return m_angleZ; }
    float Get_Alpha() const { return m_alpha; }
    const RGBColor *Get_Color() const { return &m_color; }
    uint32_t Get_ID() const { return m_particleID; }

    void Set_ID(uint32_t id) { m_particleID = id; }

    void Control_Particle_System(ParticleSystem *system) { m_systemUnderControl = system; }
    void Detach_Controlled_Particle_System() { m_systemUnderControl = nullptr; }

    ParticlePriorityType Get_Priority() const;

#ifdef GAME_DLL
    Particle *Hook_Ctor(ParticleSystem *system, const ParticleInfo &info) { return new (this) Particle(system, info); }
    void Hook_Dtor() { Particle::~Particle(); }
#endif
private:
    Particle *m_systemNext;
    Particle *m_systemPrev;
    Particle *m_overallNext;
    Particle *m_overallPrev;
    ParticleSystem *m_system;
    uint32_t m_particleID;
    Coord3D m_accel;
    Coord3D m_lastPos;
    uint32_t m_lifetimeLeft;
    uint32_t m_createTimestamp;
    float m_alpha;
    float m_alphaRate;
    int32_t m_alphaTargetKey;
    RGBColor m_color;
    RGBColor m_colorRate;
    int32_t m_colorTargetKey;
    bool m_drawable;
    bool m_inSystemList;
    bool m_inOverallList;
    ParticleSystem *m_systemUnderControl;
    friend class W3DParticleSystemManager;
};
