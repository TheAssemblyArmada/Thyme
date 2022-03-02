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
#include "globaldata.h"
#include "crc.h"
#include "endiantype.h"
#include "filesystem.h"
#include "gitverinfo.h"
#include "optionpreferences.h"
#include "rtsutils.h"
#include "version.h"
#include "weapon.h"
#include <captainslog.h>

#ifdef PLATFORM_WINDOWS
#include <winuser.h>
#endif

#ifndef GAME_DLL
GlobalData *g_theWriteableGlobalData = nullptr;
#endif

GlobalData *GlobalData::s_theOriginal = nullptr;
const int CRC_BUFFER_SIZE = 0x10000;

// clang-format off
constexpr const char *const s_terrain_lod_names[] = {
    "NONE",
    "MIN",
    "STRETCH_NO_CLOUDS",
    "HALF_CLOUDS",
    "NO_CLOUDS",
    "STRETCH_CLOUDS",
    "NO_WATER",
    "MAX",
    "AUTOMATIC",
    "DISABLE",
    nullptr
};

// List of keys handled in the ini. Class contains some variables that don't appear to be user controlled.

const FieldParse GlobalData::s_fieldParseTable[] = {
    FIELD_PARSE_BOOL("Windowed", GlobalData, m_windowed),
    FIELD_PARSE_INT("XResolution", GlobalData, m_xResolution),
    FIELD_PARSE_INT("YResolution", GlobalData, m_yResolution),
    FIELD_PARSE_ASCIISTRING("MapName", GlobalData, m_mapName),
    FIELD_PARSE_ASCIISTRING("MoveHintName", GlobalData, m_moveHintName),
    FIELD_PARSE_BOOL("UseTrees", GlobalData, m_useTrees),
    FIELD_PARSE_BOOL("UseFPSLimit", GlobalData, m_useFPSLimit),
    FIELD_PARSE_BOOL("DumpAssetUsage", GlobalData, m_dumpAssetUsage),
    FIELD_PARSE_INT("FramesPerSecondLimit", GlobalData, m_framesPerSecondLimit),
    FIELD_PARSE_INT("ChipsetType", GlobalData, m_chipsetType),
    FIELD_PARSE_INT("MaxShellScreens", GlobalData, m_maxShellScreens),
    FIELD_PARSE_BOOL("UseCloudMap", GlobalData, m_useCloudMap),
    FIELD_PARSE_BOOL("UseLightMap", GlobalData, m_useLightMap),
    FIELD_PARSE_BOOL("BilinearTerrainTex", GlobalData, m_bilinearTerrainTexture),
    FIELD_PARSE_BOOL("TrilinearTerrainTex", GlobalData, m_trilinearTerrainTexture),
    FIELD_PARSE_BOOL("MultiPassTerrain", GlobalData, m_multiPassTerrain),
    FIELD_PARSE_BOOL("AdjustCliffTextures", GlobalData, m_adjustCliffTextures),
    FIELD_PARSE_INT("Use3WayTerrainBlends", GlobalData, m_use3WayTerrainBlends),
    FIELD_PARSE_BOOL("StretchTerrain", GlobalData, m_stretchTerrain),
    FIELD_PARSE_BOOL("UseHalfHeightMap", GlobalData, m_useHalfHeightMap),
    FIELD_PARSE_BOOL("DrawEntireTerrain", GlobalData, m_drawEntireTerrain),
    FIELD_PARSE_INDEX_LIST("TerrainLOD", s_terrain_lod_names, GlobalData, m_terrainLOD),
    FIELD_PARSE_INT("TerrainLODTargetTimeMS", GlobalData, m_terrainLODTargetTimeMS),
    FIELD_PARSE_BOOL("RightMouseAlwaysScrolls", GlobalData, m_rightMouseAlwaysScrolls),
    FIELD_PARSE_BOOL("UseWaterPlane", GlobalData, m_useWaterPlane),
    FIELD_PARSE_BOOL("UseCloudPlane", GlobalData, m_useCloudPlane),
    FIELD_PARSE_REAL("DownwindAngle", GlobalData, m_downWindAngle),
    FIELD_PARSE_BOOL("UseShadowVolumes", GlobalData, m_shadowVolumes),
    FIELD_PARSE_BOOL("UseShadowDecals", GlobalData, m_shadowDecals),
    FIELD_PARSE_INT("TextureReductionFactor", GlobalData, m_textureReductionFactor),
    FIELD_PARSE_BOOL("UseBehindBuildingMarker", GlobalData, m_useBehindBuildingMarker),
    FIELD_PARSE_REAL("WaterPositionX", GlobalData, m_waterPositionX),
    FIELD_PARSE_REAL("WaterPositionY", GlobalData, m_waterPositionY),
    FIELD_PARSE_REAL("WaterPositionZ", GlobalData, m_waterPositionZ),
    FIELD_PARSE_REAL("WaterExtentX", GlobalData, m_waterExtentX),
    FIELD_PARSE_REAL("WaterExtentY", GlobalData, m_waterExtentY),
    FIELD_PARSE_INT("WaterType", GlobalData, m_waterType),
    FIELD_PARSE_INT("FeatherWater", GlobalData, m_featherWater),
    FIELD_PARSE_BOOL("ShowSoftWaterEdge", GlobalData, m_showSoftWaterEdge),
    FIELD_PARSE_ASCIISTRING("VertexWaterAvailableMaps1", GlobalData, m_vertexWaterAvailableMaps[0]),
    FIELD_PARSE_REAL("VertexWaterHeightClampLow1", GlobalData, m_vertexWaterHeightClampLow[0]),
    FIELD_PARSE_REAL("VertexWaterHeightClampHi1", GlobalData, m_vertexWaterHeightClampHigh[0]),
    FIELD_PARSE_ANGLE_REAL("VertexWaterAngle1", GlobalData, m_vertexWaterAngle[0]),
    FIELD_PARSE_REAL("VertexWaterXPosition1", GlobalData, m_vertexWaterXPos[0]),
    FIELD_PARSE_REAL("VertexWaterYPosition1", GlobalData, m_vertexWaterYPos[0]),
    FIELD_PARSE_REAL("VertexWaterZPosition1", GlobalData, m_vertexWaterZPos[0]),
    FIELD_PARSE_INT("VertexWaterXGridCells1", GlobalData, m_vertexWaterXGridCells[0]),
    FIELD_PARSE_INT("VertexWaterYGridCells1", GlobalData, m_vertexWaterYGridCells[0]),
    FIELD_PARSE_REAL("VertexWaterGridSize1", GlobalData, m_vertexWaterGridSize[0]),
    FIELD_PARSE_REAL("VertexWaterAttenuationA1", GlobalData, m_vertexWaterAttenuationA[0]),
    FIELD_PARSE_REAL("VertexWaterAttenuationB1", GlobalData, m_vertexWaterAttenuationB[0]),
    FIELD_PARSE_REAL("VertexWaterAttenuationC1", GlobalData, m_vertexWaterAttenuationC[0]),
    FIELD_PARSE_REAL("VertexWaterAttenuationRange1", GlobalData, m_vertexWaterAttenuationRange[0]),
    FIELD_PARSE_ASCIISTRING("VertexWaterAvailableMaps2", GlobalData, m_vertexWaterAvailableMaps[1]),
    FIELD_PARSE_REAL("VertexWaterHeightClampLow2", GlobalData, m_vertexWaterHeightClampLow[1]),
    FIELD_PARSE_REAL("VertexWaterHeightClampHi2", GlobalData, m_vertexWaterHeightClampHigh[1]),
    FIELD_PARSE_ANGLE_REAL("VertexWaterAngle2", GlobalData, m_vertexWaterAngle[1]),
    FIELD_PARSE_REAL("VertexWaterXPosition2", GlobalData, m_vertexWaterXPos[1]),
    FIELD_PARSE_REAL("VertexWaterYPosition2", GlobalData, m_vertexWaterYPos[1]),
    FIELD_PARSE_REAL("VertexWaterZPosition2", GlobalData, m_vertexWaterZPos[1]),
    FIELD_PARSE_INT("VertexWaterXGridCells2", GlobalData, m_vertexWaterXGridCells[1]),
    FIELD_PARSE_INT("VertexWaterYGridCells2", GlobalData, m_vertexWaterYGridCells[1]),
    FIELD_PARSE_REAL("VertexWaterGridSize2", GlobalData, m_vertexWaterGridSize[1]),
    FIELD_PARSE_REAL("VertexWaterAttenuationA2", GlobalData, m_vertexWaterAttenuationA[1]),
    FIELD_PARSE_REAL("VertexWaterAttenuationB2", GlobalData, m_vertexWaterAttenuationB[1]),
    FIELD_PARSE_REAL("VertexWaterAttenuationC2", GlobalData, m_vertexWaterAttenuationC[1]),
    FIELD_PARSE_REAL("VertexWaterAttenuationRange2", GlobalData, m_vertexWaterAttenuationRange[1]),
    FIELD_PARSE_ASCIISTRING("VertexWaterAvailableMaps3", GlobalData, m_vertexWaterAvailableMaps[2]),
    FIELD_PARSE_REAL("VertexWaterHeightClampLow3", GlobalData, m_vertexWaterHeightClampLow[2]),
    FIELD_PARSE_REAL("VertexWaterHeightClampHi3", GlobalData, m_vertexWaterHeightClampHigh[2]),
    FIELD_PARSE_ANGLE_REAL("VertexWaterAngle3", GlobalData, m_vertexWaterAngle[2]),
    FIELD_PARSE_REAL("VertexWaterXPosition3", GlobalData, m_vertexWaterXPos[2]),
    FIELD_PARSE_REAL("VertexWaterYPosition3", GlobalData, m_vertexWaterYPos[2]),
    FIELD_PARSE_REAL("VertexWaterZPosition3", GlobalData, m_vertexWaterZPos[2]),
    FIELD_PARSE_INT("VertexWaterXGridCells3", GlobalData, m_vertexWaterXGridCells[2]),
    FIELD_PARSE_INT("VertexWaterYGridCells3", GlobalData, m_vertexWaterYGridCells[2]),
    FIELD_PARSE_REAL("VertexWaterGridSize3", GlobalData, m_vertexWaterGridSize[2]),
    FIELD_PARSE_REAL("VertexWaterAttenuationA3", GlobalData, m_vertexWaterAttenuationA[2]),
    FIELD_PARSE_REAL("VertexWaterAttenuationB3", GlobalData, m_vertexWaterAttenuationB[2]),
    FIELD_PARSE_REAL("VertexWaterAttenuationC3", GlobalData, m_vertexWaterAttenuationC[2]),
    FIELD_PARSE_REAL("VertexWaterAttenuationRange3", GlobalData, m_vertexWaterAttenuationRange[2]),
    FIELD_PARSE_ASCIISTRING("VertexWaterAvailableMaps4", GlobalData, m_vertexWaterAvailableMaps[3]),
    FIELD_PARSE_REAL("VertexWaterHeightClampLow4", GlobalData, m_vertexWaterHeightClampLow[3]),
    FIELD_PARSE_REAL("VertexWaterHeightClampHi4", GlobalData, m_vertexWaterHeightClampHigh[3]),
    FIELD_PARSE_ANGLE_REAL("VertexWaterAngle4", GlobalData, m_vertexWaterAngle[3]),
    FIELD_PARSE_REAL("VertexWaterXPosition4", GlobalData, m_vertexWaterXPos[3]),
    FIELD_PARSE_REAL("VertexWaterYPosition4", GlobalData, m_vertexWaterYPos[3]),
    FIELD_PARSE_REAL("VertexWaterZPosition4", GlobalData, m_vertexWaterZPos[3]),
    FIELD_PARSE_INT("VertexWaterXGridCells4", GlobalData, m_vertexWaterXGridCells[3]),
    FIELD_PARSE_INT("VertexWaterYGridCells4", GlobalData, m_vertexWaterYGridCells[3]),
    FIELD_PARSE_REAL("VertexWaterGridSize4", GlobalData, m_vertexWaterGridSize[3]),
    FIELD_PARSE_REAL("VertexWaterAttenuationA4", GlobalData, m_vertexWaterAttenuationA[3]),
    FIELD_PARSE_REAL("VertexWaterAttenuationB4", GlobalData, m_vertexWaterAttenuationB[3]),
    FIELD_PARSE_REAL("VertexWaterAttenuationC4", GlobalData, m_vertexWaterAttenuationC[3]),
    FIELD_PARSE_REAL("VertexWaterAttenuationRange4", GlobalData, m_vertexWaterAttenuationRange[3]),
    FIELD_PARSE_REAL("SkyBoxPositionZ", GlobalData, m_skyBoxPositionZ),
    FIELD_PARSE_REAL("SkyBoxScale", GlobalData, m_skyBoxScale),
    { "DrawSkyBox", &INI::Parse_Bool, nullptr, offsetof(GlobalData, m_drawSkyBox) },
    FIELD_PARSE_REAL("CameraPitch", GlobalData, m_cameraPitch),
    FIELD_PARSE_REAL("CameraYaw", GlobalData, m_cameraYaw),
    FIELD_PARSE_REAL("CameraHeight", GlobalData, m_cameraHeight),
    FIELD_PARSE_REAL("MaxCameraHeight", GlobalData, m_maxCameraHeight),
    FIELD_PARSE_REAL("MinCameraHeight", GlobalData, m_minCameraHeight),
    FIELD_PARSE_REAL("TerrainHeightAtEdgeOfMap", GlobalData, m_terrainHeightAtMapEdge),
    FIELD_PARSE_REAL("UnitDamagedThreshold", GlobalData, m_unitDamagedThreshold),
    FIELD_PARSE_REAL("UnitReallyDamagedThreshold", GlobalData, m_unitReallyDamagedThreshold),
    FIELD_PARSE_REAL("GroundStiffness", GlobalData, m_groundStiffness),
    FIELD_PARSE_REAL("StructureStiffness", GlobalData, m_structureStiffness),
    { "Gravity", &INI::Parse_Acceleration_Real, nullptr, offsetof(GlobalData, m_gravity) },
    FIELD_PARSE_PERCENT_TO_REAL("StealthFriendlyOpacity", GlobalData, m_stealthFriendlyOpacity),
    { "DefaultOcclusionDelay", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(GlobalData, m_defaultOcclusionDelay) },
    FIELD_PARSE_REAL("PartitionCellSize", GlobalData, m_partitionCellSize),
    FIELD_PARSE_REAL("AmmoPipScaleFactor", GlobalData, m_ammoPipScaleFactor),
    FIELD_PARSE_REAL("ContainerPipScaleFactor", GlobalData, m_containerPipScaleFactor),
    FIELD_PARSE_COORD3D("AmmoPipWorldOffset", GlobalData, m_ammoPipWorldOffset),
    FIELD_PARSE_COORD3D("ContainerPipWorldOffset", GlobalData, m_containerPipWorldOffset),
    FIELD_PARSE_COORD2D("AmmoPipScreenOffset", GlobalData, m_ammoPipScreenOffset),
    FIELD_PARSE_COORD2D("ContainerPipScreenOffset", GlobalData, m_containerPipScreenOffset),
    { "HistoricDamageLimit", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(GlobalData, m_historicDamageLimit) },
    FIELD_PARSE_INT("MaxTerrainTracks", GlobalData, m_maxTerrainTracks),
    FIELD_PARSE_INDEX_LIST("TimeOfDay", g_timeOfDayNames, GlobalData, m_timeOfDay),
    FIELD_PARSE_INDEX_LIST("Weather", g_weatherNames, GlobalData, m_weather),
    FIELD_PARSE_BOOL("MakeTrackMarks", GlobalData, m_makeTrackMarks),
    FIELD_PARSE_BOOL("HideGarrisonFlags", GlobalData, m_hideGarrisonFlags),
    FIELD_PARSE_BOOL("ForceModelsToFollowTimeOfDay", GlobalData, m_forceModelsFollowTimeOfDay),
    FIELD_PARSE_BOOL("ForceModelsToFollowWeather", GlobalData, m_forceModelsFollowWeather),
    FIELD_PARSE_ASCIISTRING("LevelGainAnimationName", GlobalData, m_levelGainAnimName),
    FIELD_PARSE_REAL("LevelGainAnimationTime", GlobalData, m_levelGainAnimTime),
    FIELD_PARSE_REAL("LevelGainAnimationZRise", GlobalData, m_levelGainAnimZRise),
    FIELD_PARSE_ASCIISTRING("GetHealedAnimationName", GlobalData, m_getHealedAnimName),
    FIELD_PARSE_REAL("GetHealedAnimationTime", GlobalData, m_getHealedAnimTime),
    FIELD_PARSE_REAL("GetHealedAnimationZRise", GlobalData, m_getHealedAnimZRise),
    // Light 1
    FIELD_PARSE_RGB_COLOR("TerrainLightingMorningAmbient", GlobalData, m_terrainPlaneLighting[TIME_OF_DAY_MORNING][0].ambient),
    FIELD_PARSE_RGB_COLOR("TerrainLightingMorningDiffuse", GlobalData, m_terrainPlaneLighting[TIME_OF_DAY_MORNING][0].diffuse),
    FIELD_PARSE_COORD3D("TerrainLightingMorningLightPos", GlobalData, m_terrainPlaneLighting[TIME_OF_DAY_MORNING][0].lightPos),
    FIELD_PARSE_RGB_COLOR("TerrainLightingAfternoonAmbient", GlobalData, m_terrainPlaneLighting[TIME_OF_DAY_AFTERNOON][0].ambient),
    FIELD_PARSE_RGB_COLOR("TerrainLightingAfternoonDiffuse", GlobalData, m_terrainPlaneLighting[TIME_OF_DAY_AFTERNOON][0].diffuse),
    FIELD_PARSE_COORD3D("TerrainLightingAfternoonLightPos", GlobalData, m_terrainPlaneLighting[TIME_OF_DAY_AFTERNOON][0].lightPos),
    FIELD_PARSE_RGB_COLOR("TerrainLightingEveningAmbient", GlobalData, m_terrainPlaneLighting[TIME_OF_DAY_EVENING][0].ambient),
    FIELD_PARSE_RGB_COLOR("TerrainLightingEveningDiffuse", GlobalData, m_terrainPlaneLighting[TIME_OF_DAY_EVENING][0].diffuse),
    FIELD_PARSE_COORD3D("TerrainLightingEveningLightPos", GlobalData, m_terrainPlaneLighting[TIME_OF_DAY_EVENING][0].lightPos),
    FIELD_PARSE_RGB_COLOR("TerrainLightingNightAmbient", GlobalData, m_terrainPlaneLighting[TIME_OF_DAY_NIGHT][0].ambient),
    FIELD_PARSE_RGB_COLOR("TerrainLightingNightDiffuse", GlobalData, m_terrainPlaneLighting[TIME_OF_DAY_NIGHT][0].diffuse),
    FIELD_PARSE_COORD3D("TerrainLightingNightLightPos", GlobalData, m_terrainPlaneLighting[TIME_OF_DAY_NIGHT][0].lightPos),
    FIELD_PARSE_RGB_COLOR("TerrainObjectsLightingMorningAmbient", GlobalData, m_terrainObjectLighting[TIME_OF_DAY_MORNING][0].ambient),
    FIELD_PARSE_RGB_COLOR("TerrainObjectsLightingMorningDiffuse", GlobalData, m_terrainObjectLighting[TIME_OF_DAY_MORNING][0].diffuse),
    FIELD_PARSE_COORD3D("TerrainObjectsLightingMorningLightPos", GlobalData, m_terrainObjectLighting[TIME_OF_DAY_MORNING][0].lightPos),
    FIELD_PARSE_RGB_COLOR("TerrainObjectsLightingAfternoonAmbient", GlobalData, m_terrainObjectLighting[TIME_OF_DAY_AFTERNOON][0].ambient),
    FIELD_PARSE_RGB_COLOR("TerrainObjectsLightingAfternoonDiffuse", GlobalData, m_terrainObjectLighting[TIME_OF_DAY_AFTERNOON][0].diffuse),
    FIELD_PARSE_COORD3D("TerrainObjectsLightingAfternoonLightPos", GlobalData, m_terrainObjectLighting[TIME_OF_DAY_AFTERNOON][0].lightPos),
    FIELD_PARSE_RGB_COLOR("TerrainObjectsLightingEveningAmbient", GlobalData, m_terrainObjectLighting[TIME_OF_DAY_EVENING][0].ambient),
    FIELD_PARSE_RGB_COLOR("TerrainObjectsLightingEveningDiffuse", GlobalData, m_terrainObjectLighting[TIME_OF_DAY_EVENING][0].diffuse),
    FIELD_PARSE_COORD3D("TerrainObjectsLightingEveningLightPos", GlobalData, m_terrainObjectLighting[TIME_OF_DAY_EVENING][0].lightPos),
    FIELD_PARSE_RGB_COLOR("TerrainObjectsLightingNightAmbient", GlobalData, m_terrainObjectLighting[TIME_OF_DAY_NIGHT][0].ambient),
    FIELD_PARSE_RGB_COLOR("TerrainObjectsLightingNightDiffuse", GlobalData, m_terrainObjectLighting[TIME_OF_DAY_NIGHT][0].diffuse),
    FIELD_PARSE_COORD3D("TerrainObjectsLightingNightLightPos", GlobalData, m_terrainObjectLighting[TIME_OF_DAY_NIGHT][0].lightPos),
    // Light 2
    FIELD_PARSE_RGB_COLOR("TerrainLightingMorningAmbient2", GlobalData, m_terrainPlaneLighting[TIME_OF_DAY_MORNING][1].ambient),
    FIELD_PARSE_RGB_COLOR("TerrainLightingMorningDiffuse2", GlobalData, m_terrainPlaneLighting[TIME_OF_DAY_MORNING][1].diffuse),
    FIELD_PARSE_COORD3D("TerrainLightingMorningLightPos2", GlobalData, m_terrainPlaneLighting[TIME_OF_DAY_MORNING][1].lightPos),
    FIELD_PARSE_RGB_COLOR("TerrainLightingAfternoonAmbient2", GlobalData, m_terrainPlaneLighting[TIME_OF_DAY_AFTERNOON][1].ambient),
    FIELD_PARSE_RGB_COLOR("TerrainLightingAfternoonDiffuse2", GlobalData, m_terrainPlaneLighting[TIME_OF_DAY_AFTERNOON][1].diffuse),
    FIELD_PARSE_COORD3D("TerrainLightingAfternoonLightPos2", GlobalData, m_terrainPlaneLighting[TIME_OF_DAY_AFTERNOON][1].lightPos),
    FIELD_PARSE_RGB_COLOR("TerrainLightingEveningAmbient2", GlobalData, m_terrainPlaneLighting[TIME_OF_DAY_EVENING][1].ambient),
    FIELD_PARSE_RGB_COLOR("TerrainLightingEveningDiffuse2", GlobalData, m_terrainPlaneLighting[TIME_OF_DAY_EVENING][1].diffuse),
    FIELD_PARSE_COORD3D("TerrainLightingEveningLightPos2", GlobalData, m_terrainPlaneLighting[TIME_OF_DAY_EVENING][1].lightPos),
    FIELD_PARSE_RGB_COLOR("TerrainLightingNightAmbient2", GlobalData, m_terrainPlaneLighting[TIME_OF_DAY_NIGHT][1].ambient),
    FIELD_PARSE_RGB_COLOR("TerrainLightingNightDiffuse2", GlobalData, m_terrainPlaneLighting[TIME_OF_DAY_NIGHT][1].diffuse),
    FIELD_PARSE_COORD3D("TerrainLightingNightLightPos2", GlobalData, m_terrainPlaneLighting[TIME_OF_DAY_NIGHT][1].lightPos),
    FIELD_PARSE_RGB_COLOR("TerrainObjectsLightingMorningAmbient2", GlobalData, m_terrainObjectLighting[TIME_OF_DAY_MORNING][1].ambient),
    FIELD_PARSE_RGB_COLOR("TerrainObjectsLightingMorningDiffuse2", GlobalData, m_terrainObjectLighting[TIME_OF_DAY_MORNING][1].diffuse),
    FIELD_PARSE_COORD3D("TerrainObjectsLightingMorningLightPos2", GlobalData, m_terrainObjectLighting[TIME_OF_DAY_MORNING][1].lightPos),
    FIELD_PARSE_RGB_COLOR("TerrainObjectsLightingAfternoonAmbient2", GlobalData, m_terrainObjectLighting[TIME_OF_DAY_AFTERNOON][1].ambient),
    FIELD_PARSE_RGB_COLOR("TerrainObjectsLightingAfternoonDiffuse2", GlobalData, m_terrainObjectLighting[TIME_OF_DAY_AFTERNOON][1].diffuse),
    FIELD_PARSE_COORD3D("TerrainObjectsLightingAfternoonLightPos2", GlobalData, m_terrainObjectLighting[TIME_OF_DAY_AFTERNOON][1].lightPos),
    FIELD_PARSE_RGB_COLOR("TerrainObjectsLightingEveningAmbient2", GlobalData, m_terrainObjectLighting[TIME_OF_DAY_EVENING][1].ambient),
    FIELD_PARSE_RGB_COLOR("TerrainObjectsLightingEveningDiffuse2", GlobalData, m_terrainObjectLighting[TIME_OF_DAY_EVENING][1].diffuse),
    FIELD_PARSE_COORD3D("TerrainObjectsLightingEveningLightPos2", GlobalData, m_terrainObjectLighting[TIME_OF_DAY_EVENING][1].lightPos),
    FIELD_PARSE_RGB_COLOR("TerrainObjectsLightingNightAmbient2", GlobalData, m_terrainObjectLighting[TIME_OF_DAY_NIGHT][1].ambient),
    FIELD_PARSE_RGB_COLOR("TerrainObjectsLightingNightDiffuse2", GlobalData, m_terrainObjectLighting[TIME_OF_DAY_NIGHT][1].diffuse),
    FIELD_PARSE_COORD3D("TerrainObjectsLightingNightLightPos2", GlobalData, m_terrainObjectLighting[TIME_OF_DAY_NIGHT][1].lightPos),
    // Light 3
    FIELD_PARSE_RGB_COLOR("TerrainLightingMorningAmbient3", GlobalData, m_terrainPlaneLighting[TIME_OF_DAY_MORNING][2].ambient),
    FIELD_PARSE_RGB_COLOR("TerrainLightingMorningDiffuse3", GlobalData, m_terrainPlaneLighting[TIME_OF_DAY_MORNING][2].diffuse),
    FIELD_PARSE_COORD3D("TerrainLightingMorningLightPos3", GlobalData, m_terrainPlaneLighting[TIME_OF_DAY_MORNING][2].lightPos),
    FIELD_PARSE_RGB_COLOR("TerrainLightingAfternoonAmbient3", GlobalData, m_terrainPlaneLighting[TIME_OF_DAY_AFTERNOON][2].ambient),
    FIELD_PARSE_RGB_COLOR("TerrainLightingAfternoonDiffuse3", GlobalData, m_terrainPlaneLighting[TIME_OF_DAY_AFTERNOON][2].diffuse),
    FIELD_PARSE_COORD3D("TerrainLightingAfternoonLightPos3", GlobalData, m_terrainPlaneLighting[TIME_OF_DAY_AFTERNOON][2].lightPos),
    FIELD_PARSE_RGB_COLOR("TerrainLightingEveningAmbient3", GlobalData, m_terrainPlaneLighting[TIME_OF_DAY_EVENING][2].ambient),
    FIELD_PARSE_RGB_COLOR("TerrainLightingEveningDiffuse3", GlobalData, m_terrainPlaneLighting[TIME_OF_DAY_EVENING][2].diffuse),
    FIELD_PARSE_COORD3D("TerrainLightingEveningLightPos3", GlobalData, m_terrainPlaneLighting[TIME_OF_DAY_EVENING][2].lightPos),
    FIELD_PARSE_RGB_COLOR("TerrainLightingNightAmbient3", GlobalData, m_terrainPlaneLighting[TIME_OF_DAY_NIGHT][2].ambient),
    FIELD_PARSE_RGB_COLOR("TerrainLightingNightDiffuse3", GlobalData, m_terrainPlaneLighting[TIME_OF_DAY_NIGHT][2].diffuse),
    FIELD_PARSE_COORD3D("TerrainLightingNightLightPos3", GlobalData, m_terrainPlaneLighting[TIME_OF_DAY_NIGHT][2].lightPos),
    FIELD_PARSE_RGB_COLOR("TerrainObjectsLightingMorningAmbient3", GlobalData, m_terrainObjectLighting[TIME_OF_DAY_MORNING][2].ambient),
    FIELD_PARSE_RGB_COLOR("TerrainObjectsLightingMorningDiffuse3", GlobalData, m_terrainObjectLighting[TIME_OF_DAY_MORNING][2].diffuse),
    FIELD_PARSE_COORD3D("TerrainObjectsLightingMorningLightPos3", GlobalData, m_terrainObjectLighting[TIME_OF_DAY_MORNING][2].lightPos),
    FIELD_PARSE_RGB_COLOR("TerrainObjectsLightingAfternoonAmbient3", GlobalData, m_terrainObjectLighting[TIME_OF_DAY_AFTERNOON][2].ambient),
    FIELD_PARSE_RGB_COLOR("TerrainObjectsLightingAfternoonDiffuse3", GlobalData, m_terrainObjectLighting[TIME_OF_DAY_AFTERNOON][2].diffuse),
    FIELD_PARSE_COORD3D("TerrainObjectsLightingAfternoonLightPos3", GlobalData, m_terrainObjectLighting[TIME_OF_DAY_AFTERNOON][2].lightPos),
    FIELD_PARSE_RGB_COLOR("TerrainObjectsLightingEveningAmbient3", GlobalData, m_terrainObjectLighting[TIME_OF_DAY_EVENING][2].ambient),
    FIELD_PARSE_RGB_COLOR("TerrainObjectsLightingEveningDiffuse3", GlobalData, m_terrainObjectLighting[TIME_OF_DAY_EVENING][2].diffuse),
    FIELD_PARSE_COORD3D("TerrainObjectsLightingEveningLightPos3", GlobalData, m_terrainObjectLighting[TIME_OF_DAY_EVENING][2].lightPos),
    FIELD_PARSE_RGB_COLOR("TerrainObjectsLightingNightAmbient3", GlobalData, m_terrainObjectLighting[TIME_OF_DAY_NIGHT][2].ambient),
    FIELD_PARSE_RGB_COLOR("TerrainObjectsLightingNightDiffuse3", GlobalData, m_terrainObjectLighting[TIME_OF_DAY_NIGHT][2].diffuse),
    FIELD_PARSE_COORD3D("TerrainObjectsLightingNightLightPos3", GlobalData, m_terrainObjectLighting[TIME_OF_DAY_NIGHT][2].lightPos),
    // End of terrain lights
    FIELD_PARSE_INT("NumberGlobalLights", GlobalData, m_numberGlobalLights),
    FIELD_PARSE_REAL("InfantryLightMorningScale", GlobalData, m_infantryLight[TIME_OF_DAY_MORNING]),
    FIELD_PARSE_REAL("InfantryLightAfternoonScale", GlobalData, m_infantryLight[TIME_OF_DAY_AFTERNOON]),
    FIELD_PARSE_REAL("InfantryLightEveningScale", GlobalData, m_infantryLight[TIME_OF_DAY_EVENING]),
    FIELD_PARSE_REAL("InfantryLightNightScale", GlobalData, m_infantryLight[TIME_OF_DAY_NIGHT]),
    FIELD_PARSE_INT("MaxTranslucentObjects", GlobalData, m_maxTranslucencyObjects),
    FIELD_PARSE_REAL("OccludedColorLuminanceScale", GlobalData, m_occludedColorLuminanceScale),
    FIELD_PARSE_INT("MaxRoadSegments", GlobalData, m_maxRoadSegments),
    FIELD_PARSE_INT("MaxRoadVertex", GlobalData, m_maxRoadVertex),
    FIELD_PARSE_INT("MaxRoadIndex", GlobalData, m_maxRoadIndex),
    FIELD_PARSE_INT("MaxRoadTypes", GlobalData, m_maxRoadTypes),
    FIELD_PARSE_INT("ValuePerSupplyBox", GlobalData, m_valuesPerSupplyBox),
    FIELD_PARSE_BOOL("AudioOn", GlobalData, m_audioOn),
    FIELD_PARSE_BOOL("MusicOn", GlobalData, m_musicOn),
    FIELD_PARSE_BOOL("SoundsOn", GlobalData, m_soundsOn),
    FIELD_PARSE_BOOL("Sounds3DOn", GlobalData, m_sounds3DOn),
    FIELD_PARSE_BOOL("SpeechOn", GlobalData, m_speechOn),
    FIELD_PARSE_BOOL("VideoOn", GlobalData, m_videoOn),
    FIELD_PARSE_BOOL("DisableCameraMovements", GlobalData, m_disableCameraMovements),
    { "DebugAI", &INI::Parse_Bool, nullptr, offsetof(GlobalData, m_debugAI) },
    FIELD_PARSE_BOOL("DebugAIObstacles", GlobalData, m_debugObstacleAI),
    FIELD_PARSE_BOOL("ShowClientPhysics", GlobalData, m_showClientPhysics),
    FIELD_PARSE_BOOL("ShowTerrainNormals", GlobalData, m_showTerrainNormals),
    FIELD_PARSE_BOOL("ShowObjectHealth", GlobalData, m_showObjectHealth),
    FIELD_PARSE_REAL("ParticleScale", GlobalData, m_particleScale),
    FIELD_PARSE_ASCIISTRING("AutoFireParticleSmallPrefix", GlobalData, m_autoFireParticleSmallPrefix),
    FIELD_PARSE_ASCIISTRING("AutoFireParticleSmallSystem", GlobalData, m_autoFireParticleSmallSystem),
    FIELD_PARSE_INT("AutoFireParticleSmallMax", GlobalData, m_autoFireParticleSmallMax),
    FIELD_PARSE_ASCIISTRING("AutoFireParticleMediumPrefix", GlobalData, m_autoFireParticleMediumPrefix),
    FIELD_PARSE_ASCIISTRING("AutoFireParticleMediumSystem", GlobalData, m_autoFireParticleMediumSystem),
    FIELD_PARSE_INT("AutoFireParticleMediumMax", GlobalData, m_autoFireParticleMediumMax),
    FIELD_PARSE_ASCIISTRING("AutoFireParticleLargePrefix", GlobalData, m_autoFireParticleLargePrefix),
    FIELD_PARSE_ASCIISTRING("AutoFireParticleLargeSystem", GlobalData, m_autoFireParticleLargeSystem),
    FIELD_PARSE_INT("AutoFireParticleLargeMax", GlobalData, m_autoFireParticleLargeMax),
    FIELD_PARSE_ASCIISTRING("AutoSmokeParticleSmallPrefix", GlobalData, m_autoSmokeParticleSmallPrefix),
    FIELD_PARSE_ASCIISTRING("AutoSmokeParticleSmallSystem", GlobalData, m_autoSmokeParticleSmallSystem),
    FIELD_PARSE_INT("AutoSmokeParticleSmallMax", GlobalData, m_autoSmokeParticleSmallMax),
    FIELD_PARSE_ASCIISTRING("AutoSmokeParticleMediumPrefix", GlobalData, m_autoSmokeParticleMediumPrefix),
    FIELD_PARSE_ASCIISTRING("AutoSmokeParticleMediumSystem", GlobalData, m_autoSmokeParticleMediumSystem),
    FIELD_PARSE_INT("AutoSmokeParticleMediumMax", GlobalData, m_autoSmokeParticleMediumMax),
    FIELD_PARSE_ASCIISTRING("AutoSmokeParticleLargePrefix", GlobalData, m_autoSmokeParticleLargePrefix),
    FIELD_PARSE_ASCIISTRING("AutoSmokeParticleLargeSystem", GlobalData, m_autoSmokeParticleLargeSystem),
    FIELD_PARSE_INT("AutoSmokeParticleLargeMax", GlobalData, m_autoSmokeParticleLargeMax),
    FIELD_PARSE_ASCIISTRING("AutoAflameParticlePrefix", GlobalData, m_autoAFlameParticlePrefix),
    FIELD_PARSE_ASCIISTRING("AutoAflameParticleSystem", GlobalData, m_autoAFlameParticleSystem),
    FIELD_PARSE_INT("AutoAflameParticleMax", GlobalData, m_autoAFlameParticleMax),
    FIELD_PARSE_REAL("BuildSpeed", GlobalData, m_buildSpeed),
    FIELD_PARSE_REAL("MinDistFromEdgeOfMapForBuild", GlobalData, m_minDistanceFromMapEdgeForBuild),
    { "SupplyBuildBorder", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_supplyBuildOrder) },
    FIELD_PARSE_REAL("AllowedHeightVariationForBuilding", GlobalData, m_allowedHeightVariationForBuildings),
    FIELD_PARSE_REAL("MinLowEnergyProductionSpeed", GlobalData, m_minLowEnergyProductionSpeed),
    FIELD_PARSE_REAL("MaxLowEnergyProductionSpeed", GlobalData, m_maxLowEnergyProductionSpeed),
    FIELD_PARSE_REAL("LowEnergyPenaltyModifier", GlobalData, m_lowEnergyPenaltyModifier),
    FIELD_PARSE_REAL("MultipleFactory", GlobalData, m_multipleFactory),
    FIELD_PARSE_PERCENT_TO_REAL("RefundPercent", GlobalData, m_refundPercent),
    FIELD_PARSE_REAL("CommandCenterHealRange", GlobalData, m_commandCenterHealRange),
    FIELD_PARSE_REAL("CommandCenterHealAmount", GlobalData, m_commandCenterHealAmmount),
    FIELD_PARSE_REAL("StandardMinefieldDensity", GlobalData, m_standardMinefieldDensity),
    FIELD_PARSE_REAL("StandardMinefieldDistance", GlobalData, m_standardMinefieldDistance),
    FIELD_PARSE_INT("MaxLineBuildObjects", GlobalData, m_maxLineBuildObjects),
    FIELD_PARSE_INT("MaxTunnelCapacity", GlobalData, m_maxTunnelCapacity),
    FIELD_PARSE_INT("MaxParticleCount", GlobalData, m_maxParticleCount),
    FIELD_PARSE_INT("MaxFieldParticleCount", GlobalData, m_maxFieldParticleCount),
    FIELD_PARSE_REAL("HorizontalScrollSpeedFactor", GlobalData, m_horizontalScrollSpeedFactor),
    FIELD_PARSE_REAL("VerticalScrollSpeedFactor", GlobalData, m_verticalScrollSpeedFactor),
    FIELD_PARSE_REAL("ScrollAmountCutoff", GlobalData, m_scrollAmountCutoff),
    FIELD_PARSE_REAL("CameraAdjustSpeed", GlobalData, m_cameraAdjustSpeed),
    FIELD_PARSE_BOOL("EnforceMaxCameraHeight", GlobalData, m_enforceMaxCameraHeight),
    FIELD_PARSE_REAL("KeyboardScrollSpeedFactor", GlobalData, m_keyboardScrollFactor),
    FIELD_PARSE_REAL("KeyboardDefaultScrollSpeedFactor", GlobalData, m_keyboardDefaultScrollFactor),
    FIELD_PARSE_INDEX_LIST("MovementPenaltyDamageState", g_bodyDamageNames, GlobalData, m_movementPenaltyDamageState),
    FIELD_PARSE_PERCENT_TO_REAL("HealthBonus_Veteran", GlobalData, m_veteranHealthBonus),
    FIELD_PARSE_PERCENT_TO_REAL("HealthBonus_Elite", GlobalData, m_eliteHealthBonus),
    FIELD_PARSE_PERCENT_TO_REAL("HealthBonus_Heroic", GlobalData, m_heroicHealthBonus),
    FIELD_PARSE_PERCENT_TO_REAL("HumanSoloPlayerHealthBonus_Easy", GlobalData, m_easySoloHumanHealthBonus),
    FIELD_PARSE_PERCENT_TO_REAL("HumanSoloPlayerHealthBonus_Normal", GlobalData, m_normalSoloHumanHealthBonus),
    FIELD_PARSE_PERCENT_TO_REAL("HumanSoloPlayerHealthBonus_Hard", GlobalData, m_hardSoloHumanHealthBonus),
    FIELD_PARSE_PERCENT_TO_REAL("AISoloPlayerHealthBonus_Easy", GlobalData, m_easySoloAIHealthBonus),
    FIELD_PARSE_PERCENT_TO_REAL("AISoloPlayerHealthBonus_Normal", GlobalData, m_normalSoloAIHealthBonus),
    FIELD_PARSE_PERCENT_TO_REAL("AISoloPlayerHealthBonus_Hard", GlobalData, m_hardSoloAIHealthBonus),
    { "WeaponBonus", &WeaponBonusSet::Parse_Weapon_Bonus_Set_Ptr, nullptr, offsetof(GlobalData, m_weaponBonusSet) },
    FIELD_PARSE_REAL("DefaultStructureRubbleHeight", GlobalData, m_defaultStructureRubbleHeight),
    FIELD_PARSE_INT("FixedSeed", GlobalData, m_fixedSeed),
    FIELD_PARSE_ASCIISTRING("ShellMapName", GlobalData, m_shellMapName),
    FIELD_PARSE_BOOL("ShellMapOn", GlobalData, m_shellMapOn),
    FIELD_PARSE_BOOL("PlayIntro", GlobalData, m_playIntro),
    FIELD_PARSE_INT("FirewallBehavior", GlobalData, m_firewallBehaviour),
    FIELD_PARSE_INT("FirewallPortOverride", GlobalData, m_firewallPortOverrides),
    { "FirewallPortAllocationDelta", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_firewallPortAllocationDelta) },
    FIELD_PARSE_INT("GroupSelectMinSelectSize", GlobalData, m_groupSelectMinSelectSize),
    FIELD_PARSE_REAL("GroupSelectVolumeBase", GlobalData, m_groupSelectVolumeBase),
    FIELD_PARSE_REAL("GroupSelectVolumeIncrement", GlobalData, m_groupSelectVolumeIncrement),
    FIELD_PARSE_INT("MaxUnitSelectSounds", GlobalData, m_maxUnitSelectSounds),
    FIELD_PARSE_REAL("SelectionFlashSaturationFactor", GlobalData, m_selectionFlashSaturationFactor),
    FIELD_PARSE_BOOL("SelectionFlashHouseColor", GlobalData, m_selectionFlashHouseColor),
    FIELD_PARSE_REAL("CameraAudibleRadius", GlobalData, m_cameraAudibleRadius),
    FIELD_PARSE_REAL("GroupMoveClickToGatherAreaFactor", GlobalData, m_groupMoveClickToGatherAreaFactor),
    FIELD_PARSE_REAL("ShakeSubtleIntensity", GlobalData, m_shakeSubtleIntensity),
    FIELD_PARSE_REAL("ShakeNormalIntensity", GlobalData, m_shakeNormalIntensity),
    FIELD_PARSE_REAL("ShakeStrongIntensity", GlobalData, m_shakeStrongIntensity),
    FIELD_PARSE_REAL("ShakeSevereIntensity", GlobalData, m_shakeSevereIntensity),
    FIELD_PARSE_REAL("ShakeCineExtremeIntensity", GlobalData, m_shakeCineExtremeIntensity),
    FIELD_PARSE_REAL("ShakeCineInsaneIntensity", GlobalData, m_shakeCineInsaneIntensity),
    FIELD_PARSE_REAL("MaxShakeIntensity", GlobalData, m_maxShakeIntensity),
    FIELD_PARSE_REAL("MaxShakeRange", GlobalData, m_maxShakeRange),
    FIELD_PARSE_PERCENT_TO_REAL("SellPercentage", GlobalData, m_sellPercentage),
    FIELD_PARSE_PERCENT_TO_REAL("BaseRegenHealthPercentPerSecond", GlobalData, m_baseRegenHealthPercentPerSecond),
    { "BaseRegenDelay", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(GlobalData, m_baseRegenDelay) },
    FIELD_PARSE_ASCIISTRING("SpecialPowerViewObject", GlobalData, m_specialPowerViewObject),
    FIELD_PARSE_ASCIISTRING_VECTOR_APPEND("StandardPublicBone", GlobalData, m_standardPublicBones),
    FIELD_PARSE_BOOL("ShowMetrics", GlobalData, m_showMetrics),
    { "DefaultStartingCash", &Money::Parse_Money_Amount, nullptr, offsetof(GlobalData, m_defaultStartingCash) },
    FIELD_PARSE_RGB_COLOR("ShroudColor", GlobalData, m_shroudColor),
    FIELD_PARSE_UNSIGNED_BYTE("ClearAlpha", GlobalData, m_clearAlpha),
    FIELD_PARSE_UNSIGNED_BYTE("FogAlpha", GlobalData, m_fogAlpha),
    FIELD_PARSE_UNSIGNED_BYTE("ShroudAlpha", GlobalData, m_shroudAlpha),
    FIELD_PARSE_COLOR_INT("HotKeyTextColor", GlobalData, m_hotKeytextColor),
    FIELD_PARSE_INT("PowerBarBase", GlobalData, m_powerBarBase),
    FIELD_PARSE_REAL("PowerBarIntervals", GlobalData, m_powerBarIntervals),
    FIELD_PARSE_INT("PowerBarYellowRange", GlobalData, m_powerBarYellowRange),
    { "UnlookPersistDuration", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(GlobalData, m_unlookPersistDuration) },
    FIELD_PARSE_INT("NetworkFPSHistoryLength", GlobalData, m_networkFPSHistoryLength),
    FIELD_PARSE_INT("NetworkLatencyHistoryLength", GlobalData, m_networkLatencyHistoryLength),
    FIELD_PARSE_INT("NetworkRunAheadMetricsTime", GlobalData, m_networkRunAheadMetricsTime),
    FIELD_PARSE_INT("NetworkCushionHistoryLength", GlobalData, m_networkCushionHistoryLength),
    FIELD_PARSE_INT("NetworkRunAheadSlack", GlobalData, m_networkRunAheadSlack),
    FIELD_PARSE_INT("NetworkKeepAliveDelay", GlobalData, m_networkKeepAliveDelay),
    FIELD_PARSE_INT("NetworkDisconnectTime", GlobalData, m_networkDisconnecTime),
    FIELD_PARSE_INT("NetworkPlayerTimeoutTime", GlobalData, m_networkPlayerTimeOut),
    FIELD_PARSE_INT("NetworkDisconnectScreenNotifyTime", GlobalData, m_networkDisconnectScreenNotifyTime),
    FIELD_PARSE_REAL("KeyboardCameraRotateSpeed", GlobalData, m_keyboardCameraRotateSpeed),
    FIELD_PARSE_INT("PlayStats", GlobalData, m_playerStats),
    // C&C Generals contains a setting "UserDataLeafName" - to avoid crashes we need this line below
    // { "UserDataLeafName", &INI::Parse_AsciiString, nullptr, offsetof(GlobalData, m_userDataLeafName) },
    FIELD_PARSE_LAST
};
// clang-format on

GlobalData::GlobalData()
{
    if (s_theOriginal == nullptr) {
        s_theOriginal = this;
    }

    m_mapName.Clear();
    m_moveHintName.Clear();
    m_next = nullptr;
    m_demoToggleSpecialPowerDelays = false;
    m_playerStats = -1;
    m_setMinVertexBufferSize = false;
    m_useTrees = false;
    m_useTreeSway = true;
    m_extraAnimationsDisabled = false;
    m_useHeatEffects = true;
    m_useFPSLimit = false;
    m_dumpAssetUsage = false;
    m_framesPerSecondLimit = 0;
    m_chipsetType = 0;
    m_windowed = false;
    m_xResolution = 800;
    m_yResolution = 600;
    m_maxShellScreens = 0;
    m_useCloudMap = false;
    m_use3WayTerrainBlends = true;
    m_useLightMap = false;
    m_bilinearTerrainTexture = false;
    m_trilinearTerrainTexture = false;
    m_multiPassTerrain = false;
    m_adjustCliffTextures = false;
    m_stretchTerrain = false;
    m_useHalfHeightMap = false;
    m_terrainLOD = TERRAIN_LOD_AUTOMATIC;
    m_terrainLODTargetTimeMS = 0;
    m_dynamicLOD = true;
    m_useStaticLODLevels = true;
    m_rightMouseAlwaysScrolls = false;
    m_useWaterPlane = false;
    m_useCloudPlane = false;
    m_downWindAngle = -0.785f;
    m_shadowVolumes = false;
    m_shadowDecals = false;
    m_textureReductionFactor = -1;
    m_useBehindBuildingMarker = true;
    m_scriptDebug = false;
    m_particleEdit = false;
    m_displayDebug = false;
    m_winCursors = true;
    m_constantDebugUpdate = false;
    m_showTeamDot = false;
    m_fixedSeed = -1;
    m_horizontalScrollSpeedFactor = 1.0f;
    m_verticalScrollSpeedFactor = 1.0f;
    m_waterPositionX = 0.0f;
    m_waterPositionY = 0.0f;
    m_waterPositionZ = 0.0f;
    m_waterExtentX = 0.0f;
    m_waterExtentY = 0.0f;
    m_waterType = 0;
    m_featherWater = 0.0f;
    m_showSoftWaterEdge = true;
    m_useWaveEditor = false;
    m_unsortedShoreLines = false;
    m_showMetrics = false;

    // Set defaults for water effects.
    for (int i = 0; i < 4; ++i) {
        m_vertexWaterAvailableMaps[i].Clear();
        m_vertexWaterHeightClampLow[i] = 0.0f;
        m_vertexWaterHeightClampHigh[i] = 0.0f;
        m_vertexWaterAngle[i] = 0.0f;
        m_vertexWaterXPos[i] = 0.0f;
        m_vertexWaterYPos[i] = 0.0f;
        m_vertexWaterZPos[i] = 0.0f;
        m_vertexWaterXGridCells[i] = 0;
        m_vertexWaterYGridCells[i] = 0;
        m_vertexWaterGridSize[i] = 0.0f;
        m_vertexWaterAttenuationA[i] = 0.0f;
        m_vertexWaterAttenuationB[i] = 0.0f;
        m_vertexWaterAttenuationC[i] = 0.0f;
        m_vertexWaterAttenuationRange[i] = 0.0f;
    }

    m_skyBoxPositionZ = 0.0f;
    m_drawSkyBox = false;
    m_skyBoxScale = 4.5f;
    m_historicDamageLimit = 0;
    m_maxTerrainTracks = 0;
    m_ammoPipScaleFactor = 1.0f;
    m_containerPipScaleFactor = 1.0f;
    m_levelGainAnimTime = 0.0f;
    m_levelGainAnimZRise = 0.0f;
    m_getHealedAnimTime = 0.0f;
    m_getHealedAnimZRise = 0.0f;
    m_maxTankTrackEdges = 100;
    m_maxTankTrackOpaqueEdges = 25;
    m_maxTankTrackFadeDelay = 300000;
    m_timeOfDay = TIME_OF_DAY_AFTERNOON;
    m_weather = WEATHER_NORMAL;
    m_makeTrackMarks = false;
    m_hideGarrisonFlags = false;
    m_forceModelsFollowTimeOfDay = true;
    m_forceModelsFollowWeather = true;
    m_partitionCellSize = 0.0f;
    m_ammoPipWorldOffset.Zero();
    m_containerPipWorldOffset.Zero();
    m_ammoPipScreenOffset.x = 0.0f;
    m_ammoPipScreenOffset.y = 0.0f;
    m_containerPipScreenOffset.x = 0.0f;
    m_containerPipScreenOffset.y = 0.0f;

    // Set defaults for the lighting
    for (int i = 0; i < 3; ++i) {
        m_terrainAmbient[i].red = 0.0f;
        m_terrainAmbient[i].green = 0.0f;
        m_terrainAmbient[i].blue = 0.0f;
        m_terrainDiffuse[i].red = 0.0f;
        m_terrainDiffuse[i].green = 0.0f;
        m_terrainDiffuse[i].blue = 0.0f;
        m_terrainLightPos[i].x = 0.0f;
        m_terrainLightPos[i].y = 0.0f;
        m_terrainLightPos[i].z = -1.0f;

        for (int j = 0; j < TIME_OF_DAY_COUNT; ++j) {
            m_terrainPlaneLighting[j][i].ambient.red = 0.0f;
            m_terrainPlaneLighting[j][i].ambient.green = 0.0f;
            m_terrainPlaneLighting[j][i].ambient.blue = 0.0f;
            m_terrainPlaneLighting[j][i].diffuse.red = 0.0f;
            m_terrainPlaneLighting[j][i].diffuse.green = 0.0f;
            m_terrainPlaneLighting[j][i].diffuse.blue = 0.0f;
            m_terrainPlaneLighting[j][i].lightPos.x = 0.0f;
            m_terrainPlaneLighting[j][i].lightPos.y = 0.0f;
            m_terrainPlaneLighting[j][i].lightPos.z = -1.0f;
            m_terrainObjectLighting[j][i].ambient.red = 0.0f;
            m_terrainObjectLighting[j][i].ambient.green = 0.0f;
            m_terrainObjectLighting[j][i].ambient.blue = 0.0f;
            m_terrainObjectLighting[j][i].diffuse.red = 0.0f;
            m_terrainObjectLighting[j][i].diffuse.green = 0.0f;
            m_terrainObjectLighting[j][i].diffuse.blue = 0.0f;
            m_terrainObjectLighting[j][i].lightPos.x = 0.0f;
            m_terrainObjectLighting[j][i].lightPos.y = 0.0f;
            m_terrainObjectLighting[j][i].lightPos.z = -1.0f;
        }
    }

    m_infantryLight[TIME_OF_DAY_MORNING] = 1.5f;
    m_infantryLight[TIME_OF_DAY_AFTERNOON] = 1.5f;
    m_infantryLight[TIME_OF_DAY_EVENING] = 1.5f;
    m_infantryLight[TIME_OF_DAY_NIGHT] = 1.5f;
    m_infantryLightOverride = -1.0f;
    m_numberGlobalLights = LIGHT_COUNT;
    m_maxRoadSegments = 0;
    m_maxRoadVertex = 0;
    m_maxRoadIndex = 0;
    m_maxRoadTypes = 0;
    m_valuesPerSupplyBox = 100;
    m_audioOn = true;
    m_musicOn = true;
    m_soundsOn = true;
    m_sounds3DOn = true;
    m_speechOn = true;
    m_videoOn = true;
    m_disableCameraMovements = false;
    m_maxTranslucencyObjects = 512;
    m_maxOccludedBuildings = 512;
    m_maxOccludedObjects = 512;
    m_maxOccludedOthers = 512;
    m_occludedColorLuminanceScale = 0.5f;
    m_useFX = true;
    m_frameToJumpTo = 0;
    m_particleScale = 1.0f;
    m_autoFireParticleSmallMax = 0;
    m_autoFireParticleMediumMax = 0;
    m_autoFireParticleLargeMax = 0;
    m_autoSmokeParticleSmallMax = 0;
    m_autoSmokeParticleMediumMax = 0;
    m_autoSmokeParticleLargeMax = 0;
    m_autoFireParticleSmallPrefix.Clear();
    m_autoFireParticleMediumPrefix.Clear();
    m_autoFireParticleLargePrefix.Clear();
    m_autoSmokeParticleSmallPrefix.Clear();
    m_autoSmokeParticleMediumPrefix.Clear();
    m_autoSmokeParticleLargePrefix.Clear();
    m_autoAFlameParticlePrefix.Clear();
    m_autoFireParticleSmallSystem.Clear();
    m_autoFireParticleMediumSystem.Clear();
    m_autoFireParticleLargeSystem.Clear();
    m_autoSmokeParticleSmallSystem.Clear();
    m_autoSmokeParticleMediumSystem.Clear();
    m_autoSmokeParticleLargeSystem.Clear();
    m_autoAFlameParticleSystem.Clear();
    m_levelGainAnimName.Clear();
    m_getHealedAnimName.Clear();
    m_specialPowerViewObject.Clear();
    m_drawEntireTerrain = false;
    m_maxParticleCount = 0;
    m_maxFieldParticleCount = 30;
    m_debugAI = false;
    m_logSupplyCenterPlacement = false;
    m_debugObstacleAI = false;
    m_showClientPhysics = true;
    m_showTerrainNormals = false;
    m_showObjectHealth = false;
    m_particleEdit = false;
    m_cameraPitch = 0.0f;
    m_cameraYaw = 0.0f;
    m_cameraHeight = 0.0f;
    m_minCameraHeight = 0.01f;
    m_maxCameraHeight = 0.03f;
    m_terrainHeightAtMapEdge = 0.0f;
    m_unitDamagedThreshold = 0.5f;
    m_unitReallyDamagedThreshold = 0.1f;
    m_groundStiffness = 0.5f;
    m_structureStiffness = 0.5f;
    m_gravity = -1.0f;
    m_stealthFriendlyOpacity = 0.5f;
    m_defaultOcclusionDelay = 90;
    m_preloadAssets = false;
    m_preloadEverything = false;
    m_logAssets = false;
    m_netMinPlayers = 1;
    m_lanIPAddress = 0;
    m_buildSpeed = 0.0f;
    m_minDistanceFromMapEdgeForBuild = 0.0f;
    m_supplyBuildOrder = 0;
    m_allowedHeightVariationForBuildings = 0.0f;
    m_minLowEnergyProductionSpeed = 0.0f;
    m_maxLowEnergyProductionSpeed = 0.0f;
    m_lowEnergyPenaltyModifier = 0.0f;
    m_multipleFactory = 0.0f;
    m_refundPercent = 0.0f;
    m_commandCenterHealRange = 0.0f;
    m_commandCenterHealAmmount = 0.0f;
    m_maxTunnelCapacity = 0;
    m_maxLineBuildObjects = 0;
    m_standardMinefieldDensity = 0.01f;
    m_standardMinefieldDistance = 40.0f;
    m_groupSelectMinSelectSize = 5;
    m_groupSelectVolumeBase = 0.5f;
    m_groupSelectVolumeIncrement = 0.02f;
    m_maxUnitSelectSounds = 8;
    m_selectionFlashSaturationFactor = 0.5f;
    m_selectionFlashHouseColor = false;
    m_cameraAudibleRadius = 500.0f;
    m_groupMoveClickToGatherAreaFactor = 1.0f;
    m_shakeSubtleIntensity = 0.5f;
    m_shakeNormalIntensity = 1.0f;
    m_shakeStrongIntensity = 2.5f;
    m_shakeSevereIntensity = 5.0f;
    m_shakeCineExtremeIntensity = 8.0f;
    m_shakeCineInsaneIntensity = 12.0f;
    m_maxShakeIntensity = 10.0f;
    m_maxShakeRange = 150.0f;
    m_sellPercentage = 1.0f;
    m_baseRegenHealthPercentPerSecond = 0.0f;
    m_baseRegenDelay = 0;
    m_hotKeytextColor = 0xFFFFFF00;
    m_shroudColor.red = 1.0f;
    m_shroudColor.green = 1.0f;
    m_shroudColor.blue = 1.0f;
    m_clearAlpha = 0xFF;
    m_fogAlpha = 0x7F;
    m_shroudAlpha = 0;
    m_powerBarBase = 7;
    m_powerBarIntervals = 3.0f;
    m_powerBarYellowRange = 5;
    m_gammaValue = 1.0f;
    m_standardPublicBones.clear();
    m_antiAliasBoxValue = 0; // possibly float
    m_languageFilter = true;
    m_firewallBehaviour = 0;
    m_sendDelay = 0;
    m_firewallPortOverrides = 0;
    m_firewallPortAllocationDelta = 0;
    m_loadScreenDemo = false;
    m_demoToggleRender = false;
    m_saveCameraInReplays = false;
    m_useCameraInReplays = false;
    m_showFrameRateBar = false;
    m_unlookPersistDuration = 30;
    m_networkFPSHistoryLength = 30;
    m_networkLatencyHistoryLength = 200;
    m_networkRunAheadMetricsTime = 500;
    m_networkCushionHistoryLength = 10;
    m_networkRunAheadSlack = 10;
    m_networkKeepAliveDelay = 20;
    m_networkDisconnecTime = 5000;
    m_networkPlayerTimeOut = 60000;
    m_networkDisconnectScreenNotifyTime = 15000;
    m_unkBool25 = false;
    m_unkBool26 = false;

    if (m_timeOfDay > TIME_OF_DAY_INVALID && m_timeOfDay < TIME_OF_DAY_COUNT) {
        for (int i = 0; i < LIGHT_COUNT; ++i) {
            m_terrainAmbient[i] = m_terrainPlaneLighting[m_timeOfDay][i].ambient;
            m_terrainDiffuse[i] = m_terrainPlaneLighting[m_timeOfDay][i].diffuse;
            m_terrainLightPos[i] = m_terrainPlaneLighting[m_timeOfDay][i].lightPos;
        }
    }

    m_buildMapCache = false;
    m_initialFile.Clear();
    m_pendingFile.Clear();
    m_normalHealthBonus = 1.0f;
    m_veteranHealthBonus = 1.0f;
    m_eliteHealthBonus = 1.0f;
    m_heroicHealthBonus = 1.0f;
    m_easySoloHumanHealthBonus = 1.0f;
    m_normalSoloHumanHealthBonus = 1.0f;
    m_hardSoloHumanHealthBonus = 1.0f;
    m_easySoloAIHealthBonus = 1.0f;
    m_normalSoloAIHealthBonus = 1.0f;
    m_hardSoloAIHealthBonus = 1.0f;
    m_defaultStructureRubbleHeight = 1.0f;
    m_weaponBonusSet = NEW_POOL_OBJ(WeaponBonusSet);
    m_shellMapName = "Maps/ShellMap1/ShellMap1.map";
    m_shellMapOn = true;
    m_playIntro = true;
    m_playSizzle = true;
    m_afterIntro = false;
    m_unkBool16 = false;
    m_unkBool17 = false;
    m_musicVolumeFactor = 0.5f;
    m_SFXVolumeFactor = 0.5f;
    m_voiceVolumeFactor = 0.5f;
    m_sound3DPref = false;
    m_keyboardScrollFactor = 0.5f;
    m_keyboardDefaultScrollFactor = 0.5f;
    m_scrollAmountCutoff = 10.0f;
    m_cameraAdjustSpeed = 0.1f;
    m_enforceMaxCameraHeight = true;
    m_animateWindows = true;
    m_iniCRC = 0;
    m_gameCRC = 0;
    CRC crc; // Prepare to generate a crc of parts of the game state.

    // Original crc's the exe, but that won't work cross platform so use full commit hash from git.
    crc.Compute_CRC(g_gitSHA1, strlen(g_gitSHA1));

    // Add game version to the crc.
    if (g_theVersion) {
        // Make sure bytes are consistently correct way round as crc'd
        // as a byte array.
        int32_t version = htole32(g_theVersion->Get_Version_Number());
        crc.Compute_CRC(&version, sizeof(version));
    }

    // crc the skirmish and multiplayer scripts.
    int read = 0;
    uint8_t buffer[CRC_BUFFER_SIZE];
    File *scriptfile = g_theFileSystem->Open("Data/Scripts/SkirmishScripts.scb", File::BINARY | File::READ);
    if (scriptfile) {
        while ((read = scriptfile->Read(buffer, sizeof(buffer))) != 0) {
            crc.Compute_CRC(buffer, read);
        }

        scriptfile->Close();
    }
    scriptfile = g_theFileSystem->Open("Data/Scripts/MultiplayerScripts.scb", File::BINARY | File::READ);
    if (scriptfile) {
        while ((read = scriptfile->Read(buffer, sizeof(buffer))) != 0) {
            crc.Compute_CRC(buffer, read);
        }

        scriptfile->Close();
    }

    m_gameCRC = crc.Get_CRC();
    m_movementPenaltyDamageState = BODY_REALLYDAMAGED;
    m_updateTGAtoDDS = false;
#ifdef PLATFORM_WINDOWS
    m_doubleClickTime = GetDoubleClickTime();
#else
// TODO, probably based on whatever crossplatform event framework we use.
#endif
    m_keyboardCameraRotateSpeed = 0.1f;
#ifdef PLATFORM_WINDOWS
    m_userDataDirectory = getenv("CSIDL_MYDOCUMENTS");
    m_userDataDirectory += "\\Command and Conquer Generals Zero Hour Data\\";
#elif PLATFORM_LINUX
// TODO
#elif PLATFORM_OSX
// TODO
#endif // PLATFORM_WINDOWS
    g_theFileSystem->Create_Dir(m_userDataDirectory);
    m_retaliationModeEnabled = true;
    captainslog_info("User data directory is set to '%s'.", m_userDataDirectory.Str());
}

GlobalData::~GlobalData()
{
    if (s_theOriginal == this) {
        s_theOriginal = nullptr;
        g_theWriteableGlobalData = nullptr;
    }
}

void GlobalData::Reset()
{
    while (g_theWriteableGlobalData != s_theOriginal) {
        GlobalData *tmp = g_theWriteableGlobalData->m_next;
        delete g_theWriteableGlobalData;
        g_theWriteableGlobalData = tmp;
    }
}

bool GlobalData::Set_Time_Of_Day(TimeOfDayType time)
{
    if (time <= TIME_OF_DAY_INVALID || time >= TIME_OF_DAY_COUNT) {
        return false;
    }

    m_timeOfDay = time;

    for (int i = 0; i < LIGHT_COUNT; ++i) {
        m_terrainAmbient[i] = m_terrainPlaneLighting[time][i].ambient;
        m_terrainDiffuse[i] = m_terrainPlaneLighting[time][i].diffuse;
        m_terrainLightPos[i] = m_terrainPlaneLighting[time][i].lightPos;
    }

    return true;
}

// Was originally called through INI::parseGameDataDefinition
void GlobalData::Parse_Game_Data_Definition(INI *ini)
{
    captainslog_info("Parsing Global Data from '%s'.", ini->Get_Filename().Str());

    if (g_theWriteableGlobalData == nullptr) {
        g_theWriteableGlobalData = new GlobalData;
    } else if (ini->Get_Load_Type() != INI_LOAD_UNK && ini->Get_Load_Type() == INI_LOAD_CREATE_OVERRIDES) {
        GlobalData *tmp = new GlobalData;
        tmp->m_next = g_theWriteableGlobalData;
        g_theWriteableGlobalData = tmp;
    }

    ini->Init_From_INI(g_theWriteableGlobalData, s_fieldParseTable);

    OptionPreferences opts;
    g_theWriteableGlobalData->m_alternateMouseEnabled = opts.Get_Alternate_Mouse_Mode_Enabled();
    g_theWriteableGlobalData->m_retaliationModeEnabled = opts.Get_Retaliation_Mode_Enabled();
    g_theWriteableGlobalData->m_doubleClickAttackMovesEnabled = opts.Get_Double_Click_Attack_Move_Enabled();
    g_theWriteableGlobalData->m_keyboardScrollFactor = opts.Get_Scroll_Factor();
    g_theWriteableGlobalData->m_lanIPAddress = opts.Get_LAN_IPAddress();
    g_theWriteableGlobalData->m_sendDelay = opts.Get_Send_Delay();
    g_theWriteableGlobalData->m_firewallBehaviour = opts.Get_Firewall_Behavior();
    g_theWriteableGlobalData->m_firewallPortAllocationDelta = opts.Get_Firewall_Port_Allocation_Delta();
    g_theWriteableGlobalData->m_firewallPortOverrides = opts.Get_Firewall_Port_Override();
    g_theWriteableGlobalData->m_saveCameraInReplays = opts.Save_Camera_In_Replays();
    g_theWriteableGlobalData->m_useCameraInReplays = opts.Use_Camera_In_Replays();
    int gamma = opts.Get_Gamma_Value();

    if (gamma >= 50) {
        if (gamma > 50) {
            g_theWriteableGlobalData->m_gammaValue = ((gamma - 50) / 50.0f) + 1.0f;
        }
    } else if (gamma <= 0) {
        g_theWriteableGlobalData->m_gammaValue = 0.6f;
    } else {
        g_theWriteableGlobalData->m_gammaValue = (((50 - gamma) * 0.4f) / -50.0f) + 1.0f;
    }

    int x;
    int y;
    opts.Get_Resolution(&x, &y);
    g_theWriteableGlobalData->m_xResolution = x;
    g_theWriteableGlobalData->m_yResolution = y;
}
