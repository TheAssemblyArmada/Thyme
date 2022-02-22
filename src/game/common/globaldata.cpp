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

static const char *_terrain_lod_names[] = { "NONE",
    "MIN",
    "STRETCH_NO_CLOUDS",
    "HALF_CLOUDS",
    "NO_CLOUDS",
    "STRETCH_CLOUDS",
    "NO_WATER",
    "MAX",
    "AUTOMATIC",
    "DISABLE",
    nullptr };

// List of keys handled in the ini
// Class contains some variables that don't appear to be user
// controlled.
FieldParse GlobalData::s_fieldParseTable[] = {
    { "Windowed", &INI::Parse_Bool, nullptr, offsetof(GlobalData, m_windowed) }, //
    { "XResolution", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_xResolution) }, //
    { "YResolution", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_yResolution) }, //
    { "MapName", &INI::Parse_AsciiString, nullptr, offsetof(GlobalData, m_mapName) }, //
    { "MoveHintName", &INI::Parse_AsciiString, nullptr, offsetof(GlobalData, m_moveHintName) }, //
    { "UseTrees", &INI::Parse_Bool, nullptr, offsetof(GlobalData, m_useTrees) }, //
    { "UseFPSLimit", &INI::Parse_Bool, nullptr, offsetof(GlobalData, m_useFPSLimit) }, //
    { "DumpAssetUsage", &INI::Parse_Bool, nullptr, offsetof(GlobalData, m_dumpAssetUsage) }, //
    { "FramesPerSecondLimit", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_framesPerSecondLimit) }, //
    { "ChipsetType", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_chipsetType) }, //
    { "MaxShellScreens", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_maxShellScreens) }, //
    { "UseCloudMap", &INI::Parse_Bool, nullptr, offsetof(GlobalData, m_useCloudMap) }, //
    { "UseLightMap", &INI::Parse_Bool, nullptr, offsetof(GlobalData, m_useLightMap) }, //
    { "BilinearTerrainTex", &INI::Parse_Bool, nullptr, offsetof(GlobalData, m_bilinearTerrainTexture) }, //
    { "TrilinearTerrainTex", &INI::Parse_Bool, nullptr, offsetof(GlobalData, m_trilinearTerrainTexture) }, //
    { "MultiPassTerrain", &INI::Parse_Bool, nullptr, offsetof(GlobalData, m_multiPassTerrain) }, //
    { "AdjustCliffTextures", &INI::Parse_Bool, nullptr, offsetof(GlobalData, m_adjustCliffTextures) }, //
    { "Use3WayTerrainBlends", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_use3WayTerrainBlends) }, //
    { "StretchTerrain", &INI::Parse_Bool, nullptr, offsetof(GlobalData, m_stretchTerrain) }, //
    { "UseHalfHeightMap", &INI::Parse_Bool, nullptr, offsetof(GlobalData, m_useHalfHeightMap) }, //
    { "DrawEntireTerrain", &INI::Parse_Bool, nullptr, offsetof(GlobalData, m_drawEntireTerrain) }, //
    //{ "TerrainLOD", &INI::Parse_Index_List, &TerrainLODNames, 64 },//
    { "TerrainLOD", &INI::Parse_Index_List, _terrain_lod_names, offsetof(GlobalData, m_terrainLOD) }, //
    { "TerrainLODTargetTimeMS", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_terrainLODTargetTimeMS) }, //
    { "RightMouseAlwaysScrolls", &INI::Parse_Bool, nullptr, offsetof(GlobalData, m_rightMouseAlwaysScrolls) }, //
    { "UseWaterPlane", &INI::Parse_Bool, nullptr, offsetof(GlobalData, m_useWaterPlane) }, //
    { "UseCloudPlane", &INI::Parse_Bool, nullptr, offsetof(GlobalData, m_useCloudPlane) }, //
    { "DownwindAngle", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_downWindAngle) }, //
    { "UseShadowVolumes", &INI::Parse_Bool, nullptr, offsetof(GlobalData, m_shadowVolumes) }, //
    { "UseShadowDecals", &INI::Parse_Bool, nullptr, offsetof(GlobalData, m_shadowDecals) }, //
    { "TextureReductionFactor", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_textureReductionFactor) }, //
    { "UseBehindBuildingMarker", &INI::Parse_Bool, nullptr, offsetof(GlobalData, m_useBehindBuildingMarker) }, //
    { "WaterPositionX", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_waterPositionX) }, //
    { "WaterPositionY", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_waterPositionY) }, //
    { "WaterPositionZ", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_waterPositionZ) }, //
    { "WaterExtentX", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_waterExtentX) }, //
    { "WaterExtentY", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_waterExtentY) }, //
    { "WaterType", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_waterType) }, //
    { "FeatherWater", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_featherWater) }, //
    { "ShowSoftWaterEdge", &INI::Parse_Bool, nullptr, offsetof(GlobalData, m_showSoftWaterEdge) }, //
    { "VertexWaterAvailableMaps1",
        &INI::Parse_AsciiString,
        nullptr,
        offsetof(GlobalData, m_vertexWaterAvailableMaps[0]) }, //
    { "VertexWaterHeightClampLow1", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_vertexWaterHeightClampLow[0]) }, //
    { "VertexWaterHeightClampHi1", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_vertexWaterHeightClampHigh[0]) }, //
    { "VertexWaterAngle1", &INI::Parse_Angle_Real, nullptr, offsetof(GlobalData, m_vertexWaterAngle[0]) }, //
    { "VertexWaterXPosition1", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_vertexWaterXPos[0]) }, //
    { "VertexWaterYPosition1", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_vertexWaterYPos[0]) }, //
    { "VertexWaterZPosition1", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_vertexWaterZPos[0]) }, //
    { "VertexWaterXGridCells1", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_vertexWaterXGridCells[0]) },
    { "VertexWaterYGridCells1", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_vertexWaterYGridCells[0]) },
    { "VertexWaterGridSize1", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_vertexWaterGridSize[0]) }, //
    { "VertexWaterAttenuationA1", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_vertexWaterAttenuationA[0]) },
    { "VertexWaterAttenuationB1", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_vertexWaterAttenuationB[0]) },
    { "VertexWaterAttenuationC1", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_vertexWaterAttenuationC[0]) },
    { "VertexWaterAttenuationRange1", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_vertexWaterAttenuationRange[0]) },
    { "VertexWaterAvailableMaps2",
        &INI::Parse_AsciiString,
        nullptr,
        offsetof(GlobalData, m_vertexWaterAvailableMaps[1]) }, //
    { "VertexWaterHeightClampLow2", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_vertexWaterHeightClampLow[1]) }, //
    { "VertexWaterHeightClampHi2", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_vertexWaterHeightClampHigh[1]) }, //
    { "VertexWaterAngle2", &INI::Parse_Angle_Real, nullptr, offsetof(GlobalData, m_vertexWaterAngle[1]) }, //
    { "VertexWaterXPosition2", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_vertexWaterXPos[1]) }, //
    { "VertexWaterYPosition2", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_vertexWaterYPos[1]) }, //
    { "VertexWaterZPosition2", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_vertexWaterZPos[1]) }, //
    { "VertexWaterXGridCells2", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_vertexWaterXGridCells[1]) },
    { "VertexWaterYGridCells2", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_vertexWaterYGridCells[1]) },
    { "VertexWaterGridSize2", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_vertexWaterGridSize[1]) }, //
    { "VertexWaterAttenuationA2", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_vertexWaterAttenuationA[1]) },
    { "VertexWaterAttenuationB2", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_vertexWaterAttenuationB[1]) },
    { "VertexWaterAttenuationC2", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_vertexWaterAttenuationC[1]) },
    { "VertexWaterAttenuationRange2", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_vertexWaterAttenuationRange[1]) },
    { "VertexWaterAvailableMaps3",
        &INI::Parse_AsciiString,
        nullptr,
        offsetof(GlobalData, m_vertexWaterAvailableMaps[2]) }, //
    { "VertexWaterHeightClampLow3", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_vertexWaterHeightClampLow[2]) }, //
    { "VertexWaterHeightClampHi3", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_vertexWaterHeightClampHigh[2]) }, //
    { "VertexWaterAngle3", &INI::Parse_Angle_Real, nullptr, offsetof(GlobalData, m_vertexWaterAngle[2]) }, //
    { "VertexWaterXPosition3", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_vertexWaterXPos[2]) }, //
    { "VertexWaterYPosition3", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_vertexWaterYPos[2]) }, //
    { "VertexWaterZPosition3", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_vertexWaterZPos[2]) }, //
    { "VertexWaterXGridCells3", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_vertexWaterXGridCells[2]) },
    { "VertexWaterYGridCells3", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_vertexWaterYGridCells[2]) },
    { "VertexWaterGridSize3", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_vertexWaterGridSize[2]) }, //
    { "VertexWaterAttenuationA3", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_vertexWaterAttenuationA[2]) },
    { "VertexWaterAttenuationB3", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_vertexWaterAttenuationB[2]) },
    { "VertexWaterAttenuationC3", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_vertexWaterAttenuationC[2]) },
    { "VertexWaterAttenuationRange3", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_vertexWaterAttenuationRange[2]) },
    { "VertexWaterAvailableMaps4",
        &INI::Parse_AsciiString,
        nullptr,
        offsetof(GlobalData, m_vertexWaterAvailableMaps[3]) }, //
    { "VertexWaterHeightClampLow4", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_vertexWaterHeightClampLow[3]) }, //
    { "VertexWaterHeightClampHi4", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_vertexWaterHeightClampHigh[3]) }, //
    { "VertexWaterAngle4", &INI::Parse_Angle_Real, nullptr, offsetof(GlobalData, m_vertexWaterAngle[3]) }, //
    { "VertexWaterXPosition4", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_vertexWaterXPos[3]) }, //
    { "VertexWaterYPosition4", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_vertexWaterYPos[3]) }, //
    { "VertexWaterZPosition4", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_vertexWaterZPos[3]) }, //
    { "VertexWaterXGridCells4", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_vertexWaterXGridCells[3]) },
    { "VertexWaterYGridCells4", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_vertexWaterYGridCells[3]) },
    { "VertexWaterGridSize4", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_vertexWaterGridSize[3]) }, //
    { "VertexWaterAttenuationA4", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_vertexWaterAttenuationA[3]) },
    { "VertexWaterAttenuationB4", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_vertexWaterAttenuationB[3]) },
    { "VertexWaterAttenuationC4", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_vertexWaterAttenuationC[3]) },
    { "VertexWaterAttenuationRange4", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_vertexWaterAttenuationRange[3]) },
    { "SkyBoxPositionZ", &INI::Parse_Real, nullptr, 352 }, //
    { "SkyBoxScale", &INI::Parse_Real, nullptr, 360 }, //
    { "DrawSkyBox", &INI::Parse_Bool, nullptr, 356 }, //
    { "CameraPitch", &INI::Parse_Real, nullptr, 364 }, //
    { "CameraYaw", &INI::Parse_Real, nullptr, 368 }, //
    { "CameraHeight", &INI::Parse_Real, nullptr, 372 }, //
    { "MaxCameraHeight", &INI::Parse_Real, nullptr, 376 }, //
    { "MinCameraHeight", &INI::Parse_Real, nullptr, 380 }, //
    { "TerrainHeightAtEdgeOfMap", &INI::Parse_Real, nullptr, 384 }, //
    { "UnitDamagedThreshold", &INI::Parse_Real, nullptr, 388 }, //
    { "UnitReallyDamagedThreshold", &INI::Parse_Real, nullptr, 392 }, //
    { "GroundStiffness", &INI::Parse_Real, nullptr, 396 }, //
    { "StructureStiffness", &INI::Parse_Real, nullptr, 400 }, //
    { "Gravity", &INI::Parse_Acceleration_Real, nullptr, 404 }, //
    { "StealthFriendlyOpacity", &INI::Parse_Percent_To_Real, nullptr, 408 }, //
    { "DefaultOcclusionDelay", &INI::Parse_Duration_Unsigned_Int, nullptr, 412 }, //
    { "PartitionCellSize", &INI::Parse_Real, nullptr, 420 }, //
    { "AmmoPipScaleFactor", &INI::Parse_Real, nullptr, 464 }, //
    { "ContainerPipScaleFactor", &INI::Parse_Real, nullptr, 468 }, //
    { "AmmoPipWorldOffset", &INI::Parse_Coord3D, nullptr, 424 }, //
    { "ContainerPipWorldOffset", &INI::Parse_Coord3D, nullptr, 436 }, //
    { "AmmoPipScreenOffset", &INI::Parse_Coord2D, nullptr, 448 }, //
    { "ContainerPipScreenOffset", &INI::Parse_Coord2D, nullptr, 456 }, //
    { "HistoricDamageLimit", &INI::Parse_Duration_Unsigned_Int, nullptr, 472 }, //
    { "MaxTerrainTracks", &INI::Parse_Int, nullptr, 476 }, //
    //{ "TimeOfDay", &INI::Parse_Index_List, &TimeOfDayNames, 516 },//
    //{ "Weather", &INI::Parse_Index_List, &WeatherNames, 520 },//
    { "TimeOfDay", &INI::Parse_Index_List, g_timeOfDayNames, offsetof(GlobalData, m_timeOfDay) }, //
    { "Weather", &INI::Parse_Index_List, g_weatherNames, offsetof(GlobalData, m_weather) }, //
    { "MakeTrackMarks", &INI::Parse_Bool, nullptr, 524 }, //
    { "HideGarrisonFlags", &INI::Parse_Bool, nullptr, 525 }, //
    { "ForceModelsToFollowTimeOfDay", &INI::Parse_Bool, nullptr, 526 }, //
    { "ForceModelsToFollowWeather", &INI::Parse_Bool, nullptr, 527 }, //
    { "LevelGainAnimationName", &INI::Parse_AsciiString, nullptr, 492 }, //
    { "LevelGainAnimationTime", &INI::Parse_Real, nullptr, 496 }, //
    { "LevelGainAnimationZRise", &INI::Parse_Real, nullptr, 500 }, //
    { "GetHealedAnimationName", &INI::Parse_AsciiString, nullptr, 504 }, //
    { "GetHealedAnimationTime", &INI::Parse_Real, nullptr, 508 }, //
    { "GetHealedAnimationZRise", &INI::Parse_Real, nullptr, 512 }, //
    { "TerrainLightingMorningAmbient", &INI::Parse_RGB_Color, nullptr, 636 },
    { "TerrainLightingMorningDiffuse", &INI::Parse_RGB_Color, nullptr, 648 },
    { "TerrainLightingMorningLightPos", &INI::Parse_Coord3D, nullptr, 660 },
    { "TerrainLightingAfternoonAmbient", &INI::Parse_RGB_Color, nullptr, 744 },
    { "TerrainLightingAfternoonDiffuse", &INI::Parse_RGB_Color, nullptr, 756 },
    { "TerrainLightingAfternoonLightPos", &INI::Parse_Coord3D, nullptr, 768 },
    { "TerrainLightingEveningAmbient", &INI::Parse_RGB_Color, nullptr, 852 },
    { "TerrainLightingEveningDiffuse", &INI::Parse_RGB_Color, nullptr, 864 },
    { "TerrainLightingEveningLightPos", &INI::Parse_Coord3D, nullptr, 876 },
    { "TerrainLightingNightAmbient", &INI::Parse_RGB_Color, nullptr, 960 },
    { "TerrainLightingNightDiffuse", &INI::Parse_RGB_Color, nullptr, 972 },
    { "TerrainLightingNightLightPos", &INI::Parse_Coord3D, nullptr, 984 },
    { "TerrainObjectsLightingMorningAmbient", &INI::Parse_RGB_Color, nullptr, 1176 },
    { "TerrainObjectsLightingMorningDiffuse", &INI::Parse_RGB_Color, nullptr, 1188 },
    { "TerrainObjectsLightingMorningLightPos", &INI::Parse_Coord3D, nullptr, 1200 },
    { "TerrainObjectsLightingAfternoonAmbient", &INI::Parse_RGB_Color, nullptr, 1284 },
    { "TerrainObjectsLightingAfternoonDiffuse", &INI::Parse_RGB_Color, nullptr, 1296 },
    { "TerrainObjectsLightingAfternoonLightPos", &INI::Parse_Coord3D, nullptr, 1308 },
    { "TerrainObjectsLightingEveningAmbient", &INI::Parse_RGB_Color, nullptr, 1392 },
    { "TerrainObjectsLightingEveningDiffuse", &INI::Parse_RGB_Color, nullptr, 1404 },
    { "TerrainObjectsLightingEveningLightPos", &INI::Parse_Coord3D, nullptr, 1416 },
    { "TerrainObjectsLightingNightAmbient", &INI::Parse_RGB_Color, nullptr, 1500 },
    { "TerrainObjectsLightingNightDiffuse", &INI::Parse_RGB_Color, nullptr, 1512 },
    { "TerrainObjectsLightingNightLightPos", &INI::Parse_Coord3D, nullptr, 1524 },
    { "TerrainLightingMorningAmbient2", &INI::Parse_RGB_Color, nullptr, 672 },
    { "TerrainLightingMorningDiffuse2", &INI::Parse_RGB_Color, nullptr, 684 },
    { "TerrainLightingMorningLightPos2", &INI::Parse_Coord3D, nullptr, 696 },
    { "TerrainLightingAfternoonAmbient2", &INI::Parse_RGB_Color, nullptr, 780 },
    { "TerrainLightingAfternoonDiffuse2", &INI::Parse_RGB_Color, nullptr, 792 },
    { "TerrainLightingAfternoonLightPos2", &INI::Parse_Coord3D, nullptr, 804 },
    { "TerrainLightingEveningAmbient2", &INI::Parse_RGB_Color, nullptr, 888 },
    { "TerrainLightingEveningDiffuse2", &INI::Parse_RGB_Color, nullptr, 900 },
    { "TerrainLightingEveningLightPos2", &INI::Parse_Coord3D, nullptr, 912 },
    { "TerrainLightingNightAmbient2", &INI::Parse_RGB_Color, nullptr, 996 },
    { "TerrainLightingNightDiffuse2", &INI::Parse_RGB_Color, nullptr, 1008 },
    { "TerrainLightingNightLightPos2", &INI::Parse_Coord3D, nullptr, 1020 },
    { "TerrainObjectsLightingMorningAmbient2", &INI::Parse_RGB_Color, nullptr, 1212 },
    { "TerrainObjectsLightingMorningDiffuse2", &INI::Parse_RGB_Color, nullptr, 1224 },
    { "TerrainObjectsLightingMorningLightPos2", &INI::Parse_Coord3D, nullptr, 1236 },
    { "TerrainObjectsLightingAfternoonAmbient2", &INI::Parse_RGB_Color, nullptr, 1320 },
    { "TerrainObjectsLightingAfternoonDiffuse2", &INI::Parse_RGB_Color, nullptr, 1332 },
    { "TerrainObjectsLightingAfternoonLightPos2", &INI::Parse_Coord3D, nullptr, 1344 },
    { "TerrainObjectsLightingEveningAmbient2", &INI::Parse_RGB_Color, nullptr, 1428 },
    { "TerrainObjectsLightingEveningDiffuse2", &INI::Parse_RGB_Color, nullptr, 1440 },
    { "TerrainObjectsLightingEveningLightPos2", &INI::Parse_Coord3D, nullptr, 1452 },
    { "TerrainObjectsLightingNightAmbient2", &INI::Parse_RGB_Color, nullptr, 1536 },
    { "TerrainObjectsLightingNightDiffuse2", &INI::Parse_RGB_Color, nullptr, 1548 },
    { "TerrainObjectsLightingNightLightPos2", &INI::Parse_Coord3D, nullptr, 1560 },
    { "TerrainLightingMorningAmbient3", &INI::Parse_RGB_Color, nullptr, 708 },
    { "TerrainLightingMorningDiffuse3", &INI::Parse_RGB_Color, nullptr, 720 },
    { "TerrainLightingMorningLightPos3", &INI::Parse_Coord3D, nullptr, 732 },
    { "TerrainLightingAfternoonAmbient3", &INI::Parse_RGB_Color, nullptr, 816 },
    { "TerrainLightingAfternoonDiffuse3", &INI::Parse_RGB_Color, nullptr, 828 },
    { "TerrainLightingAfternoonLightPos3", &INI::Parse_Coord3D, nullptr, 840 },
    { "TerrainLightingEveningAmbient3", &INI::Parse_RGB_Color, nullptr, 924 },
    { "TerrainLightingEveningDiffuse3", &INI::Parse_RGB_Color, nullptr, 936 },
    { "TerrainLightingEveningLightPos3", &INI::Parse_Coord3D, nullptr, 948 },
    { "TerrainLightingNightAmbient3", &INI::Parse_RGB_Color, nullptr, 1032 },
    { "TerrainLightingNightDiffuse3", &INI::Parse_RGB_Color, nullptr, 1044 },
    { "TerrainLightingNightLightPos3", &INI::Parse_Coord3D, nullptr, 1056 },
    { "TerrainObjectsLightingMorningAmbient3", &INI::Parse_RGB_Color, nullptr, 1248 },
    { "TerrainObjectsLightingMorningDiffuse3", &INI::Parse_RGB_Color, nullptr, 1260 },
    { "TerrainObjectsLightingMorningLightPos3", &INI::Parse_Coord3D, nullptr, 1272 },
    { "TerrainObjectsLightingAfternoonAmbient3", &INI::Parse_RGB_Color, nullptr, 1356 },
    { "TerrainObjectsLightingAfternoonDiffuse3", &INI::Parse_RGB_Color, nullptr, 1368 },
    { "TerrainObjectsLightingAfternoonLightPos3", &INI::Parse_Coord3D, nullptr, 1380 },
    { "TerrainObjectsLightingEveningAmbient3", &INI::Parse_RGB_Color, nullptr, 1464 },
    { "TerrainObjectsLightingEveningDiffuse3", &INI::Parse_RGB_Color, nullptr, 1476 },
    { "TerrainObjectsLightingEveningLightPos3", &INI::Parse_Coord3D, nullptr, 1488 },
    { "TerrainObjectsLightingNightAmbient3", &INI::Parse_RGB_Color, nullptr, 1572 },
    { "TerrainObjectsLightingNightDiffuse3", &INI::Parse_RGB_Color, nullptr, 1584 },
    { "TerrainObjectsLightingNightLightPos3", &INI::Parse_Coord3D, nullptr, 1596 },
    { "NumberGlobalLights", &INI::Parse_Int, nullptr, 1784 }, //
    { "InfantryLightMorningScale", &INI::Parse_Real, nullptr, 1720 }, //
    { "InfantryLightAfternoonScale", &INI::Parse_Real, nullptr, 1724 }, //
    { "InfantryLightEveningScale", &INI::Parse_Real, nullptr, 1728 }, //
    { "InfantryLightNightScale", &INI::Parse_Real, nullptr, 1732 }, //
    { "MaxTranslucentObjects", &INI::Parse_Int, nullptr, 1764 }, //
    { "OccludedColorLuminanceScale", &INI::Parse_Real, nullptr, 1780 }, //
    { "MaxRoadSegments", &INI::Parse_Int, nullptr, 1788 }, //
    { "MaxRoadVertex", &INI::Parse_Int, nullptr, 1792 }, //
    { "MaxRoadIndex", &INI::Parse_Int, nullptr, 1796 }, //
    { "MaxRoadTypes", &INI::Parse_Int, nullptr, 1800 }, //
    { "ValuePerSupplyBox", &INI::Parse_Int, nullptr, 1952 }, //
    { "AudioOn", &INI::Parse_Bool, nullptr, 1804 }, //
    { "MusicOn", &INI::Parse_Bool, nullptr, 1805 }, //
    { "SoundsOn", &INI::Parse_Bool, nullptr, 1806 }, //
    { "Sounds3DOn", &INI::Parse_Bool, nullptr, 1807 }, //
    { "SpeechOn", &INI::Parse_Bool, nullptr, 1808 }, //
    { "VideoOn", &INI::Parse_Bool, nullptr, 1809 }, //
    { "DisableCameraMovements", &INI::Parse_Bool, nullptr, 1810 }, //
    { "DebugAI", &INI::Parse_Bool, nullptr, 1820 }, //
    { "DebugAIObstacles", &INI::Parse_Bool, nullptr, 1825 }, //
    { "ShowClientPhysics", &INI::Parse_Bool, nullptr, 1812 }, //
    { "ShowTerrainNormals", &INI::Parse_Bool, nullptr, 1813 }, //
    { "ShowObjectHealth", &INI::Parse_Bool, nullptr, 1826 }, //
    { "ParticleScale", &INI::Parse_Real, nullptr, 1840 }, //
    { "AutoFireParticleSmallPrefix", &INI::Parse_AsciiString, nullptr, offsetof(GlobalData, m_autoFireParticleSmallPrefix) },
    { "AutoFireParticleSmallSystem", &INI::Parse_AsciiString, nullptr, offsetof(GlobalData, m_autoFireParticleSmallSystem) },
    { "AutoFireParticleSmallMax", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_autoFireParticleSmallMax) }, //
    { "AutoFireParticleMediumPrefix",
        &INI::Parse_AsciiString,
        nullptr,
        offsetof(GlobalData, m_autoFireParticleMediumPrefix) },
    { "AutoFireParticleMediumSystem",
        &INI::Parse_AsciiString,
        nullptr,
        offsetof(GlobalData, m_autoFireParticleMediumSystem) },
    { "AutoFireParticleMediumMax", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_autoFireParticleMediumMax) }, //
    { "AutoFireParticleLargePrefix", &INI::Parse_AsciiString, nullptr, offsetof(GlobalData, m_autoFireParticleLargePrefix) },
    { "AutoFireParticleLargeSystem", &INI::Parse_AsciiString, nullptr, offsetof(GlobalData, m_autoFireParticleLargeSystem) },
    { "AutoFireParticleLargeMax", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_autoFireParticleLargeMax) }, //
    { "AutoSmokeParticleSmallPrefix",
        &INI::Parse_AsciiString,
        nullptr,
        offsetof(GlobalData, m_autoSmokeParticleSmallPrefix) },
    { "AutoSmokeParticleSmallSystem",
        &INI::Parse_AsciiString,
        nullptr,
        offsetof(GlobalData, m_autoSmokeParticleSmallSystem) },
    { "AutoSmokeParticleSmallMax", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_autoSmokeParticleSmallMax) }, //
    { "AutoSmokeParticleMediumPrefix",
        &INI::Parse_AsciiString,
        nullptr,
        offsetof(GlobalData, m_autoSmokeParticleMediumPrefix) },
    { "AutoSmokeParticleMediumSystem",
        &INI::Parse_AsciiString,
        nullptr,
        offsetof(GlobalData, m_autoSmokeParticleMediumSystem) },
    { "AutoSmokeParticleMediumMax", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_autoSmokeParticleMediumMax) },
    { "AutoSmokeParticleLargePrefix",
        &INI::Parse_AsciiString,
        nullptr,
        offsetof(GlobalData, m_autoSmokeParticleLargePrefix) },
    { "AutoSmokeParticleLargeSystem",
        &INI::Parse_AsciiString,
        nullptr,
        offsetof(GlobalData, m_autoSmokeParticleLargeSystem) },
    { "AutoSmokeParticleLargeMax", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_autoSmokeParticleLargeMax) },
    { "AutoAflameParticlePrefix", &INI::Parse_AsciiString, nullptr, offsetof(GlobalData, m_autoAFlameParticlePrefix) },
    { "AutoAflameParticleSystem", &INI::Parse_AsciiString, nullptr, offsetof(GlobalData, m_autoAFlameParticleSystem) },
    { "AutoAflameParticleMax", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_autoAFlameParticleMax) },
    { "BuildSpeed", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_buildSpeed) }, //
    { "MinDistFromEdgeOfMapForBuild", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_minDistanceFromMapEdgeForBuild) }, //
    { "SupplyBuildBorder", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_supplyBuildOrder) }, //
    { "AllowedHeightVariationForBuilding",
        &INI::Parse_Real,
        nullptr,
        offsetof(GlobalData, m_allowedHeightVariationForBuildings) }, //
    { "MinLowEnergyProductionSpeed", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_minLowEnergyProductionSpeed) }, //
    { "MaxLowEnergyProductionSpeed", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_maxLowEnergyProductionSpeed) }, //
    { "LowEnergyPenaltyModifier", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_lowEnergyPenaltyModifier) }, //
    { "MultipleFactory", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_multipleFactory) }, //
    { "RefundPercent", &INI::Parse_Percent_To_Real, nullptr, offsetof(GlobalData, m_refundPercent) }, //
    { "CommandCenterHealRange", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_commandCenterHealRange) }, //
    { "CommandCenterHealAmount", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_commandCenterHealAmmount) }, //
    { "StandardMinefieldDensity", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_standardMinefieldDensity) }, //
    { "StandardMinefieldDistance", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_standardMinefieldDistance) }, //
    { "MaxLineBuildObjects", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_maxLineBuildObjects) }, //
    { "MaxTunnelCapacity", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_maxTunnelCapacity) }, //
    { "MaxParticleCount", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_maxParticleCount) }, //
    { "MaxFieldParticleCount", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_maxFieldParticleCount) }, //
    { "HorizontalScrollSpeedFactor", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_horizontalScrollSpeedFactor) }, //
    { "VerticalScrollSpeedFactor", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_verticalScrollSpeedFactor) }, //
    { "ScrollAmountCutoff", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_scrollAmountCutoff) }, //
    { "CameraAdjustSpeed", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_cameraAdjustSpeed) }, //
    { "EnforceMaxCameraHeight", &INI::Parse_Bool, nullptr, offsetof(GlobalData, m_enforceMaxCameraHeight) }, //
    { "KeyboardScrollSpeedFactor", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_keyboardScrollFactor) }, //
    { "KeyboardDefaultScrollSpeedFactor",
        &INI::Parse_Real,
        nullptr,
        offsetof(GlobalData, m_keyboardDefaultScrollFactor) }, //
    { "MovementPenaltyDamageState",
        &INI::Parse_Index_List,
        g_bodyDamageNames,
        offsetof(GlobalData, m_movementPenaltyDamageState) }, //
    { "HealthBonus_Veteran", &INI::Parse_Percent_To_Real, nullptr, offsetof(GlobalData, m_veteranHealthBonus) }, //
    { "HealthBonus_Elite", &INI::Parse_Percent_To_Real, nullptr, offsetof(GlobalData, m_eliteHealthBonus) }, //
    { "HealthBonus_Heroic", &INI::Parse_Percent_To_Real, nullptr, offsetof(GlobalData, m_heroicHealthBonus) }, //
    { "HumanSoloPlayerHealthBonus_Easy",
        &INI::Parse_Percent_To_Real,
        nullptr,
        offsetof(GlobalData, m_easySoloHumanHealthBonus) }, //
    { "HumanSoloPlayerHealthBonus_Normal",
        &INI::Parse_Percent_To_Real,
        nullptr,
        offsetof(GlobalData, m_normalSoloHumanHealthBonus) }, //
    { "HumanSoloPlayerHealthBonus_Hard",
        &INI::Parse_Percent_To_Real,
        nullptr,
        offsetof(GlobalData, m_hardSoloHumanHealthBonus) }, //
    { "AISoloPlayerHealthBonus_Easy",
        &INI::Parse_Percent_To_Real,
        nullptr,
        offsetof(GlobalData, m_easySoloAIHealthBonus) }, //
    { "AISoloPlayerHealthBonus_Normal",
        &INI::Parse_Percent_To_Real,
        nullptr,
        offsetof(GlobalData, m_normalSoloAIHealthBonus) }, //
    { "AISoloPlayerHealthBonus_Hard",
        &INI::Parse_Percent_To_Real,
        nullptr,
        offsetof(GlobalData, m_hardSoloAIHealthBonus) }, //
    { "WeaponBonus", &WeaponBonusSet::Parse_Weapon_Bonus_Set_Ptr, nullptr, offsetof(GlobalData, m_weaponBonusSet) }, //
    { "DefaultStructureRubbleHeight", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_defaultStructureRubbleHeight) }, //
    { "FixedSeed", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_fixedSeed) }, //
    { "ShellMapName", &INI::Parse_AsciiString, nullptr, offsetof(GlobalData, m_shellMapName) }, //
    { "ShellMapOn", &INI::Parse_Bool, nullptr, offsetof(GlobalData, m_shellMapOn) }, //
    { "PlayIntro", &INI::Parse_Bool, nullptr, offsetof(GlobalData, m_playIntro) }, //
    { "FirewallBehavior", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_firewallBehaviour) }, //
    { "FirewallPortOverride", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_firewallPortOverrides) }, //
    { "FirewallPortAllocationDelta", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_firewallPortAllocationDelta) }, //
    { "GroupSelectMinSelectSize", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_groupSelectMinSelectSize) }, //
    { "GroupSelectVolumeBase", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_groupSelectVolumeBase) }, //
    { "GroupSelectVolumeIncrement", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_groupSelectVolumeIncrement) }, //
    { "MaxUnitSelectSounds", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_maxUnitSelectSounds) }, //
    { "SelectionFlashSaturationFactor",
        &INI::Parse_Real,
        nullptr,
        offsetof(GlobalData, m_selectionFlashSaturationFactor) }, //
    { "SelectionFlashHouseColor", &INI::Parse_Bool, nullptr, offsetof(GlobalData, m_selectionFlashHouseColor) }, //
    { "CameraAudibleRadius", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_cameraAudibleRadius) }, //
    { "GroupMoveClickToGatherAreaFactor",
        &INI::Parse_Real,
        nullptr,
        offsetof(GlobalData, m_groupMoveClickToGatherAreaFactor) }, //
    { "ShakeSubtleIntensity", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_shakeSubtleIntensity) }, //
    { "ShakeNormalIntensity", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_shakeNormalIntensity) }, //
    { "ShakeStrongIntensity", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_shakeStrongIntensity) }, //
    { "ShakeSevereIntensity", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_shakeSevereIntensity) }, //
    { "ShakeCineExtremeIntensity", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_shakeCineExtremeIntensity) }, //
    { "ShakeCineInsaneIntensity", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_shakeCineInsaneIntensity) }, //
    { "MaxShakeIntensity", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_maxShakeIntensity) }, //
    { "MaxShakeRange", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_maxShakeRange) }, //
    { "SellPercentage", &INI::Parse_Percent_To_Real, nullptr, offsetof(GlobalData, m_sellPercentage) }, //
    { "BaseRegenHealthPercentPerSecond",
        &INI::Parse_Percent_To_Real,
        nullptr,
        offsetof(GlobalData, m_baseRegenHealthPercentPerSecond) }, //
    { "BaseRegenDelay", &INI::Parse_Duration_Unsigned_Int, nullptr, offsetof(GlobalData, m_baseRegenDelay) }, //
    { "SpecialPowerViewObject", &INI::Parse_AsciiString, nullptr, offsetof(GlobalData, m_specialPowerViewObject) }, //
    { "StandardPublicBone", &INI::Parse_AsciiString_Vector_Append, nullptr, offsetof(GlobalData, m_standardPublicBones) }, //
    { "ShowMetrics", &INI::Parse_Bool, nullptr, offsetof(GlobalData, m_showMetrics) }, //
    { "DefaultStartingCash", &Money::Parse_Money_Amount, nullptr, offsetof(GlobalData, m_defaultStartingCash) }, //
    { "ShroudColor", &INI::Parse_RGB_Color, nullptr, offsetof(GlobalData, m_shroudColor) }, //
    { "ClearAlpha", &INI::Parse_Unsigned_Byte, nullptr, offsetof(GlobalData, m_clearAlpha) }, //
    { "FogAlpha", &INI::Parse_Unsigned_Byte, nullptr, offsetof(GlobalData, m_fogAlpha) }, //
    { "ShroudAlpha", &INI::Parse_Unsigned_Byte, nullptr, offsetof(GlobalData, m_shroudAlpha) }, //
    { "HotKeyTextColor", &INI::Parse_Color_Int, nullptr, offsetof(GlobalData, m_hotKeytextColor) }, //
    { "PowerBarBase", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_powerBarBase) }, //
    { "PowerBarIntervals", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_powerBarIntervals) }, //
    { "PowerBarYellowRange", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_powerBarYellowRange) }, //
    { "UnlookPersistDuration",
        &INI::Parse_Duration_Unsigned_Int,
        nullptr,
        offsetof(GlobalData, m_unlookPersistDuration) }, //
    { "NetworkFPSHistoryLength", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_networkFPSHistoryLength) }, //
    { "NetworkLatencyHistoryLength", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_networkLatencyHistoryLength) }, //
    { "NetworkRunAheadMetricsTime", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_networkRunAheadMetricsTime) }, //
    { "NetworkCushionHistoryLength", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_networkCushionHistoryLength) }, //
    { "NetworkRunAheadSlack", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_networkRunAheadSlack) }, //
    { "NetworkKeepAliveDelay", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_networkKeepAliveDelay) }, //
    { "NetworkDisconnectTime", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_networkDisconnecTime) }, //
    { "NetworkPlayerTimeoutTime", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_networkPlayerTimeOut) }, //
    { "NetworkDisconnectScreenNotifyTime",
        &INI::Parse_Int,
        nullptr,
        offsetof(GlobalData, m_networkDisconnectScreenNotifyTime) },
    { "KeyboardCameraRotateSpeed", &INI::Parse_Real, nullptr, offsetof(GlobalData, m_keyboardCameraRotateSpeed) },
    { "PlayStats", &INI::Parse_Int, nullptr, offsetof(GlobalData, m_playerStats) },
    { "UserDataLeafName", &INI::Parse_AsciiString, nullptr, offsetof(GlobalData, m_userDataLeafName) },
    { nullptr, nullptr, nullptr, 0 }
};

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
