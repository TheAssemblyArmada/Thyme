////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: TERRAINTYPES.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Classes for handling terrain textures and properties.
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

#ifndef TERRAINTYPES_H
#define TERRAINTYPES_H

#ifndef THYME_STANDALONE
#include "hooker.h"
#endif

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
public:
    TerrainType();
    virtual ~TerrainType() {}

private:
    AsciiString m_name;
    AsciiString m_texture;
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
    virtual void Init() override {};
    virtual void Reset() override {};
    virtual void Update() override {};

    TerrainType *Find_Terrain(AsciiString name);
    TerrainType *New_Terrain(AsciiString name);

    static void Parse_Terrain_Definition(INI *ini);
private:
    TerrainType *m_terrainList;

    static FieldParse s_terrainTypeParseTable[];
};

#ifdef THYME_STANDALONE
extern TerrainTypeCollection *g_theTerrainTypes;
#else
#define g_theTerrainTypes (Make_Global<TerrainTypeCollection*>(0x00A2BE54))
#endif

#endif // TERRAINTYPES_H
