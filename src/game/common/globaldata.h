////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: GLOBALDATA.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Class for handling various global variables
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

#ifndef GLOBALDATA_H
#define GLOBALDATA_H

#include "subsysteminterface.h"
#include "asciistring.h"
#include "color.h"
#include "coord.h"
#include "ini.h"
#include "money.h"

class INI;
class WeaponBonusSet;

#define LIGHT_COUNT 3

// These four enums need moving when we work out where they should go.
enum TimeOfDayType
{
    TIME_OF_DAY_INVALID = 0,
    TIME_OF_DAY_FIRST = 1,
    TIME_OF_DAY_MORNING = 1,
    TIME_OF_DAY_AFTERNOON = 2,
    TIME_OF_DAY_EVENING = 3,
    TIME_OF_DAY_NIGHT = 4,
    TIME_OF_DAY_COUNT,
};

enum WeatherType
{
    WEATHER_NORMAL = 0,
    WEATHER_SNOWY,
    WEATHER_COUNT,
};

enum TerrainLOD
{
    TERRAIN_LOD_INVALID = 0,
    TERRAIN_LOD_MIN,
    TERRAIN_LOD_STRETCH_NO_CLOUDS,
    TERRAIN_LOD_HALF_CLOUDS,
    TERRAIN_LOD_NO_CLOUDS,
    TERRAIN_LOD_STRETCH_CLOUDS,
    TERRAIN_LOD_NO_WATER,
    TERRAIN_LOD_MAX,
    TERRAIN_LOD_AUTOMATIC,
    TERRAIN_LOD_DISABLE,
    TERRAIN_LOD_NUM_TYPES,
};

enum BodyDamageType
{
    BODY_PRISTINE = 0,
    BODY_DAMAGED,
    BODY_REALLYDAMAGED,
    BODY_RUBBLE,
    BODY_COUNT,
};

class GlobalData : public SubsystemInterface
{
public:
    struct TerrainLighting
    {
        RGBColor ambient;
        RGBColor diffuse;
        Coord3D lightPos;
    };

    GlobalData();
    virtual ~GlobalData();

    // m_subsystemInterface implementation
    virtual void Init() {}
    virtual void Reset();
    virtual void Update() {}

    bool Set_Time_Of_Day(TimeOfDayType time);

    static void Parse_Game_Data_Definitions(INI *ini);
    static void Hook_Me();
    // Looks like members are likely public or there would have been a lot of
    // getters/setters.
    // pad indicates where padding will be added to keep 4 byte alignment
    // useful if we want to cram any extra variables in without breaking ABI
public:
    AsciiString m_mapName;
    AsciiString m_moveHintName;
    bool m_useTrees;
    bool m_unkBool1;
    bool m_extraAnimations;
    bool m_useHeatEffects;
    bool m_useFPSLimit;
    bool m_dumpAssetUsage;
    //char pad[2]
    int32_t m_framesPerSecondLimit;
    int32_t m_chipsetType;
    bool m_windowed;
    //char pad[3]
    int32_t m_xResolution;
    int32_t m_yResolution;
    int32_t m_maxShellScreens;
    bool m_useCloudMap;
    //char pad[3]
    int32_t m_use3WayTerrainBlends;   //Should be bool? m_if so, fix when all GlobalData reading code implemented.
    bool m_useLightMap;
    bool m_bilinearTerrainTexture;
    bool m_trilinearTerrainTexture;
    bool m_multiPassTerrain;
    bool m_adjustCliffTextures;
    bool m_stretchTerrain;
    bool m_useHalfHeightMap;
    bool m_drawEntireTerrain;
    TerrainLOD m_terrainLOD;
    bool m_dynamicLOD;
    bool m_unkBool5;
    //char pad[2]
    int32_t m_terrainLODTargetTimeMS;
    bool m_alternateMouseEnabled;
    bool m_retaliationModeEnabled;
    bool m_doubleClickAttackMovesEnabled;
    bool m_rightMouseAlwaysScrolls;
    bool m_useWaterPlane;
    bool m_useCloudPlane;
    bool m_shadowVolumes;
    bool m_shadowDecals;
    int32_t m_textureReductionFactor;
    bool m_useBehindBuildingMarker;
    //char pad[3]
    float m_waterPositionX;
    float m_waterPositionY;
    float m_waterPositionZ;
    float m_waterExtentX;
    float m_waterExtentY;
    int32_t m_waterType;
    bool m_showSoftWaterEdge;
    bool m_unkBool6;
    bool m_unkBool7;
    //char pad[1]
    float m_featherWater;
    AsciiString m_vertexWaterAvailableMaps[4];
    float m_vertexWaterHeightClampLow[4];
    float m_vertexWaterHeightClampLHigh[4];
    float m_vertexWaterAngle[4];
    float m_vertexWaterXPos[4];
    float m_vertexWaterYPos[4];
    float m_vertexWaterZPos[4];
    int32_t m_vertexWaterXGridCells[4];
    int32_t m_vertexWaterYGridCells[4];
    float m_vertexWaterGridSize[4];
    float m_vertexWaterAttenuationA[4];
    float m_vertexWaterAttenuationB[4];
    float m_vertexWaterAttenuationC[4];
    float m_vertexWaterAttenuationRange[4];
    float m_downWindAngle;
    float m_skyBoxPositionZ;
    bool m_drawSkyBox;    // init code suggests this might be an int, old BOOL typedef?
    //char pad[3]
    float m_skyBoxScale;
    float m_cameraPitch;
    float m_cameraYaw;
    float m_cameraHeight;
    float m_maxCameraHeight;
    float m_minCameraHeight;
    float m_terrainHeightAtMapEdge;
    float m_unitDamagedThreshold;
    float m_unitReallyDamagedThreshold;
    float m_groundStiffness;
    float m_structureStiffness;
    float m_gravity;
    float m_stealthFriendlyOpacity;
    uint32_t m_defaultOcclusionDelay;
    bool m_unkBool12;
    bool m_unkBool13;
    bool m_unkBool14;
    //char pad[1]
    float m_partitionCellSize;
    Coord3D m_ammoPipWorldOffset;
    Coord3D m_containerPipWorldOffset;
    Coord2D m_ammoPipScreenOffset;
    Coord2D m_containerPipScreenOffset;
    float m_ammoPipScaleFactor;
    float m_containerPipScaleFactor;
    uint32_t m_historicDamageLimit;
    int32_t m_maxTerrainTracks;
    int32_t m_unkInt2;
    int32_t m_unkInt3;
    int32_t m_unkInt4;
    AsciiString m_levelGainAnimName;
    float m_levelGainAnimTime;
    float m_levelGainAnimZRise;
    AsciiString m_getHealedAnimName;
    float m_getHealedAnimTime;
    float m_getHealedAnimZRise;
    TimeOfDayType m_timeOfDay;
    WeatherType m_weather;
    bool m_makeTrackMarks;
    bool m_hideGarrisonFlags;
    bool m_forceModelsFollowTimeOfDay;
    bool m_forceModelsFollowWeather;
    TerrainLighting m_terrainPlaneLighting[TIME_OF_DAY_COUNT][LIGHT_COUNT];
    TerrainLighting m_terrainObjectLighting[TIME_OF_DAY_COUNT][LIGHT_COUNT];
    RGBColor m_terrainAmbient[LIGHT_COUNT];
    RGBColor m_terrainDiffuse[LIGHT_COUNT];
    Coord3D m_terrainLightPos[LIGHT_COUNT];
    float m_unkFloat2;
    float m_infantryLightMorningScale;
    float m_infantryLightAfternoonScale;
    float m_infantryLightEveningScale;
    float m_infantryLightNightScale;
    float m_unkFloat3;
    float m_easySoloHumanHealthBonus;
    float m_normalSoloHumanHealthBonus;
    float m_hardSoloHumanHealthBonus;
    float m_easySoloAIHealthBonus;
    float m_normalSoloAIHealthBonus;
    float m_hardSoloAIHealthBonus;
    int32_t m_maxTranslucencyObjects;
    int32_t m_unkInt5;    // These 3 are probably some kind of max and get set to 512
    int32_t m_unkInt6;
    int32_t m_unkInt7;
    float m_occludedColorLuminanceScale;
    int32_t m_numberGlobalLights;
    int32_t m_maxRoadSegments;
    int32_t m_maxRoadVertex;
    int32_t m_maxRoadIndex;
    int32_t m_maxRoadTypes;
    bool m_audioOn;
    bool m_musicOn;
    bool m_soundsOn;
    bool m_sounds3DOn;
    bool m_speechOn;
    bool m_videoOn;
    bool m_disableCameraMovements;
    bool m_fogOfWarOn;    // not 100% sure about this, needs confirming
    bool m_showClientPhysics;
    bool m_showTerrainNormals;
    //char pad[2]
    float m_unkFloat4;
    int32_t m_debugAI;    // Possibly old BOOL typedef for int?, keep int for ABI compat until sure
    bool m_unkBool8;
    bool m_debugObstacleAI;
    bool m_showObjectHealth;
    bool m_scriptDebug;   // Requires DebugWindow.dll to do anything
    bool m_particleEdit;  // Requires ParticleEditor.dll to do anything
    bool m_displayDebug;  // not 100% sure and needs confirming
    bool m_winCursors;
    bool m_unkBool9;
    bool m_benchMark;
    bool m_unkBool11;
    //char pad[2]
    int32_t m_fixedSeed;
    float m_particleScale;
    AsciiString m_autoFireParticleSmallPrefix;
    AsciiString m_autoFireParticleSmallSystem;
    int32_t m_autoFireParticleSmallMax;
    AsciiString m_autoFireParticleMediumPrefix;
    AsciiString m_autoFireParticleMediumSystem;
    int32_t m_autoFireParticleMediumMax;
    AsciiString m_autoFireParticleLargePrefix;
    AsciiString m_autoFireParticleLargeSystem;
    int32_t m_autoFireParticleLargeMax;
    AsciiString m_autoSmokeParticleSmallPrefix;
    AsciiString m_autoSmokeParticleSmallSystem;
    int32_t m_autoSmokeParticleSmallMax;
    AsciiString m_autoSmokeParticleMediumPrefix;
    AsciiString m_autoSmokeParticleMediumSystem;
    int32_t m_autoSmokeParticleMediumMax;
    AsciiString m_autoSmokeParticleLargePrefix;
    AsciiString m_autoSmokeParticleLargeSystem;
    int32_t m_autoSmokeParticleLargeMax;
    AsciiString m_autoAFlameParticlePrefix;
    AsciiString m_autoAFlameParticleSystem;
    int32_t m_autoAFlameParticleMax;
    int32_t m_netMinPlayers;  // not 100% sure, needs confirming
    int32_t m_lanIPAddress;
    int32_t m_firewallBehaviour;
    bool m_sendDelay;
    //char pad[3]
    int32_t m_firewallPortOverrides;
    int16_t m_firewallPortAllocationDelta;
    //char pad[2]
    int32_t m_valuesPerSupplyBox;
    float m_buildSpeed;
    float m_minDistanceFromMapEdgeForBuild;
    int32_t m_supplyBuildOrder;
    float m_allowedHeightVariationForBuildings;
    float m_minLowEnergyProductionSpeed;
    float m_maxLowEnergyProductionSpeed;
    float m_lowEnergyPenaltyModifier;
    float m_multipleFactory;
    float m_refundPercent;
    float m_commandCenterHealRange;
    float m_commandCenterHealAmmount;
    int32_t m_maxLineBuildObjects;
    int32_t m_maxTunnelCapacity;
    float m_horizontalScrollSpeedFactor;
    float m_verticalScrollSpeedFactor;
    float m_scrollAmountCutoff;
    float m_cameraAdjustSpeed;
    bool m_enforceMaxCameraHeight;
    bool m_buildMapCache; // not 100% sure, needs confirming
    //char pad[2]
    AsciiString m_initialFile; // not 100% sure, needs confirming
    AsciiString m_pendingFile; // not 100% sure, needs confirming
    int32_t m_maxParticleCount;
    int32_t m_maxFieldParticleCount;
    WeaponBonusSet *m_weaponBonusSetPtr;
    float m_unkFloat5;
    float m_veteranHealthBonus;
    float m_eliteHealthBonus;
    float m_heroicHealthBonus;
    float m_defaultStructureRubbleHeight;
    AsciiString m_shellMapName;
    bool m_shellMapOn;
    bool m_playIntro;
    bool m_playSizzle;
    bool m_afterIntro;
    bool m_unkBool16;
    bool m_unkBool17;
    //char pad[2]
    float m_keyboardScrollFactor;
    float m_keyboardDefaultScrollFactor;
    float m_musicVolumeFactor;  // not 100% sure, needs confirming
    float m_SFXVolumeFactor;  // not 100% sure, needs confirming
    float m_voiceVolumeFactor;  // not 100% sure, needs confirming
    bool m_sound3DPref;  // not 100% sure, needs confirming
    bool m_animateWindows;
    bool m_setMinVertextBufferSize;  // not 100% sure, needs confirming
    //char pad[1]
    uint32_t m_iniCRC;
    uint32_t m_gameCRC;
    BodyDamageType m_movementPenaltyDamageState;
    int32_t m_groupSelectMinSelectSize;
    float m_groupSelectVolumeBase;
    float m_groupSelectVolumeIncrement;
    int32_t m_maxUnitSelectSounds;
    float m_selectionFlashSaturationFactor;
    bool m_selectionFlashHouseColor;
    //char pad[3]
    float m_cameraAudibleRadius;
    float m_groupMoveClickToGatherAreaFactor;
    int32_t m_antiAliasBoxValue;     // could be float
    bool m_languageFilter;
    bool m_unkBool20;
    bool m_unkBool21;
    bool m_saveCameraInReplays;
    bool m_useCameraInReplays;
    //char pad[3]
    float m_shakeSubtleIntensity;
    float m_shakeNormalIntensity;
    float m_shakeStrongIntensity;
    float m_shakeSevereIntensity;
    float m_shakeCineExtremeIntensity;
    float m_shakeCineInsaneIntensity;
    float m_maxShakeIntensity;
    float m_maxShakeRange;
    float m_sellPercentage;
    float m_baseRegenHealthPercentPerSecond;
    uint32_t m_baseRegenDelay;
    int32_t m_hotKeytextColor;
    AsciiString m_specialPowerViewObject;
    std::vector<AsciiString> m_standardPublicBones;
    float m_standardMinefieldDensity;
    float m_standardMinefieldDistance;
    bool m_showMetrics;
    //char pad[3]
    Money m_defaultStartingCash;
    bool m_unkBool22;
    //char pad[3]
    int32_t m_powerBarBase;
    float m_powerBarIntervals;
    int32_t m_powerBarYellowRange;
    float m_gammaValue;
    uint32_t m_unlookPersistDuration;
    bool m_updateTGAtoDDS;
    //char pad[3]
    int32_t m_doubleClickTime;
    //int32_t m_shroudColor;
    //float m_unkFloat6;
    //float m_unkFloat7;
    RGBColor m_shroudColor;
    uint8_t m_clearAlpha;
    uint8_t m_fogAlpha;
    uint8_t m_shroudAlpha;
    //char pad[1]
    int32_t m_networkFPSHistoryLength;
    int32_t m_networkLatencyHistoryLength;
    int32_t m_networkCushionHistoryLength;
    int32_t m_networkRunAheadMetricsTime;
    int32_t m_networkKeepAliveDelay;
    int32_t m_networkRunAheadSlack;
    int32_t m_networkDisconnecTime;
    int32_t m_networkPlayerTimeOut;
    int32_t m_networkDisconnectScreenNotifyTime;
    float m_keyboardCameraRotateSpeed;
    int32_t m_playerStats;
    bool m_unkBool24;
    bool m_unkBool25;
    bool m_unkBool26;
    //char pad[1]
    AsciiString m_userModDirectory;
    AsciiString m_userModFile;
    AsciiString m_userDataDirectory;
    GlobalData *m_next;

private:
    static FieldParse s_fieldParseTable[337];
    static GlobalData *s_theOriginal;
};

inline void GlobalData::Hook_Me()
{
    Hook_Function(Make_Function_Ptr<void, INI*>(0x00418090), Parse_Game_Data_Definitions);
}

#define g_theWriteableGlobalData (Make_Global<GlobalData*>(0x00A2A2A4))
//extern GlobalData *g_theWriteableGlobalData;

#endif
