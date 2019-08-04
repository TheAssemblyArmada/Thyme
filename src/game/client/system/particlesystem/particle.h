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

public:
    Particle(ParticleSystem *system, const ParticleInfo &info);
    virtual ~Particle();

    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    void Compute_Alpha_Rate();
    void Compute_Color_Rate();
    void Apply_Force(const Coord3D &force);
    void Do_Wind_Motion();

    ParticlePriorityType Get_Priority() const;

#ifdef GAME_DLL
    static void Hook_Me();
    Particle *Hook_Ctor(ParticleSystem *system, const ParticleInfo &info) { return new (this) Particle(system, info); }
    void Hook_Dtor() { Particle::~Particle(); }
    void Hook_Xfer(Xfer *xfer) { Particle::Xfer_Snapshot(xfer); }
    void Hook_LoadPP() { Particle::Load_Post_Process(); }
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
};

#ifdef GAME_DLL
#include "hooker.h"

inline void Particle::Hook_Me()
{
    Hook_Method(0x004CCC30, &Particle::Hook_Ctor);
    Hook_Method(0x004CD040, &Particle::Hook_Dtor);
    Hook_Method(0x004CD2E0, &Particle::Hook_Xfer);
    Hook_Method(0x004CD3F0, &Particle::Hook_LoadPP);
    Hook_Method(0x004CCB50, &Particle::Compute_Alpha_Rate);
    Hook_Method(0x004CCBA0, &Particle::Compute_Color_Rate);
}
#endif
