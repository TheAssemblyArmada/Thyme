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
#include "display.h"
#include "gamelogic.h"
#include "ini.h"
#include "object.h"
#include "particle.h"
#include "particlesys.h"
#include "particlesystemplate.h"
#include "xfer.h"
#include <captainslog.h>

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
    m_frame(0),
    m_playerIndex(0),
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
        captainslog_dbgassert(
            m_allParticlesHead[i] == nullptr, "INIT: ParticleSystem all particles head[%d] is not NULL!", i);
        captainslog_dbgassert(
            m_allParticlesTail[i] == nullptr, "INIT: ParticleSystem all particles tail[%d] is not NULL!", i);
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
            sys->Delete_Instance();
        }
    }

    for (int i = 0; i < PARTICLE_PRIORITY_COUNT; ++i) {
        captainslog_dbgassert(
            m_allParticlesHead[i] == nullptr, "INIT: ParticleSystem all particles head[%d] is not NULL!", i);
        captainslog_dbgassert(
            m_allParticlesTail[i] == nullptr, "INIT: ParticleSystem all particles tail[%d] is not NULL!", i);
        m_allParticlesHead[i] = nullptr;
        m_allParticlesTail[i] = nullptr;
    }

    m_particleCount = 0;
    m_fieldParticleCount = 0;
    m_particleSystemCount = 0;
    m_uniqueSystemID = PARTSYS_ID_NONE;
    m_frame = -1;
}

/**
 * @brief Update the subsystem.
 *
 * 0x004D1CC0
 */
void ParticleSystemManager::Update()
{
    if (m_frame != g_theGameLogic->Get_Frame()) {
        m_frame = g_theGameLogic->Get_Frame();

        for (auto it = m_allParticleSystemList.begin(); it != m_allParticleSystemList.end();) {
            ParticleSystem *system = *it;

            if (system != nullptr) {
                if (!system->Update(m_playerIndex)) {
                    it++;
                    system->Delete_Instance();
                } else {
                    it++;
                }
            }
        }
    }
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
            count--;
            if (!(*it)->Is_Destroyed() && (*it)->Is_Saveable()) {
                name = (*it)->Get_Template()->Get_Name();
                xfer->xferAsciiString(&name);
                xfer->xferSnapshot(*it);
            } else {
                Utf8String empty = "";
                xfer->xferAsciiString(&empty);
            }
        }

        captainslog_dbgassert(count == 0, "Mismatch in write count.");
    } else {
        for (unsigned int i = 0; i < count; ++i) {
            xfer->xferAsciiString(&name);

            if (name.Is_Not_Empty()) {
                ParticleSystemTemplate *system_template = Find_Template(name);
                captainslog_relassert(system_template != nullptr,
                    6,
                    "ParticleSystemManager::Xfer_Snapshot - Unknown particle system template '%s'",
                    name.Str());
                ParticleSystem *system = NEW_POOL_OBJ(ParticleSystem, system_template, ++m_uniqueSystemID, false);
                captainslog_relassert(system != nullptr,
                    6,
                    "ParticleSystemManager::Xfer_Snapshot - Unable to allocate particle system '%s'",
                    name.Str());
                xfer->xferSnapshot(system);
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
        retval = NEW_POOL_OBJ(ParticleSystemTemplate, name);
        auto res = m_templateStore.insert({ name, retval });

        if (!res.second && retval != nullptr) {
            retval->Delete_Instance();
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

    return NEW_POOL_OBJ(ParticleSystem, temp, ++m_uniqueSystemID, create_slaves);
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
            if ((*it)->Get_System_ID() == id) {
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
    if (object != nullptr) {
        ParticleSystem *system = nullptr;

        for (auto it = m_allParticleSystemList.begin(); it != m_allParticleSystemList.end(); it++) {
            system = *it;

            if (system != nullptr) {
                if (system->Get_Attached_Object() == object->Get_ID()) {
                    system->Destroy();
                }
            }
        }
    }
}

/**
 * @brief Preloads graphical assets for particle systems.
 *
 * 0x004D2370
 */
void ParticleSystemManager::Preload_Assets(TimeOfDayType time)
{
    for (auto it = m_templateStore.begin(); it != m_templateStore.end(); it++) {
        ParticleSystemTemplate *particle_template = (*it).second;

        if (particle_template->m_particleType == ParticleSystemInfo::PARTICLE_TYPE_PARTICLE
            && !particle_template->m_particleTypeName.Is_Empty()) {
            g_theDisplay->Preload_Texture_Assets(particle_template->m_particleTypeName);
        }
    }
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
#ifdef GAME_DEBUG
    // Debug sanity check. Assert for duplicates.
    for (const ParticleSystem *existing_system : m_allParticleSystemList)
        captainslog_dbgassert(existing_system != system, "The same ParticleSystem was added twice!");
#endif

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
            // #BUGFIX Fixes issue where loop would continue with invalidated iterator.
            // Exiting loop is ok because we do not expect any duplicates in container.
            break;
        }
    }
}

/**
 * @brief Removes count number of the oldest particles the manager knows about.
 */
unsigned int ParticleSystemManager::Remove_Oldest_Particles(unsigned int count, ParticlePriorityType priority_cap)
{
    unsigned int remaining = count - 1;

    for (unsigned int i = 0; i < count && m_particleCount != 0; ++i, --remaining) {
        for (ParticlePriorityType j = PARTICLE_PRIORITY_LOWEST; j < priority_cap; ++j) {
            if (m_allParticlesHead[j] != nullptr) {
                m_allParticlesHead[j]->Delete_Instance();
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
    ParticleSystem *sys = g_theParticleSystemManager->Create_Particle_System(temp, create_slaves);

    if (sys != nullptr && object != nullptr) {
        sys->Attach_To_Object(object);
    }

    if (sys != nullptr) {
        return sys->Get_System_ID();
    }

    return PARTSYS_ID_NONE;
}

// zh: 0x005047E0 wb: 0x009B5160
void ParticleSystemManager::Parse_Particle_System_Definition(INI *ini)
{
    auto *particle_name = ini->Get_Next_Token();
    auto *particle_template = g_theParticleSystemManager->Find_Template(particle_name);
    if (particle_template == nullptr) {
        particle_template = g_theParticleSystemManager->New_Template(particle_name);
    }

    ini->Init_From_INI(particle_template, ParticleSystemTemplate::Get_Particle_System_Template_Field_Parse_Table());
}

void ParticleSystemManager::Parse_Particle_System_Template(INI *ini, void *formal, void *store, const void *user_data)
{
    const char *name = ini->Get_Next_Token();
    ParticleSystemTemplate *tmplate = g_theParticleSystemManager->Find_Template(name);
    captainslog_dbgassert(tmplate != nullptr || strcasecmp(name, "None") == 0, "ParticleSystem %s not found!", name);
    *static_cast<ParticleSystemTemplate **>(store) = tmplate;
}
