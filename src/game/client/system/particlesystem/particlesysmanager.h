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
class RenderInfoClass;
class INI;

#ifdef THYME_USE_STLPORT
typedef std::hash_map<const Utf8String, ParticleSystemTemplate *, rts::hash<Utf8String>, std::equal_to<Utf8String>>
    partsystempmap_t;
#else
typedef std::unordered_map<const Utf8String, ParticleSystemTemplate *, rts::hash<Utf8String>, std::equal_to<Utf8String>>
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

    virtual int Get_On_Screen_Particle_Count() = 0;
    virtual void Set_On_Screen_Particle_Count(int count) { m_onScreenParticleCount = count; };
    virtual void Do_Particles(RenderInfoClass &rinfo) = 0;
    virtual void Queue_Particle_Render() = 0;
    virtual void Preload_Assets(TimeOfDayType time);

    ParticleSystemTemplate *Find_Template(const Utf8String &name);
    ParticleSystemTemplate *New_Template(const Utf8String &name);
    ParticleSystemTemplate *Find_Parent_Template(const Utf8String &name, int parent);
    ParticleSystem *Create_Particle_System(const ParticleSystemTemplate *temp, bool create_slaves);
    ParticleSystem *Find_Particle_System(ParticleSystemID id) const;
    void Destroy_Particle_System_By_ID(ParticleSystemID id);
    void Destroy_Attached_Systems(Object *object);
    void Add_Particle(Particle *particle, ParticlePriorityType priority);
    void Add_Particle_System(ParticleSystem *system);
    void Remove_Particle(Particle *particle);
    void Remove_Particle_System(ParticleSystem *system);
    int Get_Particle_Count() const { return m_particleCount; }
    int Get_Field_Particle_Count() const { return m_fieldParticleCount; }
    Particle *Get_Particle_Head(ParticlePriorityType priority) { return m_allParticlesHead[priority]; }
    unsigned int Remove_Oldest_Particles(unsigned int count, ParticlePriorityType priority_cap);

    partsystempmap_t::iterator Get_Starting_Template() { return m_templateStore.begin(); }
    partsystempmap_t::iterator Get_Ending_Template() { return m_templateStore.end(); }
    std::list<ParticleSystem *> &Get_All_Particle_Systems() { return m_allParticleSystemList; }
    unsigned int Get_Particle_System_Count() { return m_particleSystemCount; }

    void Set_Player_Index(unsigned int index) { m_playerIndex = index; }

    ParticleSystemID Create_Attached_Particle_System_ID(
        const ParticleSystemTemplate *temp, Object *object, bool create_slaves);
    static void Parse_Particle_System_Definition(INI *ini);
    static void Parse_Particle_System_Template(INI *ini, void *formal, void *store, const void *user_data);

protected:
    Particle *m_allParticlesHead[PARTICLE_PRIORITY_COUNT];
    Particle *m_allParticlesTail[PARTICLE_PRIORITY_COUNT];
    ParticleSystemID m_uniqueSystemID;
    std::list<ParticleSystem *> m_allParticleSystemList;
    int m_particleCount;
    int m_fieldParticleCount;
    unsigned int m_particleSystemCount;
    int m_onScreenParticleCount;
    int m_frame;
    unsigned int m_playerIndex;
    partsystempmap_t m_templateStore;
};

#ifdef GAME_DLL
#include "hooker.h"
extern ParticleSystemManager *&g_theParticleSystemManager;
#else
extern ParticleSystemManager *g_theParticleSystemManager;
#endif
