/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Class for handling various global variables.
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
#include "color.h"
#include "coord.h"
#include "gametype.h"
#include "ini.h"
#include "money.h"
#include "subsysteminterface.h"

class INI;
class WeaponBonusSet;

#define LIGHT_COUNT 3

// These enums need moving when we work out where they should go.
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
DEFINE_ENUMERATION_OPERATORS(TerrainLOD);

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
    virtual void Init() override {}
    virtual void Reset() override;
    virtual void Update() override {}

    bool Set_Time_Of_Day(TimeOfDayType time);

    Utf8String Get_Path_User_Data() const { return m_userDataDirectory; }

    static void Parse_Game_Data_Definition(INI *ini);
    // Looks like members are likely public or there would have been a lot of
    // getters/setters.
    // pad indicates where padding will be added to keep 4 byte alignment
    // useful if we want to cram any extra variables in without breaking ABI
public:
    Utf8String m_mapName;
    Utf8String m_moveHintName;
    bool m_useTrees;
    bool m_useTreeSway;
    bool m_extraAnimationsDisabled;
    bool m_useHeatEffects;
    bool m_useFPSLimit;
    bool m_dumpAssetUsage;
    // char pad[2]
    int32_t m_framesPerSecondLimit;
    int32_t m_chipsetType;
    bool m_windowed;
    // char pad[3]
    int32_t m_xResolution;
    int32_t m_yResolution;
    int32_t m_maxShellScreens;
    bool m_useCloudMap;
    // char pad[3]
    int32_t m_use3WayTerrainBlends; // Should be bool? m_if so, fix when all GlobalData reading code implemented.
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
    bool m_useStaticLODLevels; // Controls if LOD levels are loaded from GameLOD.ini
    // char pad[2]
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
    // char pad[3]
    float m_waterPositionX;
    float m_waterPositionY;
    float m_waterPositionZ;
    float m_waterExtentX;
    float m_waterExtentY;
    int32_t m_waterType;
    bool m_showSoftWaterEdge;
    bool m_useWaveEditor;
    bool m_unsortedShoreLines;
    // char pad[1]
    int m_featherWater;
    Utf8String m_vertexWaterAvailableMaps[4];
    float m_vertexWaterHeightClampLow[4];
    float m_vertexWaterHeightClampHigh[4];
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
    float m_drawSkyBox; // is used as a bool and appears to be defined wrong
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
    bool m_preloadAssets;
    bool m_preloadEverything;
    bool m_logAssets;
    // char pad[1]
    float m_partitionCellSize;
    Coord3D m_ammoPipWorldOffset;
    Coord3D m_containerPipWorldOffset;
    Coord2D m_ammoPipScreenOffset;
    Coord2D m_containerPipScreenOffset;
    float m_ammoPipScaleFactor;
    float m_containerPipScaleFactor;
    uint32_t m_historicDamageLimit;
    int32_t m_maxTerrainTracks;
    int32_t m_maxTankTrackEdges;
    int32_t m_maxTankTrackOpaqueEdges;
    int32_t m_maxTankTrackFadeDelay;
    Utf8String m_levelGainAnimName;
    float m_levelGainAnimTime;
    float m_levelGainAnimZRise;
    Utf8String m_getHealedAnimName;
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
    float m_infantryLight[TIME_OF_DAY_COUNT];
    float m_infantryLightOverride; // Based on ScriptAction::doSetInfantryLightingOverride
    float m_soloHealthBonus[2][DIFFICULTY_COUNT]; // for first array, 0 is human, 1 is AI
    int32_t m_maxTranslucencyObjects;
    int32_t m_maxOccludedBuildings;
    int32_t m_maxOccludedObjects;
    int32_t m_maxOccludedOthers;
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
    bool m_useFX;
    bool m_showClientPhysics;
    bool m_showTerrainNormals;
    // char pad[2]
    int32_t m_frameToJumpTo; // Perhaps not a float, set to frame number in worldbuilder as an int.
    int32_t m_debugAI; // Possibly old BOOL typedef for int?, keep int for ABI compat until sure
    bool m_logSupplyCenterPlacement;
    bool m_debugObstacleAI;
    bool m_showObjectHealth;
    bool m_scriptDebug; // Requires DebugWindow.dll to do anything
    bool m_particleEdit; // Requires ParticleEditor.dll to do anything
    bool m_displayDebug; // not 100% sure and needs confirming
    bool m_winCursors;
    bool m_constantDebugUpdate;
    bool m_showTeamDot;
#ifdef GAME_DEBUG_STRUCTS
    bool m_doStatDump;
    bool m_doStats;
    uint32_t m_statsInterval;
#endif
    bool m_writeBenchMarkFile;
    // char pad[2]
    int32_t m_fixedSeed;
    float m_particleScale;
    Utf8String m_autoFireParticleSmallPrefix;
    Utf8String m_autoFireParticleSmallSystem;
    int32_t m_autoFireParticleSmallMax;
    Utf8String m_autoFireParticleMediumPrefix;
    Utf8String m_autoFireParticleMediumSystem;
    int32_t m_autoFireParticleMediumMax;
    Utf8String m_autoFireParticleLargePrefix;
    Utf8String m_autoFireParticleLargeSystem;
    int32_t m_autoFireParticleLargeMax;
    Utf8String m_autoSmokeParticleSmallPrefix;
    Utf8String m_autoSmokeParticleSmallSystem;
    int32_t m_autoSmokeParticleSmallMax;
    Utf8String m_autoSmokeParticleMediumPrefix;
    Utf8String m_autoSmokeParticleMediumSystem;
    int32_t m_autoSmokeParticleMediumMax;
    Utf8String m_autoSmokeParticleLargePrefix;
    Utf8String m_autoSmokeParticleLargeSystem;
    int32_t m_autoSmokeParticleLargeMax;
    Utf8String m_autoAFlameParticlePrefix;
    Utf8String m_autoAFlameParticleSystem;
    int32_t m_autoAFlameParticleMax;
    int32_t m_netMinPlayers; // not 100% sure, needs confirming
    int32_t m_defaultIP;
    int32_t m_firewallBehaviour;
    bool m_sendDelay;
    // char pad[3]
    int32_t m_firewallPortOverrides;
    int16_t m_firewallPortAllocationDelta;
    // char pad[2]
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
    bool m_buildMapCache;
    // char pad[2]
    Utf8String m_initialFile;
    Utf8String m_pendingFile;
    int32_t m_maxParticleCount;
    int32_t m_maxFieldParticleCount;
    WeaponBonusSet *m_weaponBonusSet;
    float m_normalHealthBonus;
    float m_veteranHealthBonus;
    float m_eliteHealthBonus;
    float m_heroicHealthBonus;
    float m_defaultStructureRubbleHeight;
    Utf8String m_shellMapName;
    bool m_shellMapOn;
    bool m_playIntro;
    bool m_playSizzle;
    bool m_afterIntro;
    bool m_allowSkipMovie;
    bool m_unkBool17;
    // char pad[2]
    float m_keyboardScrollFactor;
    float m_keyboardDefaultScrollFactor;
    float m_musicVolumeFactor; // not 100% sure, needs confirming
    float m_SFXVolumeFactor; // not 100% sure, needs confirming
    float m_voiceVolumeFactor; // not 100% sure, needs confirming
    bool m_sound3DPref; // not 100% sure, needs confirming
    bool m_animateWindows;
    bool m_setMinVertexBufferSize; // not 100% sure, needs confirming
    // char pad[1]
    uint32_t m_iniCRC;
    uint32_t m_gameCRC;
    BodyDamageType m_movementPenaltyDamageState;
    int32_t m_groupSelectMinSelectSize;
    float m_groupSelectVolumeBase;
    float m_groupSelectVolumeIncrement;
    int32_t m_maxUnitSelectSounds;
    float m_selectionFlashSaturationFactor;
    bool m_selectionFlashHouseColor;
    // char pad[3]
    float m_cameraAudibleRadius;
    float m_groupMoveClickToGatherAreaFactor;
    int32_t m_antiAliasBoxValue; // could be float
    bool m_languageFilter;
    bool m_loadScreenDemo;
    bool m_demoToggleRender;
    bool m_saveCameraInReplays;
    bool m_useCameraInReplays;
    // char pad[3]
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
    Utf8String m_specialPowerViewObject;
    std::vector<Utf8String> m_standardPublicBones;
    float m_standardMinefieldDensity;
    float m_standardMinefieldDistance;
    bool m_showMetrics;
    // char pad[3]
    Money m_defaultStartingCash;
    bool m_showFrameRateBar;
    // char pad[3]
    int32_t m_powerBarBase;
    float m_powerBarIntervals;
    int32_t m_powerBarYellowRange;
    float m_gammaValue;
    uint32_t m_unlookPersistDuration;
    bool m_updateTGAtoDDS;
    // char pad[3]
    int32_t m_doubleClickTime;
    RGBColor m_shroudColor;
    uint8_t m_clearAlpha;
    uint8_t m_fogAlpha;
    uint8_t m_shroudAlpha;
    // char pad[1]
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
    bool m_demoToggleSpecialPowerDelays;
#ifdef GAME_DEBUG_STRUCTS
    bool m_TiVOFastMode;
    bool m_wireframe;
    bool m_stateMachineDebug;
    bool m_useCameraConstraints;
    bool m_shroudOn;
    bool m_fogOfWarOn;
    bool m_jabberOn;
    bool m_munkeeOn;
    bool m_selectTheUnselectable;
    bool m_disableCameraFade;
    bool m_disableScriptedInputDisabling;
    bool m_disableMilitaryCaption;
    // pad[3]
    int32_t m_benchmarkTimer; // Type to confirm.
    bool m_checkMemoryLeaks;
    bool m_vTune;
    bool m_cameraDebug;
    bool m_debugVisibility;
    int32_t m_debugVisibilityTileCount; // Type to confirm.
    int32_t m_debugVisibilityTileWidth; // Type to confirm.
    int32_t m_debugVisibilityTileDuration; // Type to confirm.
    bool m_debugMapThreat;
    // pad[3]
    int32_t m_maxDebugThreatMapValue; // Type to confirm.
    int32_t m_debugThreatMapTileDuration; // Type to confirm.
    bool m_debugMapCash;
    // pad[3]
    int32_t m_maxDebugCashValueMapValue; // Type to confirm.
    int32_t m_debugCashValueMapTileDuration; // Type to confirm.
    RGBColor m_debugVisibilityTileTargettableColor;
    RGBColor m_debugVisibilityTileDeshroudColor;
    RGBColor m_debugVisibilityTileGapColor;
    bool m_debugProjectilePath;
    // pad[3]
    int32_t m_debugProjectileTileWidth; // Type to confirm.
    int32_t m_debugProjectileTileDuration; // Type to confirm.
    RGBColor m_debugProjectileTileColor;
    bool m_ignoreAsserts;
    bool m_ignoreStackTrace;
    bool m_showCollisionExtents;
    bool m_showAudioLocations;
    bool m_saveStats;
    bool m_saveAllStats;
    bool m_useLocalMOTD;
    // pad[1]
    Utf8String m_baseStatsDir;
    Utf8String m_localMOTDPath;
    int32_t m_latencyAverage; // Type to confirm.
    int32_t m_latencyAmplitude; // Type to confirm.
    int32_t m_latencyPeriod; // Type to confirm.
    int32_t m_latencyNoise; // Type to confirm.
    int32_t m_packetLoss; // Type to confirm.
    bool m_extraLogging; // Type to confirm.
#endif
    bool m_unkBool25;
    bool m_unkBool26;
    // char pad[1]
    Utf8String m_userModDirectory;
    Utf8String m_userModFile;
    Utf8String m_userDataDirectory;
    GlobalData *m_next;

private:
    static const FieldParse s_fieldParseTable[337];
    static GlobalData *s_theOriginal;
};

#ifdef GAME_DLL
extern GlobalData *&g_theWriteableGlobalData;
#else
extern GlobalData *g_theWriteableGlobalData;
#endif
