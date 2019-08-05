/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Manager class for the engines particle systems.
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
#include "rtsutils.h"
#include "snapshot.h"
#include "subsysteminterface.h"
#include <list>
#include <vector>

#ifdef THYME_USE_STLPORT
#include <hash_map>
#else
#include <unordered_map>
#endif

class Particle;
class ParticleSystem;
class ParticleSystemTemplate;
class Object;

#ifdef THYME_USE_STLPORT
    typedef std::hash_map<const Utf8String, ParticleSystemTemplate *, rts::hash<Utf8String>, rts::equal_to<Utf8String>>
        partsystempmap_t;
#else
    typedef std::unordered_map<const Utf8String, ParticleSystemTemplate *, rts::hash<Utf8String>,
        rts::equal_to<Utf8String>>
        partsystempmap_t;
#endif

enum ParticleSystemID : int32_t
{
    PARTSYS_ID_NONE,
};
DEFINE_ENUMERATION_OPERATORS(ParticleSystemID);

class ParticleSystemManager : public SubsystemInterface, public SnapShot
{
public:
    ParticleSystemManager();
    virtual ~ParticleSystemManager();

    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override;

    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    ParticleSystemTemplate *Find_Template(const Utf8String &name);
    ParticleSystemTemplate *New_Template(const Utf8String &name);
    ParticleSystemTemplate *Find_Parent_Template(const Utf8String &name, int parent);
    ParticleSystem *Create_Particle_System(const ParticleSystemTemplate *temp, bool create_slaves);
    ParticleSystem *Find_Particle_System(ParticleSystemID id) const;
    void Destroy_Particle_System_By_ID(ParticleSystemID id);
    void Destroy_Attached_Systems(Object *object);
    void Preload_Assets(TimeOfDayType time);
    void Add_Particle(Particle *particle, ParticlePriorityType priority);
    void Add_Particle_System(ParticleSystem *system);
    void Remove_Particle(Particle *particle);
    void Remove_Particle_System(ParticleSystem *system);
    void Set_On_Screen_Particle_Count(int count) { m_onScreenParticleCount = count; }
    int Particle_Count() const { return m_particleCount; }
    int Field_Particle_Count() const { return m_fieldParticleCount; }
    Particle *Get_Particle_Head(ParticlePriorityType priority) { return m_allParticlesHead[priority]; }
    unsigned Remove_Oldest_Particles(unsigned count, ParticlePriorityType priority_cap);

    static ParticleSystemID Create_Attached_Particle_System_ID(
        const ParticleSystemTemplate *temp, Object *object, bool create_slaves);

#ifdef GAME_DLL
    ParticleSystemManager *Hook_Ctor() { return new (this) ParticleSystemManager(); }
    void Hook_Dtor() { ParticleSystemManager::~ParticleSystemManager(); }
    void Hook_Xfer(Xfer *xfer) { ParticleSystemManager::Xfer_Snapshot(xfer); }
    void Hook_Init() { ParticleSystemManager::Init(); }
    void Hook_Reset() { ParticleSystemManager::Reset(); }
#endif

private:
    Particle *m_allParticlesHead[PARTICLE_PRIORITY_COUNT];
    Particle *m_allParticlesTail[PARTICLE_PRIORITY_COUNT];
    ParticleSystemID m_uniqueSystemID;
    std::list<ParticleSystem *> m_allParticleSystemList;
    int m_particleCount;
    int m_fieldParticleCount;
    unsigned m_particleSystemCount;
    int m_onScreenParticleCount;
    int m_someGameLogicValue;
    int m_unkInt2;
    partsystempmap_t m_templateStore;
};

#ifdef GAME_DLL
#include "hooker.h"
extern ParticleSystemManager *&g_theParticleSystemManager;
#else
extern ParticleSystemManager *g_theParticleSystemManager;
#endif
