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

#include "always.h"
#include "color.h"
#include "gametype.h"
#include "ini.h"
#include "mempoolobj.h"
#include "subsysteminterface.h"

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

protected:
    virtual ~TerrainRoadType() override {}

public:
    TerrainRoadType();

    // TODO more setters and getters as needed.
    Utf8String Get_Name() { return m_name; }
    Utf8String Get_Damaged_OCL(int dmg, int effect) { return m_damagedTransitionOCL[dmg][effect]; }
    Utf8String Get_Damaged_FX(int dmg, int effect) { return m_damagedTransitionFX[dmg][effect]; }
    Utf8String Get_Repaired_OCL(int dmg, int effect) { return m_repairedTransitionOCL[dmg][effect]; }
    Utf8String Get_Repaired_FX(int dmg, int effect) { return m_repairedTransitionFX[dmg][effect]; }

    TerrainRoadType *Get_Next() { return m_next; }
    int Get_ID() { return m_id; }
    Utf8String Get_Texture() { return m_texture; }
    float Get_Road_Width_In_Texture() { return m_roadWidthInTexture; }
    float Get_Road_Width() { return m_roadWidth; }
    bool Is_Bridge() { return m_isBridge; }

    float Get_Bridge_Scale() { return m_bridgeScale; }
    Utf8String Get_Bridge_Model() { return m_bridgeModelName; }
    Utf8String Get_Texture_Damaged() { return m_textureDamaged; }
    Utf8String Get_Bridge_Model_Name_Damaged() { return m_bridgeModelNameDamaged; }
    Utf8String Get_Texture_Really_Damaged() { return m_textureReallyDamaged; }
    Utf8String Get_Bridge_Model_Name_Really_Damaged() { return m_bridgeModelNameReallyDamaged; }
    Utf8String Get_Texture_Broken() { return m_textureBroken; }
    Utf8String Get_Bridge_Model_Name_Broken() { return m_bridgeModelNameBroken; }
    Utf8String Get_Tower_Object_Name(BridgeTowerType type) { return m_towerObjectName[type]; }

    void Set_Name(Utf8String name) { m_name = name; }
    void Set_Damaged_OCL(int dmg, int effect, Utf8String name) { m_damagedTransitionOCL[dmg][effect] = name; }
    void Set_Damaged_FX(int dmg, int effect, Utf8String name) { m_damagedTransitionFX[dmg][effect] = name; }
    void Set_Repaired_OCL(int dmg, int effect, Utf8String name) { m_repairedTransitionOCL[dmg][effect] = name; }
    void Set_Repaired_FX(int dmg, int effect, Utf8String name) { m_repairedTransitionFX[dmg][effect] = name; }

    static void Parse_Transition_To_OCL(INI *ini, void *formal, void *store, void const *user_data);
    static void Parse_Transition_To_FX(INI *ini, void *formal, void *store, void const *user_data);

private:
    Utf8String m_name;
    bool m_isBridge;
    int m_id;
    TerrainRoadType *m_next;
    float m_roadWidth;
    float m_roadWidthInTexture;
    float m_bridgeScale;
    Utf8String m_scaffoldObjectName;
    Utf8String m_scaffoldSupportObjectName;
    RGBColor m_radarColor;
    Utf8String m_bridgeModelName;
    Utf8String m_texture;
    Utf8String m_bridgeModelNameDamaged;
    Utf8String m_textureDamaged;
    Utf8String m_bridgeModelNameReallyDamaged;
    Utf8String m_textureReallyDamaged;
    Utf8String m_bridgeModelNameBroken;
    Utf8String m_textureBroken;
    Utf8String m_towerObjectName[BRIDGE_MAX_TOWERS];
    Utf8String m_damagedToSounds[BODY_COUNT];
    Utf8String m_damagedTransitionOCL[BODY_COUNT][3];
    Utf8String m_damagedTransitionFX[BODY_COUNT][3];
    Utf8String m_repairedToSounds[BODY_COUNT];
    Utf8String m_repairedTransitionOCL[BODY_COUNT][3];
    Utf8String m_repairedTransitionFX[BODY_COUNT][3];
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

    TerrainRoadType *New_Road(Utf8String name);
    TerrainRoadType *New_Bridge(Utf8String name);

    TerrainRoadType *Find_Road(Utf8String name);
    TerrainRoadType *Find_Bridge(Utf8String name);

    static void Parse_Terrain_Road_Definition(INI *ini);
    static void Parse_Terrain_Bridge_Definition(INI *ini);

    TerrainRoadType *First_Road() { return m_roadList; }
    TerrainRoadType *First_Bridge() { return m_bridgeList; }
    TerrainRoadType *Next_Road(TerrainRoadType *road) { return road->Get_Next(); }

private:
    TerrainRoadType *m_roadList;
    TerrainRoadType *m_bridgeList;

    static int s_idCounter;
    static const FieldParse s_terrainRoadFieldParseTable[];
    static const FieldParse s_terrainBridgeFieldParseTable[];
};

#ifdef GAME_DLL
extern TerrainRoadCollection *&g_theTerrainRoads;
#else
extern TerrainRoadCollection *g_theTerrainRoads;
#endif
