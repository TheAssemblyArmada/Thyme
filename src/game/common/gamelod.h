////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: GAMELOD.H
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
#pragma once

#ifndef GAMELOD_H
#define GAMELOD_H

#include "asciistring.h"
#include "hooker.h"

enum DynamicGameLODLevel : int32_t
{
    DYNLOD_INVALID = -1,
    DYNLOD_LOW,
    DYNLOD_MEDIUM,
    DYNLOD_HIGH,
    DYNLOD_VERYHIGH,
    DYNLOD_COUNT,
};

enum StaticGameLODLevel : int32_t
{
    STATLOD_INVALID = -1,
    STATLOD_LOW,
    STATLOD_MEDIUM,
    STATLOD_HIGH,
    STATLOD_CUSTOM,
    STATLOD_COUNT,
};

enum CPUType : int32_t
{
    CPU_UNKNOWN,
    CPU_P3,
    CPU_P4,
    CPU_K7,
};

enum ChipsetType : int32_t
{
    CHIPSET_INVALID= -1,
    CHIPSET_UNKNOWN,
};

struct StaticGameLOD
{
    int minimum_fps;
    int minimum_cpu_fps;
    int sample_count_2D;
    int sample_count_3D;
    int stream_count;
    int max_particle_count;
    bool use_shadow_volumes;
    bool use_shadow_decals;
    bool use_cloud_map;
    bool use_light_map;
    bool show_soft_water_edges;
    int max_tank_track_edges;
    int max_tank_track_opaque_edges;
    int max_tank_track_fade_delay;
    bool use_buildup_scaffolds;
    bool use_tree_sway;
    bool use_emissive_night_materials;
    bool use_heat_effects;
    int texture_reduction_factor;
    bool use_fps_limit;
    bool use_dynamic_lod;
    bool use_trees;
};

struct DynamicGameLOD
{
    int minimum_fps;
    int particle_skip_mask;
    int debris_skip_mask;
    float slow_death_scale;
    int min_particle_priority;
    int min_particle_skip_priority;
};

struct LODPreset
{
    CPUType cpu_type;
    int mhz;
    float score;    // Not actually sure what this is, some kind of benchmark score?
    int video_type;
    int video_mem;
};

struct BenchProfile
{
    CPUType cpu_type;
    int mhz;
    float integer_score;
    float floating_point_score;
    float memory_score;
};

extern const char *g_staticGameLODNames[STATLOD_COUNT];
extern const char *g_dynamicGameLODNames[STATLOD_COUNT];

class GameLODManager
{
public:
    GameLODManager();

    void Init();
    void Refresh_Custom_Static_LOD();
    int Get_Static_LOD_Index(AsciiString name);
    const char *Get_Static_LOD_Name(StaticGameLODLevel level);
    StaticGameLODLevel Find_Static_LOD_Level();
    bool Set_Static_LOD_Level(StaticGameLODLevel level);
    void Apply_Static_LOD_Level(StaticGameLODLevel level);
    StaticGameLODLevel Get_Static_LOD_Level() { return m_staticLODLevel; }
    int Get_Dynamic_LOD_Index(AsciiString name);
    const char *Get_Dynamic_LOD_Name(DynamicGameLODLevel level);
    DynamicGameLODLevel Find_Dynamic_LOD_Level(float fps);
    bool Set_Dynamic_LOD_Level(DynamicGameLODLevel level);
    void Apply_Dynamic_LOD_Level(DynamicGameLODLevel level);
    int Get_Recommended_Texture_Reduction();
    bool Did_Mem_Pass() { return m_didMemPass; }

private:
    StaticGameLOD m_staticLOD[STATLOD_COUNT];
    DynamicGameLOD m_dynamicLOD[DYNLOD_COUNT];
    LODPreset m_LODPresets[STATLOD_COUNT - 1][32];
    BenchProfile m_benchProfiles[16];
    StaticGameLODLevel m_staticLODLevel;
    DynamicGameLODLevel m_dynamicLODLevel;
    int m_unkInt1;
    int m_particleSkipMask;
    int m_unkInt2;
    int m_debrisSkipMask;
    float m_slowDeathScale;
    int m_minParticlePriority;
    int m_minParticleSkipPriority;
    bool m_unkBool1;
    bool m_unkBool2;
    bool m_didMemPass;
    int m_staticLODPresetCount[STATLOD_COUNT - 1];
    int m_benchProfileCount;
    StaticGameLODLevel m_idealStaticGameDetail;
    ChipsetType m_chipsetType;
    CPUType m_cpuType;
    int m_physicalMem;
    int m_cpuMHz;
    float m_integerScore;
    float m_floatingPointScore;
    float m_memoryScore;
    float m_overallScore;
    int m_textureReductionFactor;
    int m_reallyLowMHz;
};

#define g_theGameLODManager (Make_Global<GameLODManager*>(0x00A2B924))
//extern GameLODManager *g_theGameLODManager;

#endif // GAMELOD_H
