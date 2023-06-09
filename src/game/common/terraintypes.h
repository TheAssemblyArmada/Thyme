/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Classes for handling terrain textures and properties.
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
#include "ini.h"
#include "mempoolobj.h"
#include "subsysteminterface.h"

enum TerrainClass : int32_t
{
    TERRAIN_NONE,
    TERRAIN_DESERT_1,
    TERRAIN_DESERT_2,
    TERRAIN_DESERT_3,
    TERRAIN_EASTERN_EUROPE_1,
    TERRAIN_EASTERN_EUROPE_2,
    TERRAIN_EASTERN_EUROPE_3,
    TERRAIN_SWISS_1,
    TERRAIN_SWISS_2,
    TERRAIN_SWISS_3,
    TERRAIN_SNOW_1,
    TERRAIN_SNOW_2,
    TERRAIN_SNOW_3,
    TERRAIN_DIRT,
    TERRAIN_GRASS,
    TERRAIN_TRANSITION,
    TERRAIN_ROCK,
    TERRAIN_SAND,
    TERRAIN_CLIFF,
    TERRAIN_WOOD,
    TERRAIN_BLEND_EDGE,
    TERRAIN_DESERT_LIVE,
    TERRAIN_DESERT_DRY,
    TERRAIN_SAND_ACCENT,
    TERRAIN_BEACH_TROPICAL,
    TERRAIN_BEACH_PARK,
    TERRAIN_MOUNTAIN_RUGGED,
    TERRAIN_GRASS_COBBLESTONE,
    TERRAIN_GRASS_ACCENT,
    TERRAIN_RESIDENTIAL,
    TERRAIN_SNOW_RUGGED,
    TERRAIN_SNOW_FLAT,
    TERRAIN_FIELD,
    TERRAIN_ASPHALT,
    TERRAIN_CONCRETE,
    TERRAIN_CHINA,
    TERRAIN_ROCK_ACCENT,
    TERRAIN_URBAN,
    TERRAIN_COUNT,
};

class TerrainType : public MemoryPoolObject
{
    IMPLEMENT_POOL(TerrainType);
    friend class TerrainTypeCollection;

protected:
    virtual ~TerrainType() override {}

public:
    TerrainType();
    Utf8String Get_Texture() { return m_texture; }
    Utf8String Get_Name() { return m_name; }
    bool Is_Blend_Edge() { return m_blendEdgeTexture; }
    TerrainClass Get_Class() { return m_class; }
    TerrainType *Friend_Get_Next() { return m_next; }

private:
    Utf8String m_name;
    Utf8String m_texture;
    bool m_blendEdgeTexture;
    TerrainClass m_class;
    bool m_restrictConstruction;
    TerrainType *m_next;
};

class TerrainTypeCollection : public SubsystemInterface
{
public:
    TerrainTypeCollection() : m_terrainList(nullptr) {}
    virtual ~TerrainTypeCollection();

    // SubsystemInterface implementations
    virtual void Init() override{};
    virtual void Reset() override{};
    virtual void Update() override{};

    TerrainType *Find_Terrain(Utf8String name);
    TerrainType *New_Terrain(Utf8String name);
    TerrainType *First_Terrain() { return m_terrainList; }
    TerrainType *Next_Terrain(TerrainType *terrain) { return terrain->Friend_Get_Next(); }

    static void Parse_Terrain_Definition(INI *ini);

private:
    TerrainType *m_terrainList;

    static const FieldParse s_terrainTypeParseTable[];
};

#ifdef GAME_DLL
extern TerrainTypeCollection *&g_theTerrainTypes;
#else
extern TerrainTypeCollection *g_theTerrainTypes;
#endif
