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
#include "terraintypes.h"
#include <cstddef>

#ifndef GAME_DLL
TerrainTypeCollection *g_theTerrainTypes = nullptr;
#endif

namespace
{

const char *s_terrainTypeNames[] = { "NONE",
    "DESERT_1",
    "DESERT_2",
    "DESERT_3",
    "EASTERN_EUROPE_1",
    "EASTERN_EUROPE_2",
    "EASTERN_EUROPE_3",
    "SWISS_1",
    "SWISS_2",
    "SWISS_3",
    "SNOW_1",
    "SNOW_2",
    "SNOW_3",
    "DIRT",
    "GRASS",
    "TRANSITION",
    "ROCK",
    "SAND",
    "CLIFF",
    "WOOD",
    "BLEND_EDGE",
    "DESERT_LIVE",
    "DESERT_DRY",
    "SAND_ACCENT",
    "BEACH_TROPICAL",
    "BEACH_PARK",
    "MOUNTAIN_RUGGED",
    "GRASS_COBBLESTONE",
    "GRASS_ACCENT",
    "RESIDENTIAL",
    "SNOW_RUGGED",
    "SNOW_FLAT",
    "FIELD",
    "ASPHALT",
    "CONCRETE",
    "CHINA",
    "ROCK_ACCENT",
    "URBAN",
    nullptr };

} // namespace

const FieldParse TerrainTypeCollection::s_terrainTypeParseTable[] = {
    { "Texture", &INI::Parse_AsciiString, nullptr, offsetof(TerrainType, m_texture) },
    { "BlendEdges", &INI::Parse_Bool, nullptr, offsetof(TerrainType, m_blendEdgeTexture) },
    { "Class", &INI::Parse_Index_List, s_terrainTypeNames, offsetof(TerrainType, m_class) },
    { "RestrictConstruction", &INI::Parse_Bool, nullptr, offsetof(TerrainType, m_restrictConstruction) },
    { nullptr, nullptr, nullptr, 0 }
};

TerrainType::TerrainType() :
    m_name(), m_texture(), m_blendEdgeTexture(false), m_class(TERRAIN_NONE), m_restrictConstruction(false), m_next(nullptr)
{
}

TerrainTypeCollection::~TerrainTypeCollection()
{
    while (m_terrainList != nullptr) {
        TerrainType *next = m_terrainList->m_next;
        m_terrainList->Delete_Instance();
        m_terrainList = next;
    }
}

TerrainType *TerrainTypeCollection::Find_Terrain(Utf8String name)
{
    TerrainType *retval = m_terrainList;

    // Find a TerrainType with a matching name and return it.
    while (retval != nullptr) {
        if (strcmp(retval->m_name.Str(), name.Str()) == 0) {
            break;
        }

        retval = retval->m_next;
    }

    return retval;
}

TerrainType *TerrainTypeCollection::New_Terrain(Utf8String name)
{
    TerrainType *retval = NEW_POOL_OBJ(TerrainType);
    TerrainType *def = Find_Terrain("DefaultTerrain");

    if (def != nullptr) {
        *retval = *def;
    }

    retval->m_name = name;
    retval->m_next = m_terrainList;
    m_terrainList = retval;

    return retval;
}

// Was originally INI::parseTerrainDefinition
void TerrainTypeCollection::Parse_Terrain_Definition(INI *ini)
{
    Utf8String token = ini->Get_Next_Token();
    TerrainType *tt = g_theTerrainTypes->Find_Terrain(token);

    if (tt == nullptr) {
        tt = g_theTerrainTypes->New_Terrain(token);
    }

    ini->Init_From_INI(tt, s_terrainTypeParseTable);
}
