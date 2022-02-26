/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Command line argument parser.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "commandline.h"
#include "archivefilesystem.h"
#include "globaldata.h"
#include "localfilesystem.h"
#include "version.h"
#include <captainslog.h>
#include <cstdio>
#include <cstring>
#include <sys/stat.h>

using std::fclose;
using std::fopen;
using std::sscanf;
using std::strchr;
using std::strlen;

typedef int (*cmdparse_t)(char **, int);

struct CmdParseStruct
{
    const char *argument;
    cmdparse_t handler;
};

int Parse_Win(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_windowed = true;
    }

    return 1;
}

int Parse_Full_Screen(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_windowed = false;
    }

    return 1;
}

int Parse_Full_Version(char **argv, int argc)
{
    if (g_theVersion != nullptr && argc > 1) {
        g_theVersion->Set_Use_Full_Version(atoi(argv[1]) != 0);

        return 2;
    }

    return 1;
}

int Parse_XRes(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr && argc > 1) {
        g_theWriteableGlobalData->m_xResolution = atoi(argv[1]);

        return 2;
    }

    return 1;
}

int Parse_YRes(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr && argc > 1) {
        g_theWriteableGlobalData->m_yResolution = atoi(argv[1]);

        return 2;
    }

    return 1;
}

int Parse_Script_Debug(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_scriptDebug = true;
        g_theWriteableGlobalData->m_winCursors = true;
    }

    return 1;
}

int Parse_Particle_Edit(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_particleEdit = true;
        g_theWriteableGlobalData->m_winCursors = true;
        g_theWriteableGlobalData->m_windowed = true;
    }

    return 1;
}

int Parse_No_Shell_Map(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_shellMapOn = false;
    }

    return 1;
}

int Parse_No_Shaders(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_chipsetType = 1;
    }

    return 1;
}

int Parse_Quick_Start(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_playIntro = false;
        g_theWriteableGlobalData->m_afterIntro = true;
        g_theWriteableGlobalData->m_playSizzle = false;
        g_theWriteableGlobalData->m_shellMapOn = false;
        g_theWriteableGlobalData->m_animateWindows = false;
    }

    return 1;
}

int Parse_Play_Stats(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr && argc > 1) {
        int32_t tmp;
        // #BUGFIX Check return value
        if (sscanf(argv[1], "%" PRId32, &tmp) == 1) {
            g_theWriteableGlobalData->m_playerStats = tmp;
        }
    }

    return 2;
}

int Parse_Mod(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr && argc > 1) {
        Utf8String path = argv[1];

        // If its not an absolute path, make it relative to user data dir.
        if (!strchr(path.Str(), ':') && !path.Starts_With("/") && !path.Starts_With("\\")) {
            path.Format("%s%s", g_theWriteableGlobalData->m_userDataDirectory.Str(), argv[1]);
        }

        // Check if it exists
        if (g_theLocalFileSystem->Does_File_Exist(path.Str())) {
            struct stat st;

            // Check if its a directory or file and handle accordingly
            if (stat(path.Str(), &st) == 0) {
                if ((st.st_mode & S_IFDIR)) {
                    if (!path.Ends_With("/") && !path.Ends_With("\\")) {
                        path.Concat("/");
                    }

                    g_theWriteableGlobalData->m_userModDirectory = path;
                } else {
                    g_theWriteableGlobalData->m_userModFile = path;
                }
            }
        }

        return 2;
    }

    return 1;
}

int Parse_Use_Wave_Editor(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_useWaveEditor = true;
    }

    return 1;
}

int Parse_No_Music(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_musicOn = false;
    }

    return 1;
}

int Parse_No_Video(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_videoOn = false;
    }

    return 1;
}

int Parse_Use_CSF(char **argv, int argc)
{
    // TODO

    return 1;
}

int Parse_No_Input_Disable(char **argv, int argc)
{
#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_disableScriptedInputDisabling = true;
    }
#endif
    return 1;
}

int Parse_No_Fade(char **argv, int argc)
{
#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_disableCameraFade = true;
    }
#endif
    return 1;
}

int Parse_No_Military_Caption(char **argv, int argc)
{
#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_disableMilitaryCaption = true;
    }
#endif
    return 1;
}

int Parse_Debug_CRC_From_Frame(char **argv, int argc)
{
#ifdef GAME_DEBUG_STRUCTS
    // TODO
#endif
    return 2;
}

int Parse_Debug_CRC_Until_Frame(char **argv, int argc)
{
#ifdef GAME_DEBUG_STRUCTS
    // TODO
#endif
    return 2;
}

int Parse_Keep_CRC_Saves(char **argv, int argc)
{
#ifdef GAME_DEBUG_STRUCTS
    // TODO
#endif
    return 1;
}

int Parse_CRC_Logic_Module_Data(char **argv, int argc)
{
#ifdef GAME_DEBUG_STRUCTS
    // TODO
#endif
    return 1;
}

int Parse_CRC_Client_Module_Data(char **argv, int argc)
{
#ifdef GAME_DEBUG_STRUCTS
    // TODO
#endif
    return 1;
}

int Parse_Client_Deep_CRC(char **argv, int argc)
{
#ifdef GAME_DEBUG_STRUCTS
    // TODO
#endif
    return 1;
}

int Parse_Verify_Client_CRC(char **argv, int argc)
{
#ifdef GAME_DEBUG_STRUCTS
    // TODO
#endif
    return 1;
}

int Parse_Log_Object_CRC(char **argv, int argc)
{
#ifdef GAME_DEBUG_STRUCTS
    // TODO
#endif
    return 1;
}

int Parse_Net_CRC_Interval(char **argv, int argc)
{
#ifdef GAME_DEBUG_STRUCTS
    // TODO
#endif
    return 2;
}

int Parse_Replay_CRC_Interval(char **argv, int argc)
{
#ifdef GAME_DEBUG_STRUCTS
    // TODO
#endif
    return 2;
}

int Parse_Sync(char **argv, int argc)
{
#ifdef GAME_DEBUG_STRUCTS
    // TODO
#endif
    return 1;
}

int Parse_No_Draw(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_frameToJumpTo = 1;
    }

    return 1;
}

int Parse_Log_To_Console(char **argv, int argc)
{
    // Needs refactoring of the logging module.

    return 1;
}

int Parse_No_Audio(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_audioOn = false;
        g_theWriteableGlobalData->m_speechOn = false;
        g_theWriteableGlobalData->m_soundsOn = false;
        g_theWriteableGlobalData->m_musicOn = false;
    }

    return 1;
}

int Parse_No_Shadows(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_shadowVolumes = false;
        g_theWriteableGlobalData->m_shadowDecals = false;
    }

    return 1;
}

void Parse_Map_Name(Utf8String &name)
{
    // TODO
}

int Parse_Map(char **argv, int argc)
{
    if (g_theWriteableGlobalData && argc > 1) {
        g_theWriteableGlobalData->m_mapName = argv[1];
        Parse_Map_Name(g_theWriteableGlobalData->m_mapName);
    }

    return 2;
}

int Parse_File(char **argv, int argc)
{
    if (g_theWriteableGlobalData && argc > 1) {
        g_theWriteableGlobalData->m_initialFile = argv[1];
        Parse_Map_Name(g_theWriteableGlobalData->m_initialFile);
    }

    return 2;
}

int Parse_Latency_Average(char **argv, int argc)
{
#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData && argc > 1) {
        int32_t tmp;
        sscanf(argv[1], "%" PRId32, &tmp);
        g_theWriteableGlobalData->m_latencyAverage = tmp;
    }
#endif
    return 2;
}

int Parse_Latency_Amplitude(char **argv, int argc)
{
#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData && argc > 1) {
        int32_t tmp;
        sscanf(argv[1], "%" PRId32, &tmp);
        g_theWriteableGlobalData->m_latencyAmplitude = tmp;
    }
#endif
    return 2;
}

int Parse_Latency_Period(char **argv, int argc)
{
#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData && argc > 1) {
        int32_t tmp;
        sscanf(argv[1], "%" PRId32, &tmp);
        g_theWriteableGlobalData->m_latencyPeriod = tmp;
    }
#endif
    return 2;
}

int Parse_Latency_Noise(char **argv, int argc)
{
#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData && argc > 1) {
        int32_t tmp;
        sscanf(argv[1], "%" PRId32, &tmp);
        g_theWriteableGlobalData->m_latencyNoise = tmp;
    }
#endif
    return 2;
}

int Parse_Packet_Loss(char **argv, int argc)
{
#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData && argc > 1) {
        int32_t tmp;
        sscanf(argv[1], "%" PRId32, &tmp);
        g_theWriteableGlobalData->m_packetLoss = tmp;
    }
#endif
    return 2;
}

int Parse_Low_Detail(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_terrainLOD = TERRAIN_LOD_MIN;
    }

    return 1;
}

int Parse_No_Dynamic_LOD(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_dynamicLOD = false;
    }

    return 1;
}

int Parse_No_Static_LOD(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_useStaticLODLevels = false;
    }

    return 1;
}

int Parse_FPS(char **argv, int argc)
{
    if (g_theWriteableGlobalData && argc > 1) {
        int32_t tmp;
        // #BUGFIX Check return value
        if (sscanf(argv[1], "%" PRId32, &tmp) == 1) {
            g_theWriteableGlobalData->m_framesPerSecondLimit = tmp;
        }
    }

    return 2;
}

int Parse_No_View_Limit(char **argv, int argc)
{
#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_useCameraConstraints = false;
    }
#endif
    return 1;
}

int Parse_Wire_Frame(char **argv, int argc)
{
#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_wireframe = true;
    }
#endif
    return 1;
}

int Parse_Show_Collision(char **argv, int argc)
{
#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_showCollisionExtents = true;
    }
#endif
    return 1;
}

int Parse_No_Show_Client_Physics(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_showClientPhysics = false;
    }

    return 1;
}

int Parse_Show_Terrain_Normals(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_showTerrainNormals = true;
    }

    return 1;
}

int Parse_State_Machine_Debug(char **argv, int argc)
{
#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_showCollisionExtents = true;
    }
#endif
    return 1;
}

int Parse_Jabber(char **argv, int argc)
{
#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_jabberOn = true;
    }
#endif
    return 1;
}

int Parse_Munkee(char **argv, int argc)
{
#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_munkeeOn = true;
    }
#endif
    return 1;
}

int Parse_Build_Map_Cache(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_buildMapCache = true;
    }

    return 1;
}

int Parse_Preload(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_preloadAssets = true;
    }

    return 1;
}

int Parse_Display_Debug(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_displayDebug = true;
    }

    return 1;
}

int Parse_Preload_Everything(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_preloadAssets = true;
        g_theWriteableGlobalData->m_preloadEverything = true;
    }

    return 1;
}

int Parse_Log_Assets(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        // Precreate the file? Needs access to working directory to work?
        FILE *fp = fopen("PreloadedAssets.txt", "w");

        if (fp != nullptr) {
            fclose(fp);
        }

        g_theWriteableGlobalData->m_logAssets = true;
    }

    return 1;
}

int Parse_VTune(char **argv, int argc)
{
#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_vTune = true;
    }
#endif
    return 1;
}

int Parse_No_FX(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_useFX = false;
    }

    return 1;
}

int Parse_No_Shroud(char **argv, int argc)
{
#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_shroudOn = false;
    }
#endif
    return 1;
}

int Parse_Force_Benchmark(char **argv, int argc)
{
    // Probably remove this at some point as the benchmark code is removed in Thyme.
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_writeBenchMarkFile = true;
    }

    return 1;
}

int Parse_No_Cinematics(char **argv, int argc)
{
#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_disableCameraMovements = true;
        g_theWriteableGlobalData->m_disableMilitaryCaption = true;
        g_theWriteableGlobalData->m_disableCameraFade = true;
        g_theWriteableGlobalData->m_disableScriptedInputDisabling = true;
    }
#endif
    return 1;
}

int Parse_No_Move_Camera(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_disableCameraMovements = true;
    }

    return 1;
}

int Parse_No_Logo(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_playIntro = false;
        g_theWriteableGlobalData->m_afterIntro = true;
        g_theWriteableGlobalData->m_playSizzle = false;
    }

    return 1;
}

int Parse_Shell_Map(char **argv, int argc)
{
    if (g_theWriteableGlobalData && argc > 1) {
        g_theWriteableGlobalData->m_shellMapName = argv[1];
    }

    return 2;
}

int Parse_No_Window_Animation(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_animateWindows = false;
    }

    return 1;
}

int Parse_Win_Cursors(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_winCursors = true;
    }

    return 1;
}

int Parse_Constant_Debug(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_constantDebugUpdate = false;
    }

    return 1;
}

int Parse_Extra_Logging(char **argv, int argc)
{
#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_extraLogging = false;
    }
#endif
    return 1;
}

int Parse_Do_Team_Dot(char **argv, int argc)
{
#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_doStatDump = false;
    }
#endif
    return 1;
}

int Parse_Select_Unselectable(char **argv, int argc)
{
#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_selectTheUnselectable = false;
    }
#endif
    return 1;
}

int Parse_Run_Ahead(char **argv, int argc)
{
#ifdef GAME_DEBUG_STRUCTS
    // TODO
    // Needs extra globals.
#endif
    return 1;
}

int Parse_Seed(char **argv, int argc)
{
    if (g_theWriteableGlobalData && argc > 1) {
        int32_t tmp;
        // #BUGFIX Test return value
        if (sscanf(argv[1], "%" PRId32, &tmp) == 1) {
            g_theWriteableGlobalData->m_fixedSeed = tmp;
        }
    }

    return 2;
}

int Parse_Incr_AGP_Buf(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_setMinVertexBufferSize = true;
    }

    return 1;
}

int Parse_Net_Min_Players(char **argv, int argc)
{
    if (g_theWriteableGlobalData && argc > 1) {
        int32_t tmp;
        // #BUGFIX Check return value
        if (sscanf(argv[1], "%" PRId32, &tmp) == 1) {
            g_theWriteableGlobalData->m_netMinPlayers = tmp;
        }
    }

    return 2;
}

int Parse_Demo_Load_Screen(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_loadScreenDemo = true;
    }

    return 1;
}

int Parse_Save_Stats(char **argv, int argc)
{
#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData && argc > 1) {
        g_theWriteableGlobalData->m_saveStats = true;
        g_theWriteableGlobalData->m_baseStatsDir = argv[1];
    }
#endif
    return 2;
}

int Parse_Save_All_Stats(char **argv, int argc)
{
#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData && argc > 1) {
        g_theWriteableGlobalData->m_saveStats = true;
        g_theWriteableGlobalData->m_baseStatsDir = argv[1];
        g_theWriteableGlobalData->m_saveAllStats = true;
    }
#endif
    return 2;
}

int Parse_Local_MOTD(char **argv, int argc)
{
#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData && argc > 1) {
        g_theWriteableGlobalData->m_useLocalMOTD = true;
        g_theWriteableGlobalData->m_localMOTDPath = argv[1];
    }
#endif
    return 2;
}

int Parse_Camera_Debug(char **argv, int argc)
{
#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_cameraDebug = false;
    }
#endif
    return 1;
}

int Parse_Benchmark(char **argv, int argc)
{
#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData && argc > 1) {
        int32_t tmp;
        sscanf(argv[1], "%" PRId32, &tmp);
        g_theWriteableGlobalData->m_benchmarkTimer = tmp;
        sscanf(argv[1], "%" PRId32, &tmp);
        g_theWriteableGlobalData->m_playerStats = tmp;
    }
#endif
    return 3;
}

int Parse_Stats(char **argv, int argc)
{
#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData && argc > 1) {
        g_theWriteableGlobalData->m_doStats = true;
        int32_t tmp;
        sscanf(argv[1], "%" PRId32, &tmp);
        g_theWriteableGlobalData->m_statsInterval = tmp;
    }
#endif
    return 2;
}

int Parse_Ignore_Asserts(char **argv, int argc)
{
#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_ignoreAsserts = true;
    }
#endif
    return 1;
}

int Parse_Ignore_Stack_Trace(char **argv, int argc)
{
#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_ignoreStackTrace = true;
    }
#endif
    return 1;
}

int Parse_No_FPS_Limit(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_useFPSLimit = true;
        g_theWriteableGlobalData->m_framesPerSecondLimit = 30000;
    }

    return 1;
}

int Parse_Dump_Asset_Usage(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_dumpAssetUsage = true;
    }

    return 1;
}

int Parse_Jump_To_Frame(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_useFPSLimit = true;
        g_theWriteableGlobalData->m_framesPerSecondLimit = 30000;
        int32_t tmp;
        // #BUGFIX Check return value
        if (sscanf(argv[1], "%" PRId32, &tmp) == 1) {
            g_theWriteableGlobalData->m_frameToJumpTo = tmp;
        }
    }

    return 2;
}

int Parse_Update_Images(char **argv, int argc)
{
    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_updateTGAtoDDS = true;
    }

    return 1;
}

// Parses the command line passed to the executable via argc and argv.
void Parse_Command_Line(int argc, char *argv[])
{
    // Arguments we support and the functions to parse them.
    CmdParseStruct _params[] = { { "-noshellmap", &Parse_No_Shell_Map },
        { "-win", &Parse_Win },
        { "-xres", &Parse_XRes },
        { "-yres", &Parse_YRes },
        { "-fullscreen", &Parse_Full_Screen }, // Default
        { "-fullVersion", &Parse_Full_Version },
        { "-particleEdit", &Parse_Particle_Edit },
        { "-scriptDebug", &Parse_Script_Debug },
        { "-playStats", &Parse_Play_Stats },
        { "-mod", &Parse_Mod },
        { "-noshaders", &Parse_No_Shaders },
        { "-quickstart", &Parse_Quick_Start },
        { "-useWaveEditor", &Parse_Use_Wave_Editor },
        { "-noaudio", &Parse_No_Audio },
        { "-map", &Parse_Map },
        { "-nomusic", &Parse_No_Music },
        { "-novideo", &Parse_No_Video },
        { "-noShowClientPhysics", &Parse_No_Show_Client_Physics },
        { "-noViewLimit", &Parse_No_View_Limit },
        { "-lowDetail", &Parse_Low_Detail },
        { "-noDynamicLOD", &Parse_No_Dynamic_LOD },
        { "-noStaticLOD", &Parse_No_Static_LOD },
        { "-noshadowvolumes", &Parse_No_Shadows },
        { "-showTerrainNormals", &Parse_Show_Terrain_Normals },
        { "-buildmapcache", &Parse_Build_Map_Cache },
        { "-preload", &Parse_Preload },
        { "-preloadEverything", &Parse_Preload_Everything },
        { "-logAssets", &Parse_Log_Assets },
        { "-displayDebug", &Parse_Display_Debug },
        { "-file", &Parse_File },
        { "-nofx", &Parse_No_FX },
        { "-forceBenchmark", &Parse_Force_Benchmark },
        { "-nomovecamera", &Parse_No_Move_Camera },
        { "-nologo", &Parse_No_Logo },
        { "-shellmap", &Parse_Shell_Map }, // Overrides the shell map with custom map name.
        { "-noShellAnim", &Parse_No_Window_Animation },
        { "-winCursors", &Parse_Win_Cursors }, // Default anyhow so will have no effect.
        { "-constantDebug", &Parse_Constant_Debug },
        { "-seed", &Parse_Seed }, // Useful for debugging randomness.
        { "-noagpfix", &Parse_Incr_AGP_Buf },
        { "-netMinPlayers", &Parse_Net_Min_Players },
        { "-DemoLoadScreen", &Parse_Demo_Load_Screen },
        { "-noFPSLimit", &Parse_No_FPS_Limit },
        { "-fps", &Parse_FPS },
        { "-dumpAssetUsage", &Parse_Dump_Asset_Usage },
        { "-jumpToFrame", &Parse_Jump_To_Frame },
        { "-updateImages", &Parse_Update_Images },
        { "-noDraw", &Parse_No_Draw },
        // Options below here needs additional members and/or code to work.
        { "-benchmark", &Parse_Benchmark },
        { "-stats", &Parse_Stats },
        { "-saveStats", &Parse_Save_Stats },
        { "-localMOTD", &Parse_Local_MOTD },
        { "-UseCSF", &Parse_Use_CSF },
        { "-NoInputDisable", &Parse_No_Input_Disable },
        { "-DebugCRCFromFrame", &Parse_Debug_CRC_From_Frame },
        { "-DebugCRCUntilFrame", &Parse_Debug_CRC_Until_Frame },
        { "-KeepCRCSaves", &Parse_Keep_CRC_Saves },
        { "-CRCLogicModuleData", &Parse_CRC_Logic_Module_Data },
        { "-CRCClientModuleData", &Parse_CRC_Client_Module_Data },
        { "-ClientDeepCRC", &Parse_Client_Deep_CRC },
        { "-VerifyClientCRC", &Parse_Verify_Client_CRC },
        { "-LogObjectCRCs", &Parse_Log_Object_CRC },
        { "-saveAllStats", &Parse_Save_All_Stats },
        { "-NetCRCInterval", &Parse_Net_CRC_Interval },
        { "-ReplayCRCInterval", &Parse_Replay_CRC_Interval },
        { "-nomilcap", &Parse_No_Military_Caption },
        { "-nofade", &Parse_No_Fade },
        { "-nocinematic", &Parse_No_Cinematics },
        { "-packetloss", &Parse_Packet_Loss },
        { "-latAvg", &Parse_Latency_Average },
        { "-latAmp", &Parse_Latency_Amplitude },
        { "-latPeriod", &Parse_Latency_Period },
        { "-latNoise", &Parse_Latency_Noise },
        { "-wireframe", &Parse_Wire_Frame },
        { "-showCollision", &Parse_Show_Collision },
        { "-stateMachineDebug", &Parse_State_Machine_Debug },
        { "-jabber", &Parse_Jabber },
        { "-munkee", &Parse_Munkee },
        { "-cameraDebug", &Parse_Camera_Debug },
        { "-ignoreAsserts", &Parse_Ignore_Asserts },
        { "-ignoreStackTrace", &Parse_Ignore_Stack_Trace },
        { "-logToCon", &Parse_Log_To_Console },
        { "-vTune", &Parse_VTune },
        { "-selectTheUnselectable", &Parse_Select_Unselectable },
        { "-RunAhead", &Parse_Run_Ahead },
        { "-noshroud", &Parse_No_Shroud },
        { "-ignoresync", &Parse_Sync },
        { "-showTeamDot", &Parse_Do_Team_Dot },
        { "-extraLogging", &Parse_Extra_Logging } };

    // Starting with argument 1 (0 being the name of the binary in most cases)
    // compare the argument against the list of argument handlers and call
    // it if a match is found.
    int arg = 1;

    while (arg < argc) {
        bool parsed = false;

        for (unsigned int i = 0; i < ARRAY_SIZE(_params); ++i) {
            if (strlen(_params[i].argument) == strlen(argv[arg])
                && strncasecmp(argv[arg], _params[i].argument, strlen(_params[i].argument)) == 0) {
                arg += _params[i].handler(&argv[arg], argc - arg);
                parsed = true;
                break;
            }
        }

        if (!parsed) {
            ++arg;
        }
    }

    // Loads any mod big files that were specified on the command line.
    g_theArchiveFileSystem->Load_Mods();
}
