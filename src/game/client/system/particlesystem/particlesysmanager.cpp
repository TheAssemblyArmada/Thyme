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
#include "particlesysmanager.h"
#include "ini.h"
#include "particlesys.h"
#include "particlesystemplate.h"
#include "particle.h"
#include "xfer.h"
#include <captnassert.h>

#ifdef GAME_DLL
#else
ParticleSystemManager *g_theParticleSystemManager;
#endif

/**
 * 0x004D1790
 */
ParticleSystemManager::ParticleSystemManager() :
    m_uniqueSystemID(PARTSYS_ID_NONE),
    m_allParticleSystemList(),
    m_particleCount(),
    m_fieldParticleCount(0),
    m_particleSystemCount(0),
    m_onScreenParticleCount(0),
    m_someGameLogicValue(0),
    m_unkInt2(0),
    m_templateStore()
{
    for (int i = 0; i < PARTICLE_PRIORITY_COUNT; ++i) {
        m_allParticlesHead[i] = nullptr;
        m_allParticlesTail[i] = nullptr;
    }
}

/**
 * 0x004D18E0
 */
ParticleSystemManager::~ParticleSystemManager()
{
    Reset();
}

/**
 * @brief Initialise the subsystem.
 *
 * 0x004D1BA0
 */
void ParticleSystemManager::Init()
{
    INI ini;
    ini.Load("Data/INI/ParticleSystem.ini", INI_LOAD_OVERWRITE, nullptr);

    for (int i = 0; i < PARTICLE_PRIORITY_COUNT; ++i) {
        m_allParticlesHead[i] = nullptr;
        m_allParticlesTail[i] = nullptr;
    }
}

/**
 * @brief Reset the subsystem.
 *
 * 0x004D1C40
 */
void ParticleSystemManager::Reset()
{
    while (m_particleSystemCount) {
        ParticleSystem *sys = *m_allParticleSystemList.begin();

        if (sys != nullptr) {
            Delete_Instance(sys);
        }
    }

    for (int i = 0; i < PARTICLE_PRIORITY_COUNT; ++i) {
        m_allParticlesHead[i] = nullptr;
        m_allParticlesTail[i] = nullptr;
    }

    m_particleCount = 0;
    m_fieldParticleCount = 0;
    m_particleSystemCount = 0;
    m_uniqueSystemID = PARTSYS_ID_NONE;
    m_someGameLogicValue = -1;
}

/**
 * @brief Update the subsystem.
 *
 * 0x004D1CC0
 */
void ParticleSystemManager::Update()
{
    // TODO Needs game logic.
#ifdef GAME_DLL
    Call_Method<void, ParticleSystemManager>(PICK_ADDRESS(0x004D1CC0, 0x00769348), this);
#endif
}

/**
 * @brief Xfer this Snapshot object.
 *
 * 0x004D2460
 */
void ParticleSystemManager::Xfer_Snapshot(Xfer *xfer)
{
#define PARTICLE_XFER_VERSION 1
    uint8_t version = PARTICLE_XFER_VERSION;
    xfer->xferVersion(&version, PARTICLE_XFER_VERSION);
    xfer->xferInt(reinterpret_cast<int32_t *>(&m_uniqueSystemID));
    uint32_t count = m_particleSystemCount;
    xfer->xferUnsignedInt(&count);
    Utf8String name;

    if (xfer->Get_Mode() == XFER_SAVE) {
        for (auto it = m_allParticleSystemList.begin(); it != m_allParticleSystemList.end(); ++it) {
            if (!(*it)->m_isDestroyed && (*it)->m_saveable) {
                name = (*it)->m_template->Get_Name();
                xfer->xferAsciiString(&name);
                xfer->xferSnapshot(*it);
            } else {
                Utf8String empty = "";
                xfer->xferAsciiString(&empty);
            }
        }
    } else {
        for (unsigned i = 0; i < count; ++i) {
            xfer->xferAsciiString(&name);

            if (name.Is_Not_Empty()) {
                ParticleSystemTemplate *temp = Find_Template(name);
                captain_relassert(temp != nullptr, 6, "Could not find a matching particle system template for '%s'.\n", name.Str());
                ParticleSystem *sys = new ParticleSystem(temp, ++m_uniqueSystemID, false);
                captain_relassert(sys != nullptr, 6, "Could not create particle system for '%s', allocation issue.\n", name.Str());
                xfer->xferSnapshot(sys);
            }
        }
    }
}

/**
 * @brief Find a particle system template by name.
 *
 * 0x004D1EB0
 */
ParticleSystemTemplate *ParticleSystemManager::Find_Template(const Utf8String &name)
{
    auto it = m_templateStore.find(name);

    if (it != m_templateStore.end()) {
        return it->second;
    }

    return nullptr;
}

/**
 * @brief Find a particle system template by name and create a new one if it didn't exist.
 *
 * 0x004D1EE0
 */
ParticleSystemTemplate *ParticleSystemManager::New_Template(const Utf8String &name)
{
    ParticleSystemTemplate *retval = Find_Template(name);

    if (retval == nullptr) {
        retval = new ParticleSystemTemplate(name);
        auto res = m_templateStore.insert({name, retval});

        if (!res.second && retval != nullptr) {
            delete retval;
            retval = nullptr;
        }
    }

    return retval;
}

/**
 * @brief Find a particle system template parent.
 *
 * 0x004D2130
 */
ParticleSystemTemplate *ParticleSystemManager::Find_Parent_Template(const Utf8String &name, int parent)
{
    if (name.Is_Not_Empty()) {
        for (auto it = m_templateStore.begin(); it != m_templateStore.end(); ++it) {
            if (strcmp(it->second->m_slaveSystemName.Str(), name.Str()) == 0 && parent-- == 0) {
                return it->second;
            }
        }
    }

    return nullptr;
}

/**
 * @brief Create a particle system from a template.
 *
 * 0x004D1D40
 */
ParticleSystem *ParticleSystemManager::Create_Particle_System(const ParticleSystemTemplate *temp, bool create_slaves)
{
    if (temp == nullptr) {
        return nullptr;
    }

    return new ParticleSystem(temp, ++m_uniqueSystemID, create_slaves);
}

/**
 * @brief Find a particle system from a unique id.
 *
 * 0x004D1E30
 */
ParticleSystem *ParticleSystemManager::Find_Particle_System(ParticleSystemID id) const 
{
    if (id != PARTSYS_ID_NONE) {
        for (auto it = m_allParticleSystemList.begin(); it != m_allParticleSystemList.end(); ++it) {
            if ((*it)->System_ID() == id) {
                return *it;
            }
        }
    }

    return nullptr;
}

/**
 * @brief Destroys a particle system from a unique id.
 *
 * 0x004D1E60
 */
void ParticleSystemManager::Destroy_Particle_System_By_ID(ParticleSystemID id)
{
    ParticleSystem *sys = Find_Particle_System(id);

    if (sys != nullptr) {
        sys->Destroy();
    }
}

/**
 * @brief Destroys particle systems attached to an object.
 *
 * 0x004D2270
 */
void ParticleSystemManager::Destroy_Attached_Systems(Object *object)
{
    // TODO Requires Object.
#ifdef GAME_DLL
    Call_Method<void, ParticleSystemManager, Object *>(PICK_ADDRESS(0x004D2270, 0x00769810), this, object);
#endif
}

/**
 * @brief Preloads graphical assets for particle systems.
 *
 * 0x004D2370
 */
void ParticleSystemManager::Preload_Assets(TimeOfDayType time)
{
    // TODO Requires Display.
#ifdef GAME_DLL
    Call_Method<void, ParticleSystemManager, TimeOfDayType>(PICK_ADDRESS(0x004D2370, 0x00769B8B), this, time);
#endif
}

/**
 * @brief Add a particle to the management lists.
 */
void ParticleSystemManager::Add_Particle(Particle *particle, ParticlePriorityType priority)
{
    if (!particle->m_inOverallList) {
        if (m_allParticlesHead[priority] == nullptr) {
            m_allParticlesHead[priority] = particle;
        }

        if (m_allParticlesTail[priority] != nullptr) {
            m_allParticlesTail[priority]->m_overallNext = particle;
        }

        particle->m_overallPrev = m_allParticlesTail[priority];
        m_allParticlesTail[priority] = particle;
        particle->m_overallNext = nullptr;
        particle->m_inOverallList = true;
        ++m_particleCount;
    }
}

/**
 * @brief Add a particle system to the management lists.
 */
void ParticleSystemManager::Add_Particle_System(ParticleSystem *system)
{
    m_allParticleSystemList.push_back(system);
    ++m_particleSystemCount;
}

/**
 * @brief Remote a particle from the management lists.
 *
 * 0x004D22D0
 */
void ParticleSystemManager::Remove_Particle(Particle *particle)
{
    if (particle->m_inOverallList) {
        ParticlePriorityType priority = particle->Get_Priority();

        if (particle->m_overallNext != nullptr) {
            particle->m_overallNext->m_overallPrev = particle->m_overallPrev;
        }

        if (particle->m_overallPrev != nullptr) {
            particle->m_overallPrev->m_overallNext = particle->m_overallNext;
        }

        if (particle == m_allParticlesHead[priority]) {
            m_allParticlesHead[priority] = particle->m_overallNext;
        }

        if (particle == m_allParticlesTail[priority]) {
            m_allParticlesTail[priority] = particle->m_overallPrev;
        }

        particle->m_overallPrev = nullptr;
        particle->m_overallNext = nullptr;
        particle->m_inOverallList = false;
        --m_particleCount;
    }
}

/**
 * @brief Remove a particle system from the management lists.
 */
void ParticleSystemManager::Remove_Particle_System(ParticleSystem *system)
{
    for (auto it = m_allParticleSystemList.begin(); it != m_allParticleSystemList.end(); ++it) {
        if (*it == system) {
            m_allParticleSystemList.erase(it);
            --m_particleSystemCount;
        }
    }
}

/**
 * @brief Removes count number of the oldest particles the manager knows about.
 */
unsigned ParticleSystemManager::Remove_Oldest_Particles(unsigned count, ParticlePriorityType priority_cap)
{
    unsigned remaining = count - 1;

    for (unsigned i = 0; i < count && m_particleCount != 0; ++i, --remaining) {
        for (ParticlePriorityType j = PARTICLE_PRIORITY_LOWEST; j < priority_cap; ++j) {
            if (m_allParticlesHead[j] != nullptr) {
                Delete_Instance(m_allParticlesHead[j]);
                break;
            }
        }
    }

    return count - remaining;
}

/**
 * @brief Creates particle system and attaches it to the object, returning the ID allocated.
 *
 * 0x004D1DF0
 */
ParticleSystemID ParticleSystemManager::Create_Attached_Particle_System_ID(
    const ParticleSystemTemplate *temp, Object *object, bool create_slaves)
{
#ifdef GAME_DLL
    return Call_Function<ParticleSystemID, const ParticleSystemTemplate *, Object *, bool>(
        PICK_ADDRESS(0x004D1DF0, 0x007694CC), temp, object, create_slaves);
#else
    ParticleSystem *sys = g_theParticleSystemManager->Create_Particle_System(temp, create_slaves);

    if (sys != nullptr) {
        // TODO requires Object.
    }

    return PARTSYS_ID_NONE;
#endif
}
