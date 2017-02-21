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
#ifdef _MSC_VER
#pragma once
#endif // _MSC_VER

#ifndef _GLOBALDATA_H_
#define _GLOBALDATA_H_

#include "subsysteminterface.h"
#include "asciistring.h"
#include "color.h"
#include "coord.h"

class INI;

// These two enums need moving when we work out where they should go.
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
    WEATHER_SNOWY = 1,
    WEATEHR_COUNT,
};

class GlobalData : public SubsystemInterface
{
    public:
        struct TerrainLighting
        {
            RGBColor Ambient;
            RGBColor Diffuse;
            Coord3D LightPos;
        };

        GlobalData();
        virtual ~GlobalData();

        // SubsystemInterface implementation
        virtual void Init() {}
        virtual void Reset();
        virtual void Update() {}

        static void Parse_Game_Data_Definitions(INI *ini);
    
    // Looks like members are likely public or there would have been a lot of
    // getters/setters.
    // pad indicates where padding will be added to keep 4 byte alignment
    // useful if we want to cram any extra variables in without breaking ABI
    public:
        AsciiString MapName;
        AsciiString MoveHintName;
        bool UseTrees;
        bool UnkBool1;
        bool UnkBool2;
        bool unkBool3;
        bool UseFPSLimit;
        bool DumpAssetUsage;
        //char pad[2]
        int32_t FramesPerSecondLimit;
        int32_t ChipsetType;
        bool Windowed;
        //char pad[3]
        int32_t XResolution;
        int32_t YResolution;
        int32_t MaxShellScreens;
        bool UseCloudMap;
        //char pad[3]
        int32_t Use3WayTerrainBlends;   //Should be bool? If so, fix when all GlobalData reading code implemented.
        bool UseLightMap;
        bool BilinearTerrainTexture;
        bool TrilinearTerrainTexture;
        bool MulitPassTerrain;
        bool AdjustCliffTextures;
        bool StretchTerrain;
        bool UseHalfHeightMap;
        bool DrawEntireTerrain;
        int32_t TerrainLOD;             //TODO Actually an enum TerrainLOD
        bool UnkBool4;
        bool UnkBool5;
        //char pad[2]
        int32_t TerrainLODTargetTimeMS;
        bool AlternateMouseEnabled;
        bool RetaliationModeEnabled;
        bool DoubleClickAttackMovesEnabled;
        bool RightMouseAlwaysScrolls;
        bool UseWaterPlane;
        bool UseCloudPlane;
        bool ShadowVolumes;
        bool ShadowDecals;
        int32_t TextureReductionFactor;
        bool UseBehindBuildingMarker;
        //char pad[3]
        float WaterPositionX;
        float WaterPositionY;
        float WaterPositionZ;
        float WaterExtentX;
        float WaterExtentY;
        int32_t WaterType;
        bool ShowSoftWaterEdge;
        bool UnkBool6;
        bool UnkBool7;
        //char pad[1]
        float FeatherWater;
        AsciiString VertexWaterAvailableMaps[4];
        float VertexWaterHeightClampLow[4];
        float VertexWaterHeightClampLHigh[4];
        float VertexWaterAngle[4];
        float VertexWaterXPos[4];
        float VertexWaterYPos[4];
        float VertexWaterZPos[4];
        int32_t VertexWaterXGridCells[4];
        int32_t VertexWaterYGridCells[4];
        float VertexWaterGridSize[4];
        float VertexWaterAttenuationA[4];
        float VertexWaterAttenuationB[4];
        float VertexWaterAttenuationC[4];
        float VertexWaterAttenuationRange[4];
        float UnkFloat1;
        float SkyBoxPositionZ;
        bool DrawSkyBox;    // Init code suggests this might be an int, old BOOL typedef?
        //char pad[3]
        float SkyBoxScale;
        float CameraPitch;
        float CameraYaw;
        float CameraHeight;
        float MaxCameraHeight;
        float MinCameraHeight;
        float TerrainHeightAtMapEdge;
        float UnitDamageThreshold;
        float GroundStiffness;
        float StructureStiffness;
        float Gravity;
        float StealthFriendlyOpacity;
        uint32_t DefaultOcclusionDelay;
        bool UnkBool12;
        bool UnkBool13;
        bool UnkBool14;
        //char pad[1]
        float PartitionCellSize;
        Coord3D AmmoPipWorldOffset;
        Coord3D ContainerPipWorldOffset;
        Coord2D AmmoPipScreenOffset;
        Coord2D ContainerPipScreenOffset;
        float AmmoPipScaleFactor;
        float ContainerPipScaleFactor;
        uint32_t HistoricDamageLimit;
        int32_t MaxTerrainTracks;
        int32_t UnkInt2;
        int32_t UnkInt3;
        int32_t UnkInt4;
        AsciiString LevelGainAnimName;
        float LevelGainAnimTime;
        float LevelGainAnimZRise;
        AsciiString GetHealedAnimName;
        float GetHealedAnimTime;
        float GetHealedAnimZRise;
        TimeOfDayType TimeOfDay;
        WeatherType Weather;
        bool MakeTrackMarks;
        bool HideGarrisonFlags;
        bool ForceModelsFollowTimeOfDay;
        bool ForceModelsFollowWeather;
        TerrainLighting TerrainPlaneLighting[TIME_OF_DAY_COUNT][3];
        TerrainLighting TerrainObjectLighting[TIME_OF_DAY_COUNT][3];
        TerrainLighting UnkTerrainLighting[3];
        float UnkFloat2;
        float InfantryLightMorningScale;
        float InfantryLightAfternoonScale;
        float InfantryLightEveningScale;
        float InfantryLightNightScale;
        float UnkFloat3;
        float EasySoloHumanHealthBonus;
        float NormalSoloHumanHealthBonus;
        float HardSoloHumanHealthBonus;
        float EasySoloAIHealthBonus;
        float NormalSoloAIHealthBonus;
        float HardSoloAIHealthBonus;
        int32_t UnkInt5;    // These 3 are probably some kind of max and get set to 512
        int32_t UnkInt6;
        int32_t UnkInt7;
        float OccludedColorLuminanceScale;
        int32_t NumberGlobalLights;
        int32_t MaxRoadSegments;
        int32_t MaxRoadVertex;
        int32_t MaxRoadIndex;
        int32_t MaxRoadTypes;
        bool AudioOn;
        bool MusicOn;
        bool SoundsOn;
        bool Sounds3DOn;
        bool SpeechOn;
        bool VideoOn;
        bool DisableCameraMovements;
        bool FogOfWarOn;    // Not 100% sure about this, needs confirming
        bool ShowClientPhysics;
        bool ShowTerrainNormals;
        //char pad[2]
        float UnkFloat4;
        int32_t DebugAI;    // Possibly old BOOL typedef for int?, keep int for ABI compat
        bool UnkBool8;
        bool DebugObstacleAI;
        bool ShowObjectHealth;
        bool ScriptDebug;   // Requires DebugWindow.dll to do anything
        bool ParticleEdit;  // Requires ParticleEditor.dll to do anything
        bool DisplayDebug;  // Not 100% sure and needs confirming
        bool WinCursors;
        bool UnkBool9;
        bool UnkBool10;
        bool UnkBool11;
        //char pad[2]
        int32_t FixedSeed;
        float ParticleScale;
        AsciiString AutoFireParticleSmallPrefix;
        AsciiString AutoFireParticleSmallSystem;
        int32_t AutoFireParticleSmallMax;
        AsciiString AutoFireParticleMediumPrefix;
        AsciiString AutoFireParticleMediumSystem;
        int32_t AutoFireParticleMediumMax;
        AsciiString AutoFireParticleLargePrefix;
        AsciiString AutoFireParticleLargeSystem;
        int32_t AutoFireParticleLargeMax;
        AsciiString AutoSmokeParticleSmallPrefix;
        AsciiString AutoSmokeParticleSmallSystem;
        int32_t AutoSmokeParticleSmallMax;
        AsciiString AutoSmokeParticleMediumPrefix;
        AsciiString AutoSmokeParticleMediumSystem;
        int32_t AutoSmokeParticleMediumMax;
        AsciiString AutoSmokeParticleLargePrefix;
        AsciiString AutoSmokeParticleLargeSystem;
        int32_t AutoSmokeParticleLargeMax;
        AsciiString AutoAFlameParticlePrefix;
        AsciiString AutoAFlameParticleSystem;
        int32_t AutoAFlameParticleMax;
        int32_t NetMinPlayers;  // Not 100% sure, needs confirming
        int32_t LanIPAddress;
        int32_t FirewallBehaviour;
        bool SendDelay;
        //char pad[3]
        int32_t FirewallPortOverrides;
        int16_t FirewallPortAllocationDelta;
        //char pad[2]
        int32_t ValuesPerSupplyBox;
        float BuildSpeed;
        float MinDistanceFromMapEdgeForBuild;
        int32_t SupplyBuildOrder;
        float AllowedHeightVariationForBuildings;
        float MinLowEnergyProductionSpeed;
        float MaxLowEnergyProductionSpeed;
        //TODO
        GlobalData *Next;
};

#endif
