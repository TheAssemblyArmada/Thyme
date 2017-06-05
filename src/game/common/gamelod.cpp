////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: GAMELOD.CPP
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Handles LOD level choices.
//
//       License:: Thyme is free software: you can redistribute it and/or 
//                 modify it under the terms of the GNU General Public License 
//                 as published by the Free Software Foundation, either version 
//                 2 of the License, or (at your option) any later version.
//
//                 A full copy of the GNU General Public License can be found in
//                 LICENSE
//
////////////////////////////////////////////////////////////////////////////////
#include "always.h"
#include "gamelod.h"
#include "globaldata.h"

// TODO when all references to original are reimplemented.
//GameLODManager *g_theGameLODManager = nullptr;

const char *g_staticGameLODNames[STATLOD_COUNT] = { 
    "Low", "Medium", "High", "Custom"
};

const char *g_dynamicGameLODNames[STATLOD_COUNT] = {
    "Low", "Medium", "High", "VeryHigh"
};

GameLODManager::GameLODManager()
{
}

void GameLODManager::Init()
{
}

void GameLODManager::Refresh_Custom_Static_LOD()
{
}

int GameLODManager::Get_Static_LOD_Index(AsciiString name)
{
    int index = STATLOD_LOW;

    for ( ; index < STATLOD_COUNT; ++index ) {
        if ( strcasecmp(name.Str(), g_staticGameLODNames[index]) == 0 ) {
            break;
        }
    }

    if ( index >= STATLOD_COUNT ) {
        return STATLOD_INVALID;
    }

    return index;
}

const char *GameLODManager::Get_Static_LOD_Name(StaticGameLODLevel level)
{
    return g_staticGameLODNames[level];
}

StaticGameLODLevel GameLODManager::Find_Static_LOD_Level()
{
    return StaticGameLODLevel();
}

bool GameLODManager::Set_Static_LOD_Level(StaticGameLODLevel level)
{
    if ( !g_theWriteableGlobalData->m_useStaticLODLevels ) {
        m_staticLODLevel = STATLOD_CUSTOM;
        return false;
    }

    if ( level == STATLOD_INVALID || (level != STATLOD_CUSTOM && m_staticLODLevel == level) ) {
        return false;
    }

    Apply_Static_LOD_Level(level);
    m_staticLODLevel = level;

    return true;
}

void GameLODManager::Apply_Static_LOD_Level(StaticGameLODLevel level)
{
    // TODO requires parts of TerrainVisual and GameClient classes.
    Call_Method<void, GameLODManager, StaticGameLODLevel>(0x0047AC60, this, level);
}

int GameLODManager::Get_Dynamic_LOD_Index(AsciiString name)
{
    int index = DYNLOD_LOW;

    for ( ; index < DYNLOD_COUNT; ++index ) {
        if ( strcasecmp(name.Str(), g_dynamicGameLODNames[index]) == 0 ) {
            break;
        }
    }

    if ( index >= DYNLOD_COUNT ) {
        return DYNLOD_INVALID;
    }

    return index;
}

const char *GameLODManager::Get_Dynamic_LOD_Name(DynamicGameLODLevel level)
{
    return g_dynamicGameLODNames[level];
}

DynamicGameLODLevel GameLODManager::Find_Dynamic_LOD_Level(float fps)
{
    int level = DYNLOD_VERYHIGH;
    
    while ( m_dynamicLOD[level].minimum_fps >= (int64_t)fps ) {
        --level;
        
        if ( level < 0 ) {
            return DYNLOD_LOW;
        }
    }

    return (DynamicGameLODLevel)level;
}

bool GameLODManager::Set_Dynamic_LOD_Level(DynamicGameLODLevel level)
{
    if ( level == DYNLOD_INVALID || level == m_dynamicLODLevel ) {
        return false;
    }

    Apply_Dynamic_LOD_Level(level);
    m_dynamicLODLevel = level;

    return true;
}

void GameLODManager::Apply_Dynamic_LOD_Level(DynamicGameLODLevel level)
{
    m_unkInt1 = 0;
    m_particleSkipMask = m_dynamicLOD[level].particle_skip_mask;
    m_unkInt2 = 0;
    m_debrisSkipMask = m_dynamicLOD[level].debris_skip_mask;
    m_slowDeathScale = m_dynamicLOD[level].slow_death_scale;
    m_minParticlePriority = m_dynamicLOD[level].min_particle_priority;
    m_minParticleSkipPriority = m_dynamicLOD[level].min_particle_skip_priority;
}

int GameLODManager::Get_Recommended_Texture_Reduction()
{
    if ( m_idealStaticGameDetail == STATLOD_INVALID ) {
        Find_Static_LOD_Level();
    }

    if ( m_didMemPass ) {
        return m_staticLOD[m_idealStaticGameDetail].texture_reduction_factor;
    }

    return m_staticLOD[STATLOD_LOW].texture_reduction_factor;
}
