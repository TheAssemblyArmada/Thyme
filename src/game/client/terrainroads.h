/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Classes for handling road and bridge properties.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "color.h"
#include "gametype.h"
#include "ini.h"
#include "mempoolobj.h"
#include "subsysteminterface.h"

#ifndef THYME_STANDALONE
#include "hooker.h"
#endif

enum BridgeTowerType
{
    BRIDGE_TOWER_FROM_LEFT,
    BRIDGE_TOWER_FROM_RIGHT,
    BRIDGE_TOWER_TO_LEFT,
    BRIDGE_TOWER_TO_RIGHT,
    BRIDGE_MAX_TOWERS,
};

class TerrainRoadType : public MemoryPoolObject
{
    friend class TerrainRoadCollection;

    IMPLEMENT_POOL(TerrainRoadType);

public:
    TerrainRoadType();
    virtual ~TerrainRoadType() {}

    // TODO more setters and getters as needed.
    AsciiString Get_Name() { return m_name; }
    AsciiString Get_Damaged_OCL(int dmg, int effect) { return m_damagedTransitionOCL[dmg][effect]; }
    AsciiString Get_Damaged_FX(int dmg, int effect) { return m_damagedTransitionFX[dmg][effect]; }
    AsciiString Get_Repaired_OCL(int dmg, int effect) { return m_repairedTransitionOCL[dmg][effect]; }
    AsciiString Get_Repaired_FX(int dmg, int effect) { return m_repairedTransitionFX[dmg][effect]; }

    void Set_Name(AsciiString name) { m_name = name; }
    void Set_Damaged_OCL(int dmg, int effect, AsciiString name) { m_damagedTransitionOCL[dmg][effect] = name; }
    void Set_Damaged_FX(int dmg, int effect, AsciiString name) { m_damagedTransitionFX[dmg][effect] = name; }
    void Set_Repaired_OCL(int dmg, int effect, AsciiString name) { m_repairedTransitionOCL[dmg][effect] = name; }
    void Set_Repaired_FX(int dmg, int effect, AsciiString name) { m_repairedTransitionFX[dmg][effect] = name; }

    static void Parse_Transition_To_OCL(INI *ini, void *formal, void *store, void const *user_data);
    static void Parse_Transition_To_FX(INI *ini, void *formal, void *store, void const *user_data);

private:
    AsciiString m_name;
    bool m_isBridge;
    int m_id;
    TerrainRoadType *m_next;
    float m_roadWidth;
    float m_roadWidthInTexture;
    float m_bridgeScale;
    AsciiString m_scaffoldObjectName;
    AsciiString m_scaffoldSupportObjectName;
    RGBColor m_radarColor;
    AsciiString m_bridgeModelName;
    AsciiString m_texture;
    AsciiString m_bridgeModelNameDamaged;
    AsciiString m_textureDamaged;
    AsciiString m_bridgeModelNameReallyDamaged;
    AsciiString m_textureReallyDamaged;
    AsciiString m_bridgeModelNameBroken;
    AsciiString m_textureBroken;
    AsciiString m_towerObjectName[BRIDGE_MAX_TOWERS];
    AsciiString m_damagedToSounds[BODY_COUNT];
    AsciiString m_damagedTransitionOCL[BODY_COUNT][3];
    AsciiString m_damagedTransitionFX[BODY_COUNT][3];
    AsciiString m_repairedToSounds[BODY_COUNT];
    AsciiString m_repairedTransitionOCL[BODY_COUNT][3];
    AsciiString m_repairedTransitionFX[BODY_COUNT][3];
    float m_transitionEffectsHeight;
    int m_numFXPerType;
};

class TerrainRoadCollection : public SubsystemInterface
{
public:
    TerrainRoadCollection() : m_roadList(nullptr), m_bridgeList(nullptr) { s_idCounter = 1; }
    virtual ~TerrainRoadCollection();

    virtual void Init() override {}
    virtual void Reset() override {}
    virtual void Update() override {}

    TerrainRoadType *New_Road(AsciiString name);
    TerrainRoadType *New_Bridge(AsciiString name);

    TerrainRoadType *Find_Road(AsciiString name);
    TerrainRoadType *Find_Bridge(AsciiString name);

    static void Parse_Terrain_Road_Definitions(INI *ini);
    static void Parse_Terrain_Bridge_Definitions(INI *ini);

private:
    TerrainRoadType *m_roadList;
    TerrainRoadType *m_bridgeList;

    static int s_idCounter;
    static FieldParse s_terrainRoadFieldParseTable[];
    static FieldParse s_terrainBridgeFieldParseTable[];
};

#ifndef THYME_STANDALONE
extern TerrainRoadCollection *&g_theTerrainRoads;
#else
extern TerrainRoadCollection *g_theTerrainRoads;
#endif