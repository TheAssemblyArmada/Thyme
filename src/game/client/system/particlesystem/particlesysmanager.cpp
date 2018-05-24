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

#ifndef THYME_STANDALONE
ParticleSystemManager *&g_theParticleSystemManager = Make_Global<ParticleSystemManager *>(0x00A2BDAC);
#else
ParticleSystemManager *g_theParticleSystemManager;
#endif

ParticleSystemManager::ParticleSystemManager() :
    m_uniqueSystemID(PARTSYS_ID_NONE),
    m_allParticleSystemList(),
    m_particleCount(),
    m_unkInt1(0),
    m_particleSystemCount(0),
    m_onScreenParticleCount(0),
    m_someGameLogicValue(0),
    m_unkInt2(0),
    m_templateStore()
{
    for (int i = 0; i < PARTICLE_ARRAY_SIZE; ++i) {
        m_allParticlesHead[i] = nullptr;
        m_allParticlesTail[i] = nullptr;
    }
}

ParticleSystemManager::~ParticleSystemManager()
{
    // TODO needs ParticleSystem.
#ifndef THYME_STANDALONE
    Call_Method<void, ParticleSystemManager>(0x004D18E0, this);
#endif
}

void ParticleSystemManager::Init()
{
    INI ini;
    ini.Load("Data/INI/ParticleSystem.ini", INI_LOAD_OVERWRITE, nullptr);

    for (int i = 0; i < PARTICLE_ARRAY_SIZE; ++i) {
        m_allParticlesHead[i] = nullptr;
        m_allParticlesTail[i] = nullptr;
    }
}

void ParticleSystemManager::Reset()
{
    //TODO Needs ParticleSystem
#if 0
    while (m_particleSystemCount) {
        ParticleSystem *sys = *m_allParticleSystemList.begin();

        if (sys != nullptr) {
            Delete_Instance(sys);
        }
    }

    for (int i = 0; i < PARTICLE_ARRAY_SIZE; ++i) {
        m_allParticlesHead[i] = nullptr;
        m_allParticlesTail[i] = nullptr;
    }

    m_particleCount = 0;
    m_unkInt1 = 0;
    m_particleSystemCount = 0;
    m_uniqueSystemID = 0;
    m_someGameLogicValue = -1;
#endif
}

void ParticleSystemManager::Update()
{
    // TODO Needs game logic.
#ifndef THYME_STANDALONE
    Call_Method<void, ParticleSystemManager>(0x004D1CC0, this);
#endif
}

void ParticleSystemManager::Xfer_Snapshot(Xfer *xfer)
{
    // TODO Needs ParticleSystem
#ifndef THYME_STANDALONE
    Call_Method<void, ParticleSystemManager, Xfer *>(0x004D2460, this, xfer);
#endif
}

ParticleSystemTemplate *ParticleSystemManager::Find_Template(const AsciiString &name)
{
    auto it = m_templateStore.find(name);

    if (it != m_templateStore.end()) {
        return it->second;
    }

    return nullptr;
}

ParticleSystem *ParticleSystemManager::Create_Particle_System(const ParticleSystemTemplate *temp, bool create_slaves)
{
    if (temp == nullptr) {
        return nullptr;
    }

    return new ParticleSystem(*temp, ++m_uniqueSystemID, create_slaves);
}
