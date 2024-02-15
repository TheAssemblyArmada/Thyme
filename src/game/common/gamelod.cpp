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
#include "gameclient.h"
#include "globaldata.h"
#include "optionpreferences.h"
#include "shadermanager.h"
#include "terrainvisual.h"
#include <cstddef>
#include <cstdio>

#ifndef GAME_DLL
GameLODManager *g_theGameLODManager = nullptr;
#endif

// The names and number of strings here correlates to the enums in gamelod.h
const char *g_staticGameLODNames[] = { "Low", "Medium", "High", "Custom" };
const char *g_dynamicGameLODNames[] = { "Low", "Medium", "High", "VeryHigh" };
const char *GameLODManager::s_cpuNames[] = { "XX", "P3", "P4", "K7", nullptr };
const char *GameLODManager::s_gpuNames[] = {
    "XX", "V2", "V3", "V4", "V5", "TNT", "TNT2", "GF2", "R100", "PS11", "GF3", "GF4", "PS14", "R200", "PS20", "R300", nullptr
};

void Test_Minimum_Requirements(
    GPUType *gpu, CPUType *cpu, int *cpu_speed, int *memory, float *int_score, float *float_score, float *mem_score)
{
    // This implementation just gives the "best" results for cpu and gpu type as modern hardware isn't recognised correctly.
    if (cpu != nullptr) {
        *cpu = CPU_P4;
    }

    if (gpu != nullptr) {
        *gpu = W3DShaderManager::Get_Chipset();
    }

    if (cpu_speed != nullptr) {
        *cpu_speed = 2500;
    }

    // Because memory is returned as an integer and systems with more than 2GB are now common we return a max of 2GB rather
    // than overflowing.
    if (memory != nullptr) {
        *memory = CPUDetectClass::Get_Available_Physical_Memory() > INT_MAX ?
            INT_MAX :
            CPUDetectClass::Get_Available_Physical_Memory();
    }

    // The other values are ignored here and were generated from the integrated nbench benchmarking code if all 3 values were
    // requested.
}

GameLODManager::GameLODManager() :
    m_staticLODLevel(STATLOD_INVALID),
    m_dynamicLODLevel(DYNLOD_HIGH),
    m_particleSkipCount(0),
    m_particleSkipMask(0),
    m_debrisSkipCount(0),
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

    Set_Static_LOD_Level(static_detail);
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
    for (int index = STATLOD_LOW; index < STATLOD_COUNT; index++) {
        if (name.Compare_No_Case(g_staticGameLODNames[index]) == 0) {
            return index;
        }
    }

    captainslog_dbgassert(false, "GameLODManager::Get_Static_LOD_Index - Invalid LOD name '%s'", name.Str());
    return STATLOD_INVALID;
}

const char *GameLODManager::Get_Static_LOD_Name(StaticGameLODLevel level)
{
    return g_staticGameLODNames[level];
}

StaticGameLODLevel GameLODManager::Find_Static_LOD_Level()
{
    if (m_idealStaticGameDetail == STATLOD_INVALID) {

        m_idealStaticGameDetail = STATLOD_LOW;
        Test_Minimum_Requirements(&m_gpuType, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

        if (m_gpuType == GPU_UNKNOWN) {
            m_gpuType = GPU_TNT2;
        }

        for (int i = STATLOD_HIGH; i >= STATLOD_LOW; --i) {
            for (int k = 0; k < m_staticLODPresetCount[i]; ++k) {
                if (m_cpuType == m_LODPresets[i][k].cpu_type) {
                    if ((double)m_cpuMHz / (double)m_LODPresets[i][k].mhz >= 0.94
                        && m_gpuType >= m_LODPresets[i][k].video_type
                        && (double)(m_physicalMem / 0x100000) / (double)m_LODPresets[i][k].video_mem >= 0.94) {
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
    }

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
    StaticGameLODInfo current_lod;

    if (m_staticLODLevel != STATLOD_INVALID) {
        current_lod = m_staticLOD[m_staticLODLevel];
    }

    if (level == STATLOD_CUSTOM) {
        Refresh_Custom_Static_LOD();
    }

    StaticGameLODInfo *new_lod = &m_staticLOD[level];
    int texture_reduction_factor = 0;
    bool use_trees = m_didMemPass;

    if (level == STATLOD_CUSTOM) {
        texture_reduction_factor = new_lod->texture_reduction_factor;
        use_trees = new_lod->use_trees;
    } else if (level >= STATLOD_LOW) {
        texture_reduction_factor = Get_Recommended_Texture_Reduction();
    }

    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_maxParticleCount = new_lod->max_particle_count;
        g_theWriteableGlobalData->m_shadowVolumes = new_lod->use_shadow_volumes;
        g_theWriteableGlobalData->m_shadowDecals = new_lod->use_shadow_decals;

        if (texture_reduction_factor != m_textureReductionFactor) {
            g_theWriteableGlobalData->m_textureReductionFactor = texture_reduction_factor;

            if (g_theGameClient != nullptr) {
                g_theGameClient->Adjust_LOD(0);
            }
        }

        if ((m_staticLODLevel == STATLOD_INVALID || new_lod->use_shadow_volumes != current_lod.use_shadow_volumes
                || new_lod->use_shadow_decals != current_lod.use_shadow_decals)
            && g_theGameClient != nullptr) {
            g_theGameClient->Release_Shadows();
            g_theGameClient->Allocate_Shadows();
        }

        g_theWriteableGlobalData->m_useCloudMap = new_lod->use_cloud_map;
        g_theWriteableGlobalData->m_useLightMap = new_lod->use_light_map;
        g_theWriteableGlobalData->m_showSoftWaterEdge = new_lod->show_soft_water_edges;

        if (m_staticLODLevel == STATLOD_INVALID && new_lod->show_soft_water_edges != current_lod.show_soft_water_edges
            && g_theTerrainVisual != nullptr) {
            g_theTerrainVisual->Set_Shore_Line_Detail();
        }

        g_theWriteableGlobalData->m_maxTankTrackEdges = new_lod->max_tank_track_edges;
        g_theWriteableGlobalData->m_maxTankTrackOpaqueEdges = new_lod->max_tank_track_opaque_edges;
        g_theWriteableGlobalData->m_maxTankTrackFadeDelay = new_lod->max_tank_track_fade_delay;
        g_theWriteableGlobalData->m_useTreeSway = new_lod->use_tree_sway;
        g_theWriteableGlobalData->m_extraAnimationsDisabled = !new_lod->use_buildup_scaffolds;
        g_theWriteableGlobalData->m_useHeatEffects = new_lod->use_heat_effects;
        g_theWriteableGlobalData->m_dynamicLOD = new_lod->use_dynamic_lod;
        g_theWriteableGlobalData->m_useFPSLimit = new_lod->use_fps_limit;
        g_theWriteableGlobalData->m_useTrees = use_trees;
    }

    if (!m_didMemPass || Is_Slow_CPU()) {
        g_theWriteableGlobalData->m_shellMapOn = false;
    }

    if (g_theTerrainVisual != nullptr) {
        g_theTerrainVisual->Set_Terrain_Tracks_Detail();
    }
}

int GameLODManager::Get_Dynamic_LOD_Index(Utf8String name)
{
    for (int index = DYNLOD_LOW; index < DYNLOD_COUNT; index++) {
        if (name.Compare_No_Case(g_dynamicGameLODNames[index]) == 0) {
            return index;
        }
    }

    captainslog_dbgassert(false, "GameLODManager::Get_Dynamic_LOD_Index - Invalid LOD name '%s'", name.Str());

    return DYNLOD_INVALID;
}

const char *GameLODManager::Get_Dynamic_LOD_Name(DynamicGameLODLevel level)
{
    return g_dynamicGameLODNames[level];
}

DynamicGameLODLevel GameLODManager::Find_Dynamic_LOD_Level(float fps)
{
    for (int level = DYNLOD_VERYHIGH; level >= DYNLOD_LOW; level--) {
        if (m_dynamicLOD[level].minimum_fps < fps) {
            return static_cast<DynamicGameLODLevel>(level);
        }
    }

    return DYNLOD_LOW;
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
    m_debrisSkipCount = 0;
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
        // #BUGFIX Test to avoid invalid indexing
        if (m_idealStaticGameDetail > STATLOD_INVALID && m_idealStaticGameDetail < STATLOD_COUNT) {
            return m_staticLOD[m_idealStaticGameDetail].texture_reduction_factor;
        }
    }

    return m_staticLOD[STATLOD_LOW].texture_reduction_factor;
}

// Was originally INI::parseStaticGameLODDefinition
void GameLODManager::Parse_Static_LOD_Definition(INI *ini)
{
    static const FieldParse _static_lod_parsers[] = {
        { "MinimumFPS", &INI::Parse_Int, nullptr, offsetof(StaticGameLODInfo, minimum_fps) },
        { "MinimumProcessorFps", &INI::Parse_Int, nullptr, offsetof(StaticGameLODInfo, minimum_cpu_fps) },
        { "SampleCount2D", &INI::Parse_Int, nullptr, offsetof(StaticGameLODInfo, sample_count_2D) },
        { "SampleCount3D", &INI::Parse_Int, nullptr, offsetof(StaticGameLODInfo, sample_count_3D) },
        { "StreamCount", &INI::Parse_Int, nullptr, offsetof(StaticGameLODInfo, stream_count) },
        { "MaxParticleCount", &INI::Parse_Int, nullptr, offsetof(StaticGameLODInfo, max_particle_count) },
        { "UseShadowVolumes", &INI::Parse_Bool, nullptr, offsetof(StaticGameLODInfo, use_shadow_volumes) },
        { "UseShadowDecals", &INI::Parse_Bool, nullptr, offsetof(StaticGameLODInfo, use_shadow_decals) },
        { "UseCloudMap", &INI::Parse_Bool, nullptr, offsetof(StaticGameLODInfo, use_cloud_map) },
        { "UseLightMap", &INI::Parse_Bool, nullptr, offsetof(StaticGameLODInfo, use_light_map) },
        { "ShowSoftWaterEdge", &INI::Parse_Bool, nullptr, offsetof(StaticGameLODInfo, show_soft_water_edges) },
        { "MaxTankTrackEdges", &INI::Parse_Int, nullptr, offsetof(StaticGameLODInfo, max_tank_track_edges) },
        { "MaxTankTrackOpaqueEdges", &INI::Parse_Int, nullptr, offsetof(StaticGameLODInfo, max_tank_track_opaque_edges) },
        { "MaxTankTrackFadeDelay", &INI::Parse_Int, nullptr, offsetof(StaticGameLODInfo, max_tank_track_fade_delay) },
        { "UseBuildupScaffolds", &INI::Parse_Bool, nullptr, offsetof(StaticGameLODInfo, use_buildup_scaffolds) },
        { "UseTreeSway", &INI::Parse_Bool, nullptr, offsetof(StaticGameLODInfo, use_tree_sway) },
        { "UseEmissiveNightMaterials",
            &INI::Parse_Bool,
            nullptr,
            offsetof(StaticGameLODInfo, use_emissive_night_materials) },
        { "UseHeatEffects", &INI::Parse_Bool, nullptr, offsetof(StaticGameLODInfo, use_heat_effects) },
        { "TextureReductionFactor", &INI::Parse_Int, nullptr, offsetof(StaticGameLODInfo, texture_reduction_factor) },
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

// Was originally INI::parseDynamicGameLODDefinition
void GameLODManager::Parse_Dynamic_LOD_Definition(INI *ini)
{
    static const char *_particle_prioritiy_names[] = { "NONE",
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
        nullptr };

    static const FieldParse _dynamic_lod_parsers[] = {
        { "MinimumFPS", &INI::Parse_Int, nullptr, offsetof(DynamicGameLODInfo, minimum_fps) },
        { "ParticleSkipMask", &INI::Parse_Int, nullptr, offsetof(DynamicGameLODInfo, particle_skip_mask) },
        { "DebrisSkipMask", &INI::Parse_Int, nullptr, offsetof(DynamicGameLODInfo, debris_skip_mask) },
        { "SlowDeathScale", &INI::Parse_Real, nullptr, offsetof(DynamicGameLODInfo, slow_death_scale) },
        { "MinParticlePriority",
            &INI::Parse_Index_List,
            _particle_prioritiy_names,
            offsetof(DynamicGameLODInfo, min_particle_priority) },
        { "MinParticleSkipPriority",
            &INI::Parse_Index_List,
            _particle_prioritiy_names,
            offsetof(DynamicGameLODInfo, min_particle_skip_priority) },
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

// Was originally INI::parseLODPreset
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

// Was originally INI::parseBenchProfile
void GameLODManager::Parse_Bench_Profile(INI *ini)
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

// Was originally parseReallyLowMHz
void GameLODManager::Parse_Really_Low_MHz(INI *ini)
{
    int mhz;
    INI::Parse_Int(ini, nullptr, &mhz, nullptr);

    if (g_theGameLODManager != nullptr) {
        g_theGameLODManager->m_reallyLowMHz = mhz;
    }
}

void GameLODManager::Parse_Static_Game_LOD_Level(INI *ini, void *formal, void *store, const void *user_data)
{
    const char *str = ini->Get_Next_Token();

    for (unsigned int i = 0; i < ARRAY_SIZE(g_staticGameLODNames); i++) {
        if (!strcasecmp(str, g_staticGameLODNames[i])) {
            *static_cast<unsigned int *>(store) = i;
            return;
        }
    }

    captainslog_debug("invalid GameLODLevel token %s -- expected LOW/MEDIUM/HIGH", str);
    throw CODE_06;
}