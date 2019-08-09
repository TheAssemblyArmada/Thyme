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
#include "gamelod.h"
#include "cpudetect.h"
#include "globaldata.h"
#include "optionpreferences.h"
#include <cstdio>
#include <cstddef>

#ifdef GAME_DLL
#else
GameLODManager *g_theGameLODManager = nullptr;
#endif

// The names and number of strings here correlates to the enums in gamelod.h
const char *g_staticGameLODNames[] = { "Low", "Medium", "High", "Custom" };
const char *g_dynamicGameLODNames[] = { "Low", "Medium", "High", "VeryHigh" };
const char *GameLODManager::s_cpuNames[] = { "XX", "P3", "P4", "K7" };
const char *GameLODManager::s_gpuNames[] = {
    "XX",
    "V2",
    "V3",
    "V4",
    "V5",
    "TNT",
    "TNT2",
    "GF2",
    "R100",
    "PS11",
    "GF3",
    "GF4",
    "PS14",
    "R200",
    "PS20",
    "R300" 
};

void Test_Minimum_Requirements(GPUType *gpu, CPUType *cpu, int *cpu_speed, int *memory, float *int_score, float *float_score, float *mem_score)
{
#ifdef GAME_DLL
    Call_Function<void, GPUType *, CPUType *, int *, int *, float *, float *, float *>(0x0074EB20, gpu, cpu, cpu_speed, memory, int_score, float_score, mem_score);
#else
    *cpu = CPU_P3;
    *gpu = GPU_GF4;
    *cpu_speed = 2500;
    *memory = CPUDetectClass::Get_Available_Physical_Memory() > INT_MAX ? INT_MAX : CPUDetectClass::Get_Available_Physical_Memory();
#endif
}

GameLODManager::GameLODManager() :
    m_staticLODLevel(STATLOD_INVALID),
    m_dynamicLODLevel(DYNLOD_HIGH),
    m_particleSkipCount(0),
    m_particleSkipMask(0),
    m_unkInt2(0),
    m_debrisSkipMask(0),
    m_slowDeathScale(1.0f),
    m_minParticlePriority(PARTICLE_PRIORITY_NONE),
    m_minParticleSkipPriority(PARTICLE_PRIORITY_NONE),
    m_unkBool1(0),
    m_unkBool2(0),
    m_didMemPass(0),
    m_benchProfileCount(0),
    m_idealStaticGameDetail(STATLOD_INVALID),
    m_gpuType(GPU_COUNT),
    m_cpuType(CPU_UNKNOWN),
    m_physicalMem(0),
    m_cpuMHz(0),
    m_integerScore(1.0f),
    m_floatingPointScore(1.0f),
    m_memoryScore(1.0f),
    m_overallScore(0.0f),
    m_textureReductionFactor(0),
    m_reallyLowMHz(400)
{
    for (int i = 0; i < STATLOD_COUNT; ++i) {
        m_staticLOD[i].minimum_fps = 0;
        m_staticLOD[i].minimum_cpu_fps = 0;
        m_staticLOD[i].sample_count_2D = 6;
        m_staticLOD[i].sample_count_3D = 24;
        m_staticLOD[i].stream_count = 2;
        m_staticLOD[i].max_particle_count = 2500;
        m_staticLOD[i].use_shadow_volumes = true;
        m_staticLOD[i].use_shadow_decals = true;
        m_staticLOD[i].use_cloud_map = true;
        m_staticLOD[i].use_light_map = true;
        m_staticLOD[i].show_soft_water_edges = true;
        m_staticLOD[i].max_tank_track_edges = 100;
        m_staticLOD[i].max_tank_track_opaque_edges = 25;
        m_staticLOD[i].max_tank_track_fade_delay = 300000;
        m_staticLOD[i].use_buildup_scaffolds = true;
        m_staticLOD[i].use_tree_sway = true;
        m_staticLOD[i].use_emissive_night_materials = true;
        m_staticLOD[i].use_heat_effects = true;
        m_staticLOD[i].texture_reduction_factor = 0;
        m_staticLOD[i].use_fps_limit = true;
        m_staticLOD[i].use_dynamic_lod = true;
        m_staticLOD[i].use_trees = true;
    }

    for (int i = 0; i < DYNLOD_COUNT; ++i) {
        m_dynamicLOD[i].minimum_fps = 0;
        m_dynamicLOD[i].particle_skip_mask = 0;
        m_dynamicLOD[i].debris_skip_mask = 0;
        m_dynamicLOD[i].slow_death_scale = 1.0f;
        m_dynamicLOD[i].min_particle_priority = PARTICLE_PRIORITY_WEAPON_EXPLOSION;
        m_dynamicLOD[i].min_particle_skip_priority = PARTICLE_PRIORITY_WEAPON_EXPLOSION;
    }

    for (int i = 0; i < STATLOD_COUNT - 1; ++i) {
        for (int j = 0; j < 32; ++j) {
            m_LODPresets[i][j].cpu_type = CPU_UNKNOWN;
            m_LODPresets[i][j].mhz = 1;
            m_LODPresets[i][j].score = 1.0f;
            m_LODPresets[i][j].video_type = GPU_UNKNOWN;
            m_LODPresets[i][j].video_mem = 1;
        }
    }

    for (int i = 0; i < 16; ++i) {
        m_benchProfiles[i].cpu_type = CPU_UNKNOWN;
        m_benchProfiles[i].mhz = 1;
        m_benchProfiles[i].integer_score = 1.0f;
        m_benchProfiles[i].floating_point_score = 1.0f;
        m_benchProfiles[i].memory_score = 1.0f;
    }

    for (int i = 0; i < STATLOD_COUNT - 1; ++i) {
        m_staticLODPresetCount[i] = 0;
    }
}

void GameLODManager::Init()
{
    INI ini;
    ini.Load("Data/INI/GameLOD.ini", INI_LOAD_OVERWRITE, nullptr);
    ini.Load("Data/INI/GameLODPresets.ini", INI_LOAD_OVERWRITE, nullptr);
    Refresh_Custom_Static_LOD();

    OptionPreferences opts;
    StaticGameLODLevel static_detail = (StaticGameLODLevel)opts.Get_Static_Game_Detail();
    m_idealStaticGameDetail = (StaticGameLODLevel)opts.Get_Ideal_Static_Game_Detail();

    Test_Minimum_Requirements(nullptr, &m_cpuType, &m_cpuMHz, &m_physicalMem, nullptr, nullptr, nullptr);

    // If physical memory is greater than 256MiB the test passes. Note that the test currently uses
    // GlobalMemoryStatus on windows and only stores the result in a 32bit int so this test can fail on
    // modern machines.
    if ((float)m_physicalMem / 2.684E8f >= 0.94f) {
        m_didMemPass = true;
    }

#if GAME_ALLOW_BENCHMARK
    // This section benchmarks the system to try and work out the best lod level to use against
    // some existing presets. Mac version does not have the benchmark code and does not
    // bother doing this.
    if ((m_idealStaticGameDetail == STATLOD_INVALID && m_cpuType == CPU_UNKNOWN)
        || g_theWriteableGlobalData->m_writeBenchMarkFile) {
        Test_Minimum_Requirements(
            nullptr, nullptr, nullptr, nullptr, &m_integerScore, &m_floatingPointScore, &m_memoryScore);

        if (g_theWriteableGlobalData->m_writeBenchMarkFile) {
            FILE *bmfile = fopen("Benchmark.txt", "w");

            // Print a benchmark entry to the file we just opened.
            if (bmfile != nullptr) {
                fprintf(bmfile,
                    "BenchProfile = %s %d %f %f %f",
                    s_cpuNames[m_cpuType],
                    m_cpuMHz,
                    m_integerScore,
                    m_floatingPointScore,
                    m_memoryScore);

                fclose(bmfile);
            }
        }

        m_overallScore = m_integerScore + m_floatingPointScore;

        for (int i = 0; i < m_benchProfileCount; ++i) {
            if (m_integerScore / m_benchProfiles[i].integer_score >= 0.94f
                && m_floatingPointScore / m_benchProfiles[i].floating_point_score >= 0.94f
                && m_memoryScore / m_benchProfiles[i].memory_score >= 0.94f) {
                int j;

                for (j = STATLOD_HIGH; j >= STATLOD_LOW; --j) {
                    int k;

                    for (k = 0; k < m_staticLODPresetCount[j]; ++k) {
                        if (m_benchProfiles[i].cpu_type == m_LODPresets[j][k].cpu_type
                            && (double)m_benchProfiles[i].mhz / (double)m_LODPresets[j][k].mhz > 0.94) {
                            break;
                        }
                    }

                    if (k < m_staticLODPresetCount[j]) {
                        break;
                    }
                }

                if (j >= STATLOD_LOW) {
                    m_cpuType = m_benchProfiles[i].cpu_type;
                    m_cpuMHz = m_benchProfiles[i].mhz;
                }
            }
        }
    }
#endif

    if (static_detail == STATLOD_CUSTOM) {
        g_theWriteableGlobalData->m_textureReductionFactor = opts.Get_Texture_Reduction();
        g_theWriteableGlobalData->m_shadowVolumes = opts.Get_3DShadows_Enabled();
        g_theWriteableGlobalData->m_shadowDecals = opts.Get_2DShadows_Enabled();
        g_theWriteableGlobalData->m_useBehindBuildingMarker = opts.Get_Building_Occlusion_Enabled();
        g_theWriteableGlobalData->m_maxParticleCount = opts.Get_Particle_Cap();
        g_theWriteableGlobalData->m_dynamicLOD = opts.Get_Dynamic_LOD_Enabled();
        g_theWriteableGlobalData->m_useFPSLimit = opts.Get_FPSLimit_Enabled();
        g_theWriteableGlobalData->m_useLightMap = opts.Get_Lightmap_Enabled();
        g_theWriteableGlobalData->m_useCloudMap = opts.Get_Cloud_Shadows_Enabled();
        g_theWriteableGlobalData->m_showSoftWaterEdge = opts.Get_Smooth_Water_Enabled();
        g_theWriteableGlobalData->m_useHeatEffects = opts.Get_Use_Heat_Effects();
        g_theWriteableGlobalData->m_extraAnimationsDisabled = opts.Get_Extra_Animations_Disabled();
        g_theWriteableGlobalData->m_useTreeSway = !g_theWriteableGlobalData->m_extraAnimationsDisabled;
        g_theWriteableGlobalData->m_useTrees = opts.Get_Trees_Enabled();
    }

    if (g_theWriteableGlobalData->m_useStaticLODLevels) {
        if (static_detail != STATLOD_INVALID && (static_detail == STATLOD_CUSTOM || m_staticLODLevel != static_detail)) {
            Apply_Static_LOD_Level(static_detail);
            m_staticLODLevel = static_detail;
        }
    } else {
        m_staticLODLevel = STATLOD_CUSTOM;
    }
}

void GameLODManager::Refresh_Custom_Static_LOD()
{
    m_staticLOD[STATLOD_CUSTOM].max_particle_count = g_theWriteableGlobalData->m_maxParticleCount;
    m_staticLOD[STATLOD_CUSTOM].use_shadow_volumes = g_theWriteableGlobalData->m_shadowVolumes;
    m_staticLOD[STATLOD_CUSTOM].use_shadow_decals = g_theWriteableGlobalData->m_shadowDecals;
    m_staticLOD[STATLOD_CUSTOM].use_cloud_map = g_theWriteableGlobalData->m_useCloudMap;
    m_staticLOD[STATLOD_CUSTOM].use_light_map = g_theWriteableGlobalData->m_useLightMap;
    m_staticLOD[STATLOD_CUSTOM].show_soft_water_edges = g_theWriteableGlobalData->m_showSoftWaterEdge;
    m_staticLOD[STATLOD_CUSTOM].max_tank_track_edges = g_theWriteableGlobalData->m_maxTankTrackEdges;
    m_staticLOD[STATLOD_CUSTOM].max_tank_track_opaque_edges = g_theWriteableGlobalData->m_maxTankTrackOpaqueEdges;
    m_staticLOD[STATLOD_CUSTOM].max_tank_track_fade_delay = g_theWriteableGlobalData->m_maxTankTrackFadeDelay;
    m_staticLOD[STATLOD_CUSTOM].use_buildup_scaffolds = !g_theWriteableGlobalData->m_extraAnimationsDisabled;
    m_staticLOD[STATLOD_CUSTOM].use_tree_sway = !g_theWriteableGlobalData->m_extraAnimationsDisabled;
    m_staticLOD[STATLOD_CUSTOM].use_heat_effects = g_theWriteableGlobalData->m_useHeatEffects;
    m_staticLOD[STATLOD_CUSTOM].texture_reduction_factor = g_theWriteableGlobalData->m_textureReductionFactor;
    m_staticLOD[STATLOD_CUSTOM].use_fps_limit = g_theWriteableGlobalData->m_useFPSLimit;
    m_staticLOD[STATLOD_CUSTOM].use_dynamic_lod = g_theWriteableGlobalData->m_dynamicLOD;
    m_staticLOD[STATLOD_CUSTOM].use_trees = g_theWriteableGlobalData->m_useTrees;
}

int GameLODManager::Get_Static_LOD_Index(Utf8String name)
{
    int index = STATLOD_LOW;

    for (; index < STATLOD_COUNT; ++index) {
        if (strcasecmp(name.Str(), g_staticGameLODNames[index]) == 0) {
            break;
        }
    }

    if (index >= STATLOD_COUNT) {
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
    if (m_idealStaticGameDetail != STATLOD_INVALID) {
        return m_idealStaticGameDetail;
    }

    Test_Minimum_Requirements(&m_gpuType, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

    if (m_gpuType == GPU_UNKNOWN) {
        m_gpuType = GPU_TNT2;
    }

    for (int i = STATLOD_HIGH; i >= STATLOD_LOW; --i) {
        for (int j = 0; j < m_staticLODPresetCount[j]; ++j) {
            if (m_cpuType == m_LODPresets[i][j].cpu_type) {
                if ((double)m_cpuMHz / (double)m_LODPresets[i][j].mhz >= 0.94 && m_gpuType >= m_LODPresets[i][j].video_type
                    && (double)(m_physicalMem / 0x100000) / (double)m_LODPresets[i][j].video_mem >= 0.94) {
                    m_idealStaticGameDetail = (StaticGameLODLevel)i;
                    break;
                }
            }
        }

        if (m_idealStaticGameDetail >= i) {
            break;
        }
    }

    OptionPreferences opts;
    opts.Set_Ideal_Static_Game_Detail(m_idealStaticGameDetail);
    opts.Set_Static_Game_Detail(m_idealStaticGameDetail);
    opts.Write();

    return m_idealStaticGameDetail;
}

bool GameLODManager::Set_Static_LOD_Level(StaticGameLODLevel level)
{
    if (!g_theWriteableGlobalData->m_useStaticLODLevels) {
        m_staticLODLevel = STATLOD_CUSTOM;
        return false;
    }

    if (level == STATLOD_INVALID || (level != STATLOD_CUSTOM && m_staticLODLevel == level)) {
        return false;
    }

    Apply_Static_LOD_Level(level);
    m_staticLODLevel = level;

    return true;
}

void GameLODManager::Apply_Static_LOD_Level(StaticGameLODLevel level)
{
#ifdef GAME_DLL
    // TODO requires parts of TerrainVisual and GameClient classes.
    Call_Method<void, GameLODManager, StaticGameLODLevel>(0x0047AC60, this, level);
#endif
}

int GameLODManager::Get_Dynamic_LOD_Index(Utf8String name)
{
    int index = DYNLOD_LOW;

    for (; index < DYNLOD_COUNT; ++index) {
        if (strcasecmp(name.Str(), g_dynamicGameLODNames[index]) == 0) {
            break;
        }
    }

    if (index >= DYNLOD_COUNT) {
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

    while (m_dynamicLOD[level].minimum_fps >= (int64_t)fps) {
        --level;

        if (level < 0) {
            return DYNLOD_LOW;
        }
    }

    return (DynamicGameLODLevel)level;
}

bool GameLODManager::Set_Dynamic_LOD_Level(DynamicGameLODLevel level)
{
    if (level == DYNLOD_INVALID || level == m_dynamicLODLevel) {
        return false;
    }

    Apply_Dynamic_LOD_Level(level);
    m_dynamicLODLevel = level;

    return true;
}

void GameLODManager::Apply_Dynamic_LOD_Level(DynamicGameLODLevel level)
{
    m_particleSkipCount = 0;
    m_particleSkipMask = m_dynamicLOD[level].particle_skip_mask;
    m_unkInt2 = 0;
    m_debrisSkipMask = m_dynamicLOD[level].debris_skip_mask;
    m_slowDeathScale = m_dynamicLOD[level].slow_death_scale;
    m_minParticlePriority = m_dynamicLOD[level].min_particle_priority;
    m_minParticleSkipPriority = m_dynamicLOD[level].min_particle_skip_priority;
}

int GameLODManager::Get_Recommended_Texture_Reduction()
{
    if (m_idealStaticGameDetail == STATLOD_INVALID) {
        Find_Static_LOD_Level();
    }

    if (m_didMemPass) {
        return m_staticLOD[m_idealStaticGameDetail].texture_reduction_factor;
    }

    return m_staticLOD[STATLOD_LOW].texture_reduction_factor;
}

void GameLODManager::Parse_Static_LOD_Definitions(INI *ini)
{
    static FieldParse _static_lod_parsers[] =
    {
        { "MinimumFPS", &INI::Parse_Int, nullptr, offsetof(StaticGameLOD, minimum_fps) },
        { "MinimumProcessorFps", &INI::Parse_Int, nullptr, offsetof(StaticGameLOD, minimum_cpu_fps) },
        { "SampleCount2D", &INI::Parse_Int, nullptr, offsetof(StaticGameLOD, sample_count_2D) },
        { "SampleCount3D", &INI::Parse_Int, nullptr, offsetof(StaticGameLOD, sample_count_3D) },
        { "StreamCount", &INI::Parse_Int, nullptr, offsetof(StaticGameLOD, stream_count) },
        { "MaxParticleCount", &INI::Parse_Int, nullptr, offsetof(StaticGameLOD, max_particle_count) },
        { "UseShadowVolumes", &INI::Parse_Bool, nullptr, offsetof(StaticGameLOD, use_shadow_volumes) },
        { "UseShadowDecals", &INI::Parse_Bool, nullptr, offsetof(StaticGameLOD, use_shadow_decals) },
        { "UseCloudMap", &INI::Parse_Bool, nullptr, offsetof(StaticGameLOD, use_cloud_map) },
        { "UseLightMap", &INI::Parse_Bool, nullptr, offsetof(StaticGameLOD, use_light_map) },
        { "ShowSoftWaterEdge", &INI::Parse_Bool, nullptr, offsetof(StaticGameLOD, show_soft_water_edges) },
        { "MaxTankTrackEdges", &INI::Parse_Int, nullptr, offsetof(StaticGameLOD, max_tank_track_edges) },
        { "MaxTankTrackOpaqueEdges", &INI::Parse_Int, nullptr, offsetof(StaticGameLOD, max_tank_track_opaque_edges) },
        { "MaxTankTrackFadeDelay", &INI::Parse_Int, nullptr, offsetof(StaticGameLOD, max_tank_track_fade_delay) },
        { "UseBuildupScaffolds", &INI::Parse_Bool, nullptr, offsetof(StaticGameLOD, use_buildup_scaffolds) },
        { "UseTreeSway", &INI::Parse_Bool, nullptr, offsetof(StaticGameLOD, use_tree_sway) },
        { "UseEmissiveNightMaterials", &INI::Parse_Bool, nullptr, offsetof(StaticGameLOD, use_emissive_night_materials) },
        { "UseHeatEffects", &INI::Parse_Bool, nullptr, offsetof(StaticGameLOD, use_heat_effects) },
        { "TextureReductionFactor", &INI::Parse_Int, nullptr, offsetof(StaticGameLOD, texture_reduction_factor) },
        { nullptr, nullptr, nullptr, 0 }
    };

    Utf8String token = ini->Get_Next_Token();

    if (g_theGameLODManager != nullptr) {
        int level = g_theGameLODManager->Get_Static_LOD_Index(token);

        if (level != STATLOD_INVALID) {
            ini->Init_From_INI(&g_theGameLODManager->m_staticLOD[level], _static_lod_parsers);
        }
    }
}

void GameLODManager::Parse_Dynamic_LOD_Definitions(INI *ini)
{
    static const char *_particle_prioritiy_names[] =
    {
        "NONE",
        "WEAPON_EXPLOSION",
        "SCORCHMARK",
        "DUST_TRAIL",
        "BUILDUP",
        "DEBRIS_TRAIL",
        "UNIT_DAMAGE_FX",
        "DEATH_EXPLOSION",
        "SEMI_CONSTANT",
        "CONSTANT",
        "WEAPON_TRAIL",
        "AREA_EFFECT",
        "CRITICAL",
        "ALWAYS_RENDER",
        nullptr
    };

    static FieldParse _dynamic_lod_parsers[] =
    {
        { "MinimumFPS", &INI::Parse_Int, nullptr, offsetof(DynamicGameLOD, minimum_fps) },
        { "ParticleSkipMask", &INI::Parse_Int, nullptr, offsetof(DynamicGameLOD, particle_skip_mask) },
        { "DebrisSkipMask", &INI::Parse_Int, nullptr, offsetof(DynamicGameLOD, debris_skip_mask) },
        { "SlowDeathScale", &INI::Parse_Real, nullptr, offsetof(DynamicGameLOD, slow_death_scale) },
        { "MinParticlePriority", &INI::Parse_Index_List, _particle_prioritiy_names, offsetof(DynamicGameLOD, min_particle_priority) },
        { "MinParticleSkipPriority", &INI::Parse_Index_List, _particle_prioritiy_names, offsetof(DynamicGameLOD, min_particle_skip_priority) },
        { nullptr, nullptr, nullptr, 0 }
    };

    Utf8String token = ini->Get_Next_Token();

    if (g_theGameLODManager != nullptr) {
        int level = g_theGameLODManager->Get_Dynamic_LOD_Index(token);

        if (level != STATLOD_INVALID) {
            ini->Init_From_INI(&g_theGameLODManager->m_dynamicLOD[level], _dynamic_lod_parsers);
        }
    }
}

void GameLODManager::Parse_LOD_Preset(INI *ini)
{
    Utf8String token = ini->Get_Next_Token();

    if (g_theGameLODManager != nullptr) {
        int level = g_theGameLODManager->Get_Static_LOD_Index(token);

        if (level != STATLOD_INVALID) {
            int preset_count = g_theGameLODManager->m_staticLODPresetCount[level];

            if (preset_count < MAX_PRESETS) {
                ++g_theGameLODManager->m_staticLODPresetCount[level];

                INI::Parse_Index_List(
                    ini, nullptr, &g_theGameLODManager->m_LODPresets[level][preset_count].cpu_type, s_cpuNames);

                INI::Parse_Int(ini, nullptr, &g_theGameLODManager->m_LODPresets[level][preset_count].mhz, nullptr);

                INI::Parse_Index_List(
                    ini, nullptr, &g_theGameLODManager->m_LODPresets[level][preset_count].video_type, s_gpuNames);

                INI::Parse_Int(ini, nullptr, &g_theGameLODManager->m_LODPresets[level][preset_count].video_mem, nullptr);
            }
        }
    }
}

void GameLODManager::Parse_Bench_Profiles(INI *ini)
{
    if (g_theGameLODManager != nullptr) {
        int bench_count = g_theGameLODManager->m_benchProfileCount;

        if (bench_count < MAX_PROFILES) {
            ++g_theGameLODManager->m_benchProfileCount;
            INI::Parse_Index_List(ini, nullptr, &g_theGameLODManager->m_benchProfiles[bench_count].cpu_type, s_cpuNames);
            INI::Parse_Int(ini, nullptr, &g_theGameLODManager->m_benchProfiles[bench_count].mhz, nullptr);
            INI::Parse_Real(ini, nullptr, &g_theGameLODManager->m_benchProfiles[bench_count].integer_score, nullptr);
            INI::Parse_Real(ini, nullptr, &g_theGameLODManager->m_benchProfiles[bench_count].floating_point_score, nullptr);
            INI::Parse_Real(ini, nullptr, &g_theGameLODManager->m_benchProfiles[bench_count].memory_score, nullptr);
        }
    }
}

void GameLODManager::Parse_Low_MHz(INI *ini)
{
    int mhz;
    INI::Parse_Int(ini, nullptr, &mhz, nullptr);

    if (g_theGameLODManager != nullptr) {
        g_theGameLODManager->m_reallyLowMHz = mhz;
    }
}
