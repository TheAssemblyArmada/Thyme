/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Handle LOD level choices.
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
#include "asciistring.h"
#include "gametype.h"
#include "ini.h"

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

enum GPUType : int32_t
{
    GPU_INVALID = -1,
    GPU_UNKNOWN,
    GPU_V2,
    GPU_V3,
    GPU_V4,
    GPU_V5,
    GPU_TNT,
    GPU_TNT2,
    GPU_GF2,
    GPU_R100,
    GPU_PS11,
    GPU_GF3,
    GPU_GF4,
    GPU_PS14,
    GPU_R200,
    GPU_PS20,
    GPU_R300,
    GPU_COUNT,
};

struct StaticGameLODInfo
{
    StaticGameLODInfo() :
        minimum_fps(0),
        minimum_cpu_fps(0),
        sample_count_2D(6),
        sample_count_3D(24),
        stream_count(2),
        max_particle_count(2500),
        use_shadow_volumes(true),
        use_shadow_decals(true),
        use_cloud_map(true),
        use_light_map(true),
        show_soft_water_edges(true),
        max_tank_track_edges(100),
        max_tank_track_opaque_edges(25),
        max_tank_track_fade_delay(300000),
        use_buildup_scaffolds(true),
        use_tree_sway(true),
        use_emissive_night_materials(true),
        use_heat_effects(true),
        texture_reduction_factor(false),
        use_fps_limit(true),
        use_dynamic_lod(true),
        use_trees(true)
    {
    }

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

struct DynamicGameLODInfo
{
    DynamicGameLODInfo() :
        minimum_fps(0),
        particle_skip_mask(0),
        debris_skip_mask(0),
        slow_death_scale(1.0f),
        min_particle_priority(PARTICLE_PRIORITY_LOWEST),
        min_particle_skip_priority(PARTICLE_PRIORITY_LOWEST)
    {
    }

    int minimum_fps;
    int particle_skip_mask;
    int debris_skip_mask;
    float slow_death_scale;
    ParticlePriorityType min_particle_priority;
    ParticlePriorityType min_particle_skip_priority;
};

struct LODPreset
{
    LODPreset() : cpu_type(CPU_UNKNOWN), mhz(1), score(1.0f), video_type(GPU_UNKNOWN), video_mem(1) {}

    CPUType cpu_type;
    int mhz;
    float score; // Not actually sure what this is, some kind of benchmark score?
    GPUType video_type;
    int video_mem;
};

struct BenchProfile
{
    BenchProfile() : cpu_type(CPU_UNKNOWN), mhz(1), integer_score(1.0f), floating_point_score(1.0f), memory_score(1.0f) {}
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
    int Get_Static_LOD_Index(Utf8String name);
    const char *Get_Static_LOD_Name(StaticGameLODLevel level);
    StaticGameLODLevel Find_Static_LOD_Level();
    bool Set_Static_LOD_Level(StaticGameLODLevel level);
    void Apply_Static_LOD_Level(StaticGameLODLevel level);
    StaticGameLODLevel Get_Static_LOD_Level() { return m_staticLODLevel; }
    int Get_Dynamic_LOD_Index(Utf8String name);
    const char *Get_Dynamic_LOD_Name(DynamicGameLODLevel level);
    DynamicGameLODLevel Find_Dynamic_LOD_Level(float fps);
    bool Set_Dynamic_LOD_Level(DynamicGameLODLevel level);
    void Apply_Dynamic_LOD_Level(DynamicGameLODLevel level);
    int Get_Recommended_Texture_Reduction();
    bool Did_Mem_Pass() { return m_didMemPass; }
    void Set_Texture_Reduction_Factor(int factor) { m_textureReductionFactor = factor; }

    void Increment_Particle_Skip_Count() { ++m_particleSkipCount; }
    int Particle_Count() const { return m_particleSkipCount; }
    int Particle_Skip_Mask() const { return m_particleSkipMask; }
    bool Is_Particle_Skipped() { return m_particleSkipMask != (m_particleSkipMask & (++m_particleSkipCount)); }
    bool Is_Debris_Skipped() { return m_debrisSkipMask != (m_debrisSkipMask & (++m_debrisSkipCount)); }
    ParticlePriorityType Min_Particle_Priority() const { return m_minParticlePriority; }
    ParticlePriorityType Min_Particle_Skip_Priority() const { return m_minParticleSkipPriority; }
    bool Is_Slow_CPU() const { return m_cpuMHz < m_reallyLowMHz; }

    static void Parse_Static_LOD_Definition(INI *ini);
    static void Parse_Dynamic_LOD_Definition(INI *ini);
    static void Parse_LOD_Preset(INI *ini);
    static void Parse_Bench_Profile(INI *ini);
    static void Parse_Really_Low_MHz(INI *ini);
    static void Parse_Static_Game_LOD_Level(INI *ini, void *formal, void *store, const void *user_data);

private:
    enum
    {
        MAX_PRESETS = 32,
        MAX_PROFILES = 16,
    };

    StaticGameLODInfo m_staticLOD[STATLOD_COUNT];
    DynamicGameLODInfo m_dynamicLOD[DYNLOD_COUNT];
    LODPreset m_LODPresets[STATLOD_COUNT - 1][MAX_PRESETS];
    BenchProfile m_benchProfiles[MAX_PROFILES];
    StaticGameLODLevel m_staticLODLevel;
    DynamicGameLODLevel m_dynamicLODLevel;
    int m_particleSkipCount;
    int m_particleSkipMask;
    int m_debrisSkipCount;
    int m_debrisSkipMask;
    float m_slowDeathScale;
    ParticlePriorityType m_minParticlePriority;
    ParticlePriorityType m_minParticleSkipPriority;
    bool m_unkBool1;
    bool m_unkBool2;
    bool m_didMemPass;
    int m_staticLODPresetCount[STATLOD_COUNT - 1];
    int m_benchProfileCount;
    StaticGameLODLevel m_idealStaticGameDetail;
    GPUType m_gpuType;
    CPUType m_cpuType;
    int m_physicalMem;
    int m_cpuMHz;
    float m_integerScore;
    float m_floatingPointScore;
    float m_memoryScore;
    float m_overallScore;
    int m_textureReductionFactor;
    int m_reallyLowMHz;

    static const char *s_cpuNames[];
    static const char *s_gpuNames[];
};

void Test_Minimum_Requirements(
    GPUType *gpu, CPUType *cpu, int *cpu_speed, int *memory, float *int_score, float *float_score, float *mem_score);

#ifdef GAME_DLL
#include "hooker.h"

extern GameLODManager *&g_theGameLODManager;
#else
extern GameLODManager *g_theGameLODManager;
#endif