/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "worldheightmap.h"
#include "file.h"
#include "filesystem.h"
#include "globaldata.h"
#include "inputstream.h"
#include "mapobject.h"
#include "polygontrigger.h"
#include "sideslist.h"
#include "staticnamekey.h"
#include "targa.h"
#include "terraintex.h"
#include "terraintypes.h"
#include "thingfactory.h"
#include "w3dshadow.h"
#include <stdio.h>

const float HEIGHTMAP_SCALE = 0.625f;

#ifndef GAME_DLL
TileData *WorldHeightMap::s_alphaTiles[12];
#endif

static unsigned char s_buffer[16384];
static unsigned char s_blendBuffer[16384];

class GDIFileStream : public InputStream
{
public:
    GDIFileStream(File *file) : m_file(file) {}
    int Read(void *dst, int size) override { return m_file->Read(dst, size); }

private:
    File *m_file;
};

WorldHeightMap::WorldHeightMap() :
    m_width(0),
    m_height(0),
    m_dataSize(0),
    m_data(nullptr),
    m_seismicUpdateFlag(nullptr),
    m_seismicZVelocities(nullptr),
    m_cellFlipState(nullptr),
    m_cliffState(nullptr),
    m_tileNdxes(nullptr),
    m_blendTileNdxes(nullptr),
    m_cliffInfoNdxes(nullptr),
    m_extraBlendTileNdxes(nullptr),
    m_numBitmapTiles(0),
    m_numBlendedTiles(1),
    m_numCliffInfo(1),
    m_numTextureClasses(0),
    m_terrainTex(nullptr),
    m_terrainTexHeight(1),
    m_alphaTerrainTex(nullptr),
    m_alphaTexHeight(1),
    m_drawOriginX(0),
    m_drawOriginY(0),
    m_drawWidthX(129),
    m_drawHeightY(129)
{
    for (int i = 0; i < TILE_COUNT; i++) {
        m_sourceTiles[i] = nullptr;
        m_edgeTiles[i] = nullptr;
    }

    g_theSidesList->Validate_Sides();
    Setup_Alpha_Tiles();
}

WorldHeightMap::WorldHeightMap(ChunkInputStream *strm, bool logical_data_only) :
    m_width(0),
    m_height(0),
    m_dataSize(0),
    m_data(nullptr),
    m_seismicUpdateFlag(nullptr),
    m_seismicZVelocities(nullptr),
    m_cellFlipState(nullptr),
    m_cliffState(nullptr),
    m_tileNdxes(nullptr),
    m_blendTileNdxes(nullptr),
    m_cliffInfoNdxes(nullptr),
    m_extraBlendTileNdxes(nullptr),
    m_numBitmapTiles(0),
    m_numBlendedTiles(1),
    m_numCliffInfo(1),
    m_numTextureClasses(0),
    m_terrainTex(nullptr),
    m_terrainTexHeight(1),
    m_alphaTerrainTex(nullptr),
    m_alphaTexHeight(1),
    m_drawOriginX(0),
    m_drawOriginY(0),
    m_drawWidthX(129),
    m_drawHeightY(129)
{
    for (int i = 0; i < TILE_COUNT; i++) {
        m_sourceTiles[i] = nullptr;
        m_edgeTiles[i] = nullptr;
    }

    if (g_theWriteableGlobalData) {
        if (g_theWriteableGlobalData->m_stretchTerrain) {
            m_drawWidthX = 65;
            m_drawHeightY = 65;
        }
    }

    DataChunkInput file(strm);

    if (logical_data_only) {
        file.Register_Parser("HeightMapData", "", Parse_Size_Only_In_Chunk, nullptr);
        file.Register_Parser("WorldInfo", "", Parse_World_Dict_Data_Chunk, nullptr);
        file.Register_Parser("ObjectsList", "", Parse_Objects_Data_Chunk, nullptr);

        Free_List_Of_Map_Objects();

        file.Register_Parser("PolygonTriggers", "", PolygonTrigger::Parse_Polygon_Triggers_Data_Chunk, nullptr);
        file.Register_Parser("SidesList", "", SidesList::Parse_Sides_Data_Chunk, nullptr);
    } else {
        file.Register_Parser("HeightMapData", "", Parse_Height_Map_Data_Chunk, nullptr);
        file.Register_Parser("BlendTileData", "", Parse_Blend_Tile_Data_Chunk, nullptr);
        file.Register_Parser("GlobalLighting", "", Parse_Lighting_Data_Chunk, nullptr);
    }

    if (!file.Parse(this)) {
        throw CODE_05;
    }

    if (!logical_data_only) {
        for (int i = 0; i < m_dataSize; i++) {
            if (m_cliffInfoNdxes[i] < 0 || m_cliffInfoNdxes[i] >= m_numCliffInfo) {
                m_cliffInfoNdxes[i] = 0;
            }

            if (m_blendTileNdxes[i] < 0 || m_blendTileNdxes[i] >= m_numBlendedTiles) {
                m_blendTileNdxes[i] = 0;
            }

            if (m_extraBlendTileNdxes[i] < 0 || m_extraBlendTileNdxes[i] >= m_numBlendedTiles) {
                m_extraBlendTileNdxes[i] = 0;
            }
        }
    }

    if (g_theWriteableGlobalData) {
        if (g_theWriteableGlobalData->m_drawEntireTerrain) {
            m_drawWidthX = m_width;
            m_drawHeightY = m_height;
        }
    }

    if (m_drawWidthX > m_width) {
        m_drawWidthX = m_width;
    }

    if (m_drawHeightY > m_height) {
        m_drawHeightY = m_height;
    }

    g_theSidesList->Validate_Sides();
    Setup_Alpha_Tiles();
}

WorldHeightMap::~WorldHeightMap()
{
    if (m_data) {
        delete[] m_data;
        m_data = nullptr;
    }

    if (m_tileNdxes) {
        delete[] m_tileNdxes;
        m_tileNdxes = nullptr;
    }

    if (m_blendTileNdxes) {
        delete[] m_blendTileNdxes;
        m_blendTileNdxes = nullptr;
    }

    if (m_extraBlendTileNdxes) {
        delete[] m_extraBlendTileNdxes;
        m_extraBlendTileNdxes = nullptr;
    }

    if (m_cliffInfoNdxes) {
        delete[] m_cliffInfoNdxes;
        m_cliffInfoNdxes = nullptr;
    }

    if (m_cellFlipState) {
        delete[] m_cellFlipState;
        m_cellFlipState = nullptr;
    }

    if (m_seismicUpdateFlag) {
        delete[] m_seismicUpdateFlag;
        m_seismicUpdateFlag = nullptr;
    }

    if (m_seismicZVelocities) {
        delete[] m_seismicZVelocities;
        m_seismicZVelocities = nullptr;
    }

    if (m_cliffState) {
        delete[] m_cliffState;
        m_cliffState = nullptr;
    }

    for (int i = 0; i < TILE_COUNT; i++) {
        Ref_Ptr_Release(m_sourceTiles[i]);
        Ref_Ptr_Release(m_edgeTiles[i]);
    }

    for (int i = 0; i < 12; i++) {
        Ref_Ptr_Release(s_alphaTiles[i]);
    }

    Ref_Ptr_Release(m_terrainTex);
    Ref_Ptr_Release(m_alphaTerrainTex);
}

void WorldHeightMap::Free_List_Of_Map_Objects()
{
    if (MapObject::s_theMapObjectListPtr) {
        MapObject::s_theMapObjectListPtr->Delete_Instance();
        MapObject::s_theMapObjectListPtr = nullptr;
    }

    MapObject::Get_World_Dict()->Clear();
}

bool WorldHeightMap::Get_Flip_State(int x, int y) const
{
    if (x < 0 || y < 0) {
        return false;
    }

    if (y >= m_height) {
        return false;
    }

    if (x >= m_width) {
        return false;
    }

    if (m_cellFlipState) {
        return ((1 << (x & 7)) & m_cellFlipState[(x / 8) + m_flipStateWidth * y]) != 0;
    }

    return false;
}

void WorldHeightMap::Set_Flip_State(int x_index, int y_index, bool state)
{
    if (x_index >= 0 && y_index >= 0 && y_index < m_height && x_index < m_width) {
        if (m_cellFlipState) {
            unsigned char *c = &m_cellFlipState[(x_index / 8) + m_flipStateWidth * y_index];
            if (state) {
                *c |= 1 << (x_index & 7);
            } else {
                *c &= ~(unsigned char)(1 << (x_index & 7));
            }
        }
    }
}

void WorldHeightMap::Clear_Flip_States()
{
    if (m_cellFlipState) {
        memset(m_cellFlipState, 0, m_height * m_flipStateWidth);
    }
}

float WorldHeightMap::Get_Bilinear_Sample_Seismic_Z_Velocity(int x, int y)
{
    if (x < 0 || y < 0) {
        return 0.0f;
    }

    if (y >= m_height) {
        return 0.0f;
    }

    if (x >= m_width) {
        return 0.0f;
    }

    if (!m_seismicZVelocities) {
        return 0.0f;
    }

    float f1 = m_seismicZVelocities[x + m_width * y] + 0.0f;
    float f2 = 0.0f + 1.0f;

    if (y > 0) {
        f1 = f1 + m_seismicZVelocities[x + m_width * (y - 1)];
        f2 = f2 + 1.0f;

        if (x > 0) {
            f1 = f1 + m_seismicZVelocities[m_width * (y - 1) - 1 + x];
            f2 = f2 + 1.0f;
        }

        if (x < m_width - 1) {
            f1 = f1 + m_seismicZVelocities[m_width * (y - 1) + 1 + x];
            f2 = f2 + 1.0f;
        }
    }

    if (y < m_height - 1) {
        f1 = f1 + m_seismicZVelocities[x + m_width * (y + 1)];
        f2 = f2 + 1.0f;

        if (x > 0) {
            f1 = f1 + m_seismicZVelocities[m_width * (y + 1) - 1 + x];
            f2 = f2 + 1.0f;
        }

        if (x < m_width - 1) {
            f1 = f1 + m_seismicZVelocities[m_width * (y + 1) + 1 + x];
            f2 = f2 + 1.0f;
        }
    }

    if (x > 0) {
        f1 = f1 + m_seismicZVelocities[m_width * y - 1 + x];
        f2 = f2 + 1.0f;
    }

    if (x < m_width - 1) {
        f1 = f1 + m_seismicZVelocities[m_width * y + 1 + x];
        f2 = f2 + 1.0f;
    }

    return f1 / f2;
}

float WorldHeightMap::Get_Seismic_Z_Velocity(int x, int y) const
{
    if (x < 0 || y < 0) {
        return 0.0f;
    }

    if (y >= m_height) {
        return 0.0f;
    }

    if (x >= m_width) {
        return 0.0f;
    }

    if (m_seismicZVelocities) {
        return m_seismicZVelocities[x + m_width * y];
    }

    return 0.0f;
}

void WorldHeightMap::Set_Seismic_Z_Velocity(int x, int y, float velocity)
{
    if (x >= 0 && y >= 0 && y < m_height && x < m_width) {
        if (m_seismicZVelocities) {
            m_seismicZVelocities[x + m_width * y] = velocity;
        }
    }
}

bool WorldHeightMap::Get_Cliff_State(int x_index, int y_index) const
{
    if (x_index < 0 || y_index < 0) {
        return false;
    }

    if (y_index >= m_height) {
        return false;
    }

    if (x_index >= m_width) {
        return false;
    }

    if (m_cliffState) {
        return ((1 << (x_index & 7)) & m_cliffState[(x_index / 8) + m_flipStateWidth * y_index]) != 0;
    }

    return false;
}

bool WorldHeightMap::Parse_World_Dict_Data_Chunk(DataChunkInput &file, DataChunkInfo *info, void *user_data)
{
    Dict d = file.Read_Dict();
    *MapObject::Get_World_Dict() = d;
    bool b;
    WeatherType w = (WeatherType)MapObject::Get_World_Dict()->Get_Int(g_weatherKey.Key(), &b);

    if (b) {
        g_theWriteableGlobalData->m_weather = w;
    }

    return true;
}

bool WorldHeightMap::Parse_Lighting_Data_Chunk(DataChunkInput &file, DataChunkInfo *info, void *user_data)
{
    g_theWriteableGlobalData->m_timeOfDay = (TimeOfDayType)file.Read_Int32();

    GlobalData::TerrainLighting initLightValues;
    memset(&initLightValues, 0, sizeof(initLightValues));
    initLightValues.lightPos.z = -1.0f;

    for (int i = 0; i < TIME_OF_DAY_COUNT - 1; i++) {
        for (int j = 0; j < 3; j++) {
            g_theWriteableGlobalData->m_terrainObjectLighting[i + 1][j] = initLightValues;
            g_theWriteableGlobalData->m_terrainPlaneLighting[i + 1][j] = initLightValues;
        }
    }

    for (int i = 0; i < TIME_OF_DAY_COUNT - 1; i++) {
        g_theWriteableGlobalData->m_terrainPlaneLighting[i + 1][0].ambient.red = file.Read_Real32();
        g_theWriteableGlobalData->m_terrainPlaneLighting[i + 1][0].ambient.green = file.Read_Real32();
        g_theWriteableGlobalData->m_terrainPlaneLighting[i + 1][0].ambient.blue = file.Read_Real32();
        g_theWriteableGlobalData->m_terrainPlaneLighting[i + 1][0].diffuse.red = file.Read_Real32();
        g_theWriteableGlobalData->m_terrainPlaneLighting[i + 1][0].diffuse.green = file.Read_Real32();
        g_theWriteableGlobalData->m_terrainPlaneLighting[i + 1][0].diffuse.blue = file.Read_Real32();
        g_theWriteableGlobalData->m_terrainPlaneLighting[i + 1][0].lightPos.x = file.Read_Real32();
        g_theWriteableGlobalData->m_terrainPlaneLighting[i + 1][0].lightPos.y = file.Read_Real32();
        g_theWriteableGlobalData->m_terrainPlaneLighting[i + 1][0].lightPos.z = file.Read_Real32();

        g_theWriteableGlobalData->m_terrainObjectLighting[i + 1][0].ambient.red = file.Read_Real32();
        g_theWriteableGlobalData->m_terrainObjectLighting[i + 1][0].ambient.green = file.Read_Real32();
        g_theWriteableGlobalData->m_terrainObjectLighting[i + 1][0].ambient.blue = file.Read_Real32();
        g_theWriteableGlobalData->m_terrainObjectLighting[i + 1][0].diffuse.red = file.Read_Real32();
        g_theWriteableGlobalData->m_terrainObjectLighting[i + 1][0].diffuse.green = file.Read_Real32();
        g_theWriteableGlobalData->m_terrainObjectLighting[i + 1][0].diffuse.blue = file.Read_Real32();

        g_theWriteableGlobalData->m_terrainObjectLighting[i + 1][0].lightPos.x = file.Read_Real32();
        g_theWriteableGlobalData->m_terrainObjectLighting[i + 1][0].lightPos.y = file.Read_Real32();
        g_theWriteableGlobalData->m_terrainObjectLighting[i + 1][0].lightPos.z = file.Read_Real32();

        if (info->version >= 2) {
            for (int j = 1; j < LIGHT_COUNT; j++) {
                g_theWriteableGlobalData->m_terrainObjectLighting[i + 1][j].ambient.red = file.Read_Real32();
                g_theWriteableGlobalData->m_terrainObjectLighting[i + 1][j].ambient.green = file.Read_Real32();
                g_theWriteableGlobalData->m_terrainObjectLighting[i + 1][j].ambient.blue = file.Read_Real32();
                g_theWriteableGlobalData->m_terrainObjectLighting[i + 1][j].diffuse.red = file.Read_Real32();
                g_theWriteableGlobalData->m_terrainObjectLighting[i + 1][j].diffuse.green = file.Read_Real32();
                g_theWriteableGlobalData->m_terrainObjectLighting[i + 1][j].diffuse.blue = file.Read_Real32();
                g_theWriteableGlobalData->m_terrainObjectLighting[i + 1][j].lightPos.x = file.Read_Real32();
                g_theWriteableGlobalData->m_terrainObjectLighting[i + 1][j].lightPos.y = file.Read_Real32();
                g_theWriteableGlobalData->m_terrainObjectLighting[i + 1][j].lightPos.z = file.Read_Real32();
            }
        }

        if (info->version >= 3) {
            for (int j = 1; j < LIGHT_COUNT; j++) {
                g_theWriteableGlobalData->m_terrainPlaneLighting[i + 1][j].ambient.red = file.Read_Real32();
                g_theWriteableGlobalData->m_terrainPlaneLighting[i + 1][j].ambient.green = file.Read_Real32();
                g_theWriteableGlobalData->m_terrainPlaneLighting[i + 1][j].ambient.blue = file.Read_Real32();
                g_theWriteableGlobalData->m_terrainPlaneLighting[i + 1][j].diffuse.red = file.Read_Real32();
                g_theWriteableGlobalData->m_terrainPlaneLighting[i + 1][j].diffuse.green = file.Read_Real32();
                g_theWriteableGlobalData->m_terrainPlaneLighting[i + 1][j].diffuse.blue = file.Read_Real32();
                g_theWriteableGlobalData->m_terrainPlaneLighting[i + 1][j].lightPos.x = file.Read_Real32();
                g_theWriteableGlobalData->m_terrainPlaneLighting[i + 1][j].lightPos.y = file.Read_Real32();
                g_theWriteableGlobalData->m_terrainPlaneLighting[i + 1][j].lightPos.z = file.Read_Real32();
            }
        }
    }

    if (!file.At_End_Of_Chunk()) {
        int color = file.Read_Int32();

        if (g_theW3DShadowManager) {
            g_theW3DShadowManager->Set_Shadow_Color(color);
        }
    }

    captainslog_dbgassert(file.At_End_Of_Chunk(), "Unexpected data left over.");
    return true;
}

bool WorldHeightMap::Parse_Objects_Data_Chunk(DataChunkInput &file, DataChunkInfo *info, void *user_data)
{
    file.m_currentObject = nullptr;
    file.Register_Parser("Object", info->label, Parse_Object_Data_Chunk, nullptr);
    return file.Parse(user_data);
}

bool WorldHeightMap::Parse_Height_Map_Data_Chunk(DataChunkInput &file, DataChunkInfo *info, void *user_data)
{
    return static_cast<WorldHeightMap *>(user_data)->Parse_Height_Map_Data(file, info, user_data);
}

bool WorldHeightMap::Parse_Height_Map_Data(DataChunkInput &file, DataChunkInfo *info, void *user_data)
{
    m_width = file.Read_Int32();
    m_height = file.Read_Int32();

    if (info->version >= 3) {
        m_borderSize = file.Read_Int32();
    } else {
        m_borderSize = 0;
    }

    if (info->version >= 4) {
        int size = file.Read_Int32();
        m_boundaries.resize(size);

        for (int i = 0; i < size; i++) {
            m_boundaries[i].x = file.Read_Int32();
            m_boundaries[i].y = file.Read_Int32();
        }
    } else {
        m_boundaries.resize(1);
        m_boundaries[0].x = m_width - 2 * m_borderSize;
        m_boundaries[0].y = m_height - 2 * m_borderSize;
    }

    m_dataSize = file.Read_Int32();
    m_data = new unsigned char[m_dataSize];

    if (m_dataSize <= 0 || m_dataSize != m_height * m_width) {
        throw CODE_05;
    }

    m_seismicUpdateWidth = (m_width + 7) / 8;
    m_seismicUpdateFlag = new unsigned char[m_height * m_seismicUpdateWidth];
    Clear_Seismic_Update_Flags();
    m_seismicZVelocities = new float[m_dataSize];
    Fill_Seismic_Z_Velocities(0.0f);
    file.Read_Byte_Array(m_data, m_dataSize);

    if (info->version == 1) {
        int x = (m_width + 1) / 2;
        int y = (m_height + 1) / 2;

        for (int i = 0; i < y; i++) {
            for (int j = 0; j < x; j++) {
                m_data[j + y * i] = m_data[2 * j + m_width * 2 * i];
            }
        }
    }

    captainslog_dbgassert(file.At_End_Of_Chunk(), "Unexpected data left over.");
    return true;
}

bool WorldHeightMap::Parse_Size_Only_In_Chunk(DataChunkInput &file, DataChunkInfo *info, void *user_data)
{
    return static_cast<WorldHeightMap *>(user_data)->Parse_Size_Only(file, info, user_data);
}

bool WorldHeightMap::Parse_Size_Only(DataChunkInput &file, DataChunkInfo *info, void *user_data)
{
    m_width = file.Read_Int32();
    m_height = file.Read_Int32();

    if (info->version >= 3) {
        m_borderSize = file.Read_Int32();
    } else {
        m_borderSize = 0;
    }

    if (info->version >= 4) {
        int size = file.Read_Int32();
        m_boundaries.resize(size);

        for (int i = 0; i < size; i++) {
            m_boundaries[i].x = file.Read_Int32();
            m_boundaries[i].y = file.Read_Int32();
        }
    } else {
        m_boundaries.resize(1);
        m_boundaries[0].x = m_width - 2 * m_borderSize;
        m_boundaries[0].y = m_height - 2 * m_borderSize;
    }

    m_dataSize = file.Read_Int32();
    m_data = new unsigned char[m_dataSize];

    if (m_dataSize <= 0 || m_dataSize != m_height * m_width) {
        throw CODE_05;
    }

    file.Read_Byte_Array(m_data, m_dataSize);

    if (info->version == 1) {
        int x = (m_width + 1) / 2;
        int y = (m_height + 1) / 2;

        for (int i = 0; i < y; i++) {
            for (int j = 0; j < x; j++) {
                m_data[j + y * i] = m_data[2 * j + m_width * 2 * i];
            }
        }

        m_width = x;
    }

    return true;
}

bool WorldHeightMap::Parse_Blend_Tile_Data_Chunk(DataChunkInput &file, DataChunkInfo *info, void *user_data)
{
    return static_cast<WorldHeightMap *>(user_data)->Parse_Blend_Tile_Data(file, info, user_data);
}

bool WorldHeightMap::Parse_Blend_Tile_Data(DataChunkInput &file, DataChunkInfo *info, void *user_data)
{
    int size = file.Read_Int32();

    if (m_dataSize != size) {
        throw CODE_05;
    }

    m_tileNdxes = new short[m_dataSize];
    m_cliffInfoNdxes = new short[m_dataSize];
    m_blendTileNdxes = new short[m_dataSize];
    m_extraBlendTileNdxes = new short[m_dataSize];
    m_flipStateWidth = (m_width + 7) / 8;
    m_cellFlipState = new unsigned char[m_height * m_flipStateWidth];
    m_cliffState = new unsigned char[m_height * m_flipStateWidth];
    memset(m_cellFlipState, 0, m_height * m_flipStateWidth);
    memset(m_cliffState, 0, m_height * m_flipStateWidth);

    file.Read_Byte_Array(reinterpret_cast<uint8_t *>(m_tileNdxes), 2 * m_dataSize);
    file.Read_Byte_Array(reinterpret_cast<uint8_t *>(m_blendTileNdxes), 2 * m_dataSize);

    if (info->version >= 6) {
        file.Read_Byte_Array(reinterpret_cast<uint8_t *>(m_extraBlendTileNdxes), 2 * m_dataSize);

        if (!g_theWriteableGlobalData->m_use3WayTerrainBlends) {
            memset(m_extraBlendTileNdxes, 0, sizeof(short) * m_dataSize);
        }
    }

    if (info->version >= 5) {
        file.Read_Byte_Array(reinterpret_cast<uint8_t *>(m_cliffInfoNdxes), 2 * m_dataSize);
    }

    if (info->version >= 7) {
        if (info->version == 7) {
            int w = (m_width + 1) / 8;
            unsigned char *c = new unsigned char[w * m_height];
            file.Read_Byte_Array(c, w * m_height);

            for (int i = 0; i < m_height; i++) {
                for (int j = 0; j < w; j++) {
                    m_cliffState[j + m_flipStateWidth * i] = c[j + w * i];
                }
            }

            // BUGFIX memory leak
            delete[] c;
        } else {
            file.Read_Byte_Array(m_cliffState, m_flipStateWidth * m_height);
        }
    } else {
        Init_Cliff_Flags_From_Heights();
    }
    m_numBitmapTiles = file.Read_Int32();
    captainslog_dbgassert(m_numBitmapTiles > 0 || m_numBitmapTiles < 2048, "Unlikely numBitmapTiles.");
    m_numBlendedTiles = file.Read_Int32();
    captainslog_dbgassert(m_numBlendedTiles > 0 || m_numBlendedTiles < 16193, "Unlikely numBlendedTiles.");

    if (info->version >= 5) {
        m_numCliffInfo = file.Read_Int32();
    } else {
        m_numCliffInfo = 1;
    }

    m_numTextureClasses = file.Read_Int32();
    captainslog_dbgassert(m_numTextureClasses > 0 || m_numTextureClasses < 16193, "Unlikely m_numTextureClasses.");

    for (int i = 0; i < m_numTextureClasses; i++) {
        m_textureClasses[i].global_texture_class = 0xFFFFFFFF;
        m_textureClasses[i].first_tile = file.Read_Int32();
        m_textureClasses[i].num_tiles = file.Read_Int32();
        m_textureClasses[i].width = file.Read_Int32();
        file.Read_Int32();
        m_textureClasses[i].name = file.Read_AsciiString();
        Read_Tex_Class(&m_textureClasses[i], m_sourceTiles);
    }

    m_numEdgeTextureClasses = 0;
    m_numEdgeTiles = 0;

    if (info->version >= 4) {
        m_numEdgeTiles = file.Read_Int32();
        m_numEdgeTextureClasses = file.Read_Int32();

        for (int i = 0; i < m_numEdgeTextureClasses; i++) {
            m_edgeTextureClasses[i].global_texture_class = 0xFFFFFFFF;
            m_edgeTextureClasses[i].first_tile = file.Read_Int32();
            m_edgeTextureClasses[i].num_tiles = file.Read_Int32();
            m_edgeTextureClasses[i].width = file.Read_Int32();
            m_edgeTextureClasses[i].name = file.Read_AsciiString();
            Read_Tex_Class(&m_edgeTextureClasses[i], m_edgeTiles);
        }
    }

    for (int i = 1; i < m_numBlendedTiles; i++) {
        m_blendedTiles[i].blend_ndx = file.Read_Int32();
        m_blendedTiles[i].horiz = file.Read_Byte();
        m_blendedTiles[i].vert = file.Read_Byte();
        m_blendedTiles[i].right_diagonal = file.Read_Byte();
        m_blendedTiles[i].left_diagonal = file.Read_Byte();
        m_blendedTiles[i].inverted = file.Read_Byte();

        if (!g_theWriteableGlobalData->m_use3WayTerrainBlends) {
            m_blendedTiles[i].inverted &= ~2;
        }

        if (info->version >= 3) {
            m_blendedTiles[i].long_diagonal = file.Read_Byte();
        } else {
            m_blendedTiles[i].long_diagonal = 0;
        }

        if (info->version >= 4) {
            m_blendedTiles[i].custom_blend_edge_class = file.Read_Int32();
        } else {
            m_blendedTiles[i].custom_blend_edge_class = 0xFFFFFFFF;
        }

        int flag = file.Read_Int32();
        captainslog_dbgassert(flag == FLAG_VAL, "Invalid format.");

        if (flag != FLAG_VAL) {
            throw CODE_05;
        }
    }

    if (info->version >= 5) {
        for (int i = 1; i < m_numCliffInfo; i++) {
            m_cliffInfo[i].tile_index = file.Read_Int32();
            m_cliffInfo[i].u0 = file.Read_Real32();
            m_cliffInfo[i].v0 = file.Read_Real32();
            m_cliffInfo[i].u1 = file.Read_Real32();
            m_cliffInfo[i].v1 = file.Read_Real32();
            m_cliffInfo[i].u2 = file.Read_Real32();
            m_cliffInfo[i].v2 = file.Read_Real32();
            m_cliffInfo[i].u3 = file.Read_Real32();
            m_cliffInfo[i].v3 = file.Read_Real32();
            m_cliffInfo[i].flip = file.Read_Byte() != 0;
            m_cliffInfo[i].mutant = file.Read_Byte() != 0;
        }
    }

    if (info->version == 1) {
        int x = (m_width + 1) / 2;
        int y = (m_height + 1) / 2;

        for (int i = 0; i < y; i++) {
            for (int j = 0; j < x; j++) {
                m_tileNdxes[j + x * i] = m_tileNdxes[2 * j + m_width * 2 * i];
                m_blendTileNdxes[j + x * i] = 0;
                m_extraBlendTileNdxes[j + x * i] = 0;
                m_cliffInfoNdxes[j + x * i] = 0;
            }
        }

        m_numBlendedTiles = 1;
        m_numCliffInfo = 1;
        m_width = x;
        m_height = y;
        m_dataSize = m_height * m_width;
    }

    captainslog_dbgassert(file.At_End_Of_Chunk(), "Unexpected data left over.");
    return true;
}

void WorldHeightMap::Read_Tex_Class(TXTextureClass *tex_class, TileData **tile_data)
{
    File *file;
    TerrainType *terrain = g_theTerrainTypes->Find_Terrain(tex_class->name);

    if (!terrain) {
        file = g_theFileSystem->Open_File(tex_class->name.Str(), File::BINARY | File::READ);
    } else {
        char fname[260];
        sprintf(fname, "%s%s", "Art/Terrain/", terrain->Get_Texture().Str());
        file = g_theFileSystem->Open_File(fname, File::BINARY | File::READ);
    }

    if (file) {
        GDIFileStream stream(file);
        int numTiles = Count_Tiles(&stream, nullptr);
        file->Seek(0, File::START);

        if (numTiles >= tex_class->num_tiles) {
            numTiles = tex_class->num_tiles;
            int i;

            for (i = 10; i >= 1; i--) {
                if (numTiles >= i * i) {
                    numTiles = i * i;
                    break;
                }
            }

            Read_Tiles(&stream, &tile_data[tex_class->first_tile], i);
        }

        file->Close();
    }
}

bool WorldHeightMap::Parse_Object_Data_Chunk(DataChunkInput &file, DataChunkInfo *info, void *user_data)
{
    return static_cast<WorldHeightMap *>(file.m_userData)->Parse_Object_Data(file, info, user_data, info->version >= 2);
}

bool WorldHeightMap::Parse_Object_Data(DataChunkInput &file, DataChunkInfo *info, void *user_data, bool read_dict)
{
    MapObject *pPrevious = static_cast<MapObject *>(file.m_currentObject);
    Coord3D loc;
    loc.x = file.Read_Real32();
    loc.y = file.Read_Real32();
    loc.z = file.Read_Real32();
    float minz = -1000.0f;
    float f2 = 1593.75f;

    if (info->version <= 2) {
        loc.z = 0.0f;
    }

    float angle = file.Read_Real32();
    int flags = file.Read_Int32();
    Utf8String name = file.Read_AsciiString();
    Dict d(0);

    if (read_dict) {
        d = file.Read_Dict();
    }

    if (loc.z < minz || loc.z > f2) {
        captainslog_debug("Removing object at z height %f", loc.z);
    } else {
        MapObject *obj = NEW_POOL_OBJ(MapObject, loc, name, angle, flags, &d, g_theThingFactory->Find_Template(name, false));

        if (obj->Get_Properties()->Get_Type(g_waypointIDKey) == Dict::DICT_INT) {
            obj->Set_Is_Waypoint();
        }

        if (obj->Get_Properties()->Get_Type(g_lightHeightAboveTerrainKey) == Dict::DICT_REAL) {
            obj->Set_Is_Light();
        }

        if (obj->Get_Properties()->Get_Type(g_scorchTypeKey) == Dict::DICT_INT) {
            obj->Set_Is_Scorch();
        }

        if (pPrevious) {
            captainslog_dbgassert(MapObject::s_theMapObjectListPtr && !pPrevious->Get_Next(), "Bad linkage.");

            pPrevious->Set_Next_Map(obj);
        } else {
            captainslog_dbgassert(!MapObject::s_theMapObjectListPtr, "Bad linkage.");
            MapObject::s_theMapObjectListPtr = obj;
        }

        file.m_currentObject = obj;
    }

    return true;
}

int WorldHeightMap::Count_Tiles(InputStream *str, bool *b)
{
    TGAHeader hdr;

    if (b) {
        *b = false;
    }

    if (str->Read(&hdr, sizeof(hdr)) != sizeof(hdr)) {
        return 0;
    }

    int w = hdr.width / TILE_PIXEL_EXTENT;
    int h = hdr.height / TILE_PIXEL_EXTENT;

    if (hdr.cmap_type) {
        return 0;
    }

    if (hdr.image_type != TGA_TYPE_COLOR && hdr.image_type != TGA_TYPE_RLE_COLOR) {
        return 0;
    }

    if (hdr.pixel_depth < 24) {
        return 0;
    }

    if (hdr.pixel_depth > 32) {
        return 0;
    }

    if (w > 10 || h > 10) {
        return 0;
    }

    for (int i = 10; i > 0; --i) {
        if (w >= i && h >= i) {
            return i * i;
        }
    }

    if (!b || hdr.height != 32 || hdr.width != 32) {
        return 0;
    }

    *b = true;
    return 1;
}

bool WorldHeightMap::Read_Tiles(InputStream *str, TileData **tiles, int num_rows)
{
    TGAHeader hdr;

    if (str->Read(&hdr, sizeof(hdr)) != sizeof(hdr)) {
        return 0;
    }

    int w = hdr.width / TILE_PIXEL_EXTENT;
    int h = hdr.height / TILE_PIXEL_EXTENT;

    if (hdr.height == 32) {
        h = 1;
    }

    if (hdr.width == 32) {
        w = 1;
    }

    if (w < num_rows && h < num_rows) {
        return false;
    }

    bool compressed = (hdr.image_type & 8) != 0;
    int bytesPerPixel = (hdr.pixel_depth + 7) / 8;

    if (bytesPerPixel < 3) {
        return false;
    }

    if (bytesPerPixel > 4) {
        return false;
    }

    for (int i = 0; i < num_rows * num_rows; i++) {
        if (!tiles[i]) {
            tiles[i] = new TileData();
        }
    }

    int repeatCount = 0;
    bool running = false;

    for (int i = 0; i < num_rows * TILE_PIXEL_EXTENT; i++) {
        for (int j = 0; j < hdr.width; j++) {
            unsigned char a;
            unsigned char r;
            unsigned char g;
            unsigned char b;

            if (i >= hdr.height) {
                a = 0;
                r = 0;
                g = 0;
                b = 0;
            } else {
                unsigned char buf[4];

                if (compressed && !repeatCount) {
                    unsigned char flag;
                    str->Read(&flag, 1);
                    repeatCount = (flag & 127) + 1;

                    if ((flag & 128) != 0) {
                        running = true;
                        str->Read(&buf, bytesPerPixel);
                    } else {
                        running = false;
                    }
                }

                if (compressed) {
                    repeatCount--;
                }

                if (!running) {
                    str->Read(&buf, bytesPerPixel);
                }

                b = buf[2];
                g = buf[1];
                r = buf[0];

                if (bytesPerPixel == 4) {
                    a = buf[3];
                } else {
                    a = 255;
                }
            }

            if (j < num_rows * TILE_PIXEL_EXTENT) {
                unsigned char *data = tiles[i / TILE_PIXEL_EXTENT * num_rows + j / TILE_PIXEL_EXTENT]->Get_Data_Ptr();
                unsigned char *d = &data[4 * (((i % TILE_PIXEL_EXTENT) * TILE_PIXEL_EXTENT) + j % TILE_PIXEL_EXTENT)];
                *d++ = r;
                *d++ = g;
                *d++ = b;
                *d = a;
            }
        }

        captainslog_dbgassert(!repeatCount, "Invalid tga.");
    }

    for (int i = 0; i < num_rows * num_rows; i++) {
        tiles[i]->Update_Mips();
    }

    return true;
}

int WorldHeightMap::Update_Tile_Texture_Positions(int *edge_height)
{
    bool available_grid[28][28];
    int max_height = 0;

    for (int i = 0; i < 28; i++) {
        for (int j = 0; j < 28; j++) {
            available_grid[i][j] = true;
        }
    }

    for (int i = 0; i < m_numBitmapTiles; i++) {
        if (m_sourceTiles[i]) {
            m_sourceTiles[i]->m_tileLocationInTexture.x = 0;
            m_sourceTiles[i]->m_tileLocationInTexture.y = 0;
        }
    }

    for (int width = 28; width > 0; width--) {
        for (int tex_class = 0; tex_class < m_numTextureClasses; tex_class++) {
            int tex_class_width = m_textureClasses[tex_class].width;

            if (tex_class_width == width) {
                bool b = 0;

                int i;
                int j;
                for (i = 0; i < 28 - tex_class_width + 1; i++) {

                    for (j = 0; j < 28 - tex_class_width + 1; j++) {
                        if (available_grid[i][j]) {
                            bool b2 = true;

                            for (int k = 0; k < tex_class_width && b2; k++) {
                                for (int l = 0; l < tex_class_width && b2; l++) {
                                    if (!available_grid[i + l][j + k]) {
                                        b2 = false;
                                    }
                                }
                            }

                            if (b2) {
                                b = true;
                            }
                            break;
                        }
                    }

                    if (b) {
                        break;
                    }
                }

                if (b) {
                    int x = 72 * j + 4;
                    int y = 72 * i + 4;
                    m_textureClasses[tex_class].position_in_texture.x = x;
                    m_textureClasses[tex_class].position_in_texture.y = y;

                    if (max_height < y + (tex_class_width * TILE_PIXEL_EXTENT) + 4) {
                        max_height = y + (tex_class_width * TILE_PIXEL_EXTENT) + 4;
                    }

                    for (int k = 0; k < tex_class_width; k++) {
                        for (int l = 0; l < tex_class_width; l++) {
                            available_grid[i + l][j + k] = 0;
                            int src_tile_index = tex_class_width * l + k + m_textureClasses[tex_class].first_tile;

                            if (m_sourceTiles[src_tile_index]) {
                                m_sourceTiles[src_tile_index]->m_tileLocationInTexture.x = (k * TILE_PIXEL_EXTENT) + x;
                                m_sourceTiles[src_tile_index]->m_tileLocationInTexture.y =
                                    ((tex_class_width - l - 1) * TILE_PIXEL_EXTENT) + y;
                            }
                        }
                    }
                } else {
                    m_textureClasses[tex_class].position_in_texture.x = 0;
                    m_textureClasses[tex_class].position_in_texture.y = 0;
                }
            }
        }
    }

    for (int bmp_tile_index = 0; bmp_tile_index < m_numBitmapTiles; bmp_tile_index++) {
        if (m_edgeTiles[bmp_tile_index]) {
            m_edgeTiles[bmp_tile_index]->m_tileLocationInTexture.x = 0;
            m_edgeTiles[bmp_tile_index]->m_tileLocationInTexture.y = 0;
        }
    }

    int max_edge_height = 0;

    for (int i = 0; i < 28; i++) {
        for (int j = 0; j < 28; j++) {
            available_grid[i][j] = true;
        }
    }

    for (int tex_class = 0; tex_class < m_numEdgeTextureClasses; tex_class++) {
        int tex_class_width = m_edgeTextureClasses[tex_class].width;
        bool b = 0;

        int i;
        int j;
        for (i = 0; i < 28 - tex_class_width + 1; i++) {
            for (j = 0; j < 28 - tex_class_width + 1; j++) {
                if (available_grid[i][j]) {
                    bool b2 = true;

                    for (int k = 0; k < tex_class_width && b2; ++k) {
                        for (int l = 0; l < tex_class_width && b2; l++) {
                            if (!available_grid[i + l][j + k]) {
                                b2 = false;
                            }
                        }
                    }

                    if (b2) {
                        b = true;
                    }
                    break;
                }
            }

            if (b) {
                break;
            }
        }

        if (b) {
            int x = 72 * j + 4;
            int y = 72 * i + 4;
            m_edgeTextureClasses[tex_class].position_in_texture.x = x;
            m_edgeTextureClasses[tex_class].position_in_texture.y = y;

            if (max_edge_height < y + (tex_class_width * TILE_PIXEL_EXTENT) + 4) {
                max_edge_height = y + (tex_class_width * TILE_PIXEL_EXTENT) + 4;
            }

            for (int k = 0; k < tex_class_width; k++) {
                for (int l = 0; l < tex_class_width; l++) {
                    available_grid[i + l][j + k] = 0;
                    int edge_tile_index = tex_class_width * l + k + m_edgeTextureClasses[tex_class].first_tile;

                    if (m_edgeTiles[edge_tile_index]) {
                        m_edgeTiles[edge_tile_index]->m_tileLocationInTexture.x = (k * TILE_PIXEL_EXTENT) + x;
                        m_edgeTiles[edge_tile_index]->m_tileLocationInTexture.y =
                            ((tex_class_width - l - 1) * TILE_PIXEL_EXTENT) + y;
                    }
                }
            }
        } else {
            m_edgeTextureClasses[tex_class].position_in_texture.x = 0;
            m_edgeTextureClasses[tex_class].position_in_texture.y = 0;
        }
    }

    if (edge_height) {
        *edge_height = max_edge_height;
    }

    return max_height;
}

void WorldHeightMap::Get_UV_For_Ndx(int tile_ndx, float *min_u, float *min_v, float *max_u, float *max_v, bool full_tile)
{
    if (!m_sourceTiles[tile_ndx / 4]) {
        *max_v = 0.0f;
        *max_u = 0.0f;
        *min_v = 0.0f;
        *min_u = 0.0f;
    } else {
        TileData *tile = m_sourceTiles[tile_ndx / 4];
        *min_u = (float)tile->m_tileLocationInTexture.x;
        *min_v = (float)tile->m_tileLocationInTexture.y;
        *max_u = *min_u + 64.0f;
        *max_v = *min_v + 64.0f;
        *min_u = *min_u / 2048.0f;
        *min_v = *min_v / (float)m_terrainTexHeight;
        *max_u = *max_u / 2048.0f;
        *max_v = *max_v / (float)m_terrainTexHeight;

        if (!full_tile) {
            float u = (*min_u + *max_u) / 2.0f;
            float v = (*min_v + *max_v) / 2.0f;

            if ((tile_ndx & 2) != 0) {
                *max_v = v;
            } else {
                *min_v = v;
            }

            if ((tile_ndx & 1) != 0) {
                *min_u = u;
            } else {
                *max_u = u;
            }
        }
    }
}

bool WorldHeightMap::Is_Cliff_Mapped_Texture(int x, int y)
{
    int i = (m_drawOriginY + y) * m_width + m_drawOriginX + x;
    return i >= 0 && i < m_dataSize && m_cliffInfoNdxes[i] != 0;
}

bool WorldHeightMap::Get_UV_Data(int x_index, int y_index, float *const u, float *const v, bool full_tile)
{
    int i = m_drawOriginX + x_index + m_width * (m_drawOriginY + y_index);

    if (i < m_dataSize && m_tileNdxes) {
        return Get_UV_For_Tile_Index(i, m_tileNdxes[i], u, v, full_tile);
    } else {
        return false;
    }
}

bool WorldHeightMap::Get_UV_For_Tile_Index(int ndx, short tile_ndx, float *const u, float *const v, bool full_tile)
{
    static float STRETCH_LIMIT = 1.5f;
    static float TILE_LIMIT = 4.0f;
    static float TALL_STRETCH_LIMIT = 2.0f;
    static float DIAMOND_STRETCH_LIMIT = 2.4f;
    static float HEIGHT_SCALE = 0.0625f;

    float max_v = 0.0f;
    float max_u = 0.0f;
    float min_v = 0.0f;
    float min_u = 0.0f;

    if (ndx >= m_dataSize || !m_tileNdxes) {
        return false;
    }

    Get_UV_For_Ndx(tile_ndx, &min_u, &min_v, &max_u, &max_v, full_tile);
    u[0] = min_u;
    u[1] = max_u;
    u[2] = max_u;
    u[3] = min_u;
    v[0] = max_v;
    v[1] = max_v;
    v[2] = min_v;
    v[3] = min_v;

    if (g_theWriteableGlobalData && !g_theWriteableGlobalData->m_adjustCliffTextures) {
        return false;
    }

    if (min_u == 0.0f) {
        return false;
    }

    if (full_tile) {
        return false;
    }

    if (m_cliffInfoNdxes[ndx]) {
        TCliffInfo cliff = m_cliffInfo[m_cliffInfoNdxes[ndx]];
        bool b = false;
        int i1 = tile_ndx / 4;
        int i2 = cliff.tile_index / 4;
        int i;

        for (i = 0; i < m_numTextureClasses; i++) {
            if (i1 >= m_textureClasses[i].first_tile
                && i1 < m_textureClasses[i].first_tile + m_textureClasses[i].num_tiles) {
                b = i2 >= m_textureClasses[i].first_tile
                    && i2 < m_textureClasses[i].first_tile + m_textureClasses[i].num_tiles;
                break;
            }
        }

        if (b) {
            float f1 = (float)m_textureClasses[i].position_in_texture.x;
            float f2 = (float)((m_textureClasses[i].width * TILE_PIXEL_EXTENT) + m_textureClasses[i].position_in_texture.y);
            float f3 = f1 / 2048.0f;
            float f4 = f2 / (float)m_terrainTexHeight;
            float f5 = (float)(2048 / m_terrainTexHeight);
            u[0] = cliff.u0 + f3;
            u[1] = cliff.u1 + f3;
            u[2] = cliff.u2 + f3;
            u[3] = cliff.u3 + f3;
            v[0] = cliff.v0 * f5 + f4;
            v[1] = cliff.v1 * f5 + f4;
            v[2] = cliff.v2 * f5 + f4;
            v[3] = cliff.v3 * f5 + f4;
            return cliff.flip;
        }
    }

    max_v = 0.0f;
    max_u = 0.0f;
    min_v = 0.0f;
    min_u = 0.0f;
    Get_UV_For_Ndx(tile_ndx, &min_u, &min_v, &max_u, &max_v, false);
    u[0] = min_u;
    u[1] = max_u;
    u[2] = max_u;
    u[3] = min_u;
    v[0] = max_v;
    v[1] = max_v;
    v[2] = min_v;
    v[3] = min_v;

    if (g_theWriteableGlobalData && !g_theWriteableGlobalData->m_adjustCliffTextures) {
        return false;
    }

    if (min_u == 0.0f) {
        return false;
    }

    if (ndx < m_dataSize - m_width - 1) {
        int i3 = m_data[ndx];
        int i4 = m_data[ndx + 1];
        int i5 = m_data[m_width + 1 + ndx];
        int i6 = m_data[m_width + ndx];
        int i7 = i3;
        int i8 = i3;

        if (i3 > i4) {
            i8 = m_data[ndx + 1];
        }

        if (i3 < i4) {
            i7 = m_data[ndx + 1];
        }

        if (i8 > i5) {
            i8 = m_data[m_width + 1 + ndx];
        }

        if (i7 < i5) {
            i7 = m_data[m_width + 1 + ndx];
        }

        if (i8 > i6) {
            i8 = m_data[m_width + ndx];
        }

        if (i7 < i6) {
            i7 = m_data[m_width + ndx];
        }

        float i9 = i7 - i8;
        float i10 = 0;
        float i11 = 0;
        float i12 = (2 * (i7 - i8) + 1) / 3 + i8;
        float i13 = (i7 - i8 + 1) / 3 + i8;

        if (i3 < i12) {
            i10 = 1;
        }

        if (i4 < i12) {
            ++i10;
        }

        if (i5 < i12) {
            ++i10;
        }

        if (i6 < i12) {
            ++i10;
        }

        if (i3 > i13) {
            i11 = 1;
        }

        if (i4 > i13) {
            ++i11;
        }

        if (i5 > i13) {
            ++i11;
        }

        if (i6 > i13) {
            ++i11;
        }

        if ((float)i9 * HEIGHT_SCALE < STRETCH_LIMIT) {
            return false;
        }

        int j;
        for (j = 0; j < m_numTextureClasses
             && (m_textureClasses[j].first_tile < 0 || (tile_ndx / 4) < m_textureClasses[j].first_tile
                 || (tile_ndx / 4) >= m_textureClasses[j].first_tile + m_textureClasses[j].num_tiles);
             j++) {
            ;
        }

        if (j >= m_numTextureClasses) {
            return false;
        }

        float f6 = (float)m_textureClasses[j].position_in_texture.x;
        float f7 = (float)m_textureClasses[j].position_in_texture.y;
        float f8 = (float)(m_textureClasses[j].width * TILE_PIXEL_EXTENT) + f6;
        float f9 = (float)(m_textureClasses[j].width * TILE_PIXEL_EXTENT) + f7;
        float f10 = f7 / (float)m_terrainTexHeight;
        float f11 = f8 / 2048.0f;
        float f12 = f9 / (float)m_terrainTexHeight;
        float f13 = TILE_LIMIT / ((float)i9 * HEIGHT_SCALE);

        if (f13 > TILE_LIMIT) {
            f13 = TILE_LIMIT;
        }

        if (f13 < 1.0f) {
            f13 = 1.0f;
        }

        float f14 = f12 - f10;

        if (i11 != 1 && i10 != 1 && (i11 != 2 || i10 != 2) && (float)i9 * HEIGHT_SCALE < DIAMOND_STRETCH_LIMIT) {
            return false;
        }

        if (i10 == 1 || i11 > i10) {
            if (i3 == i8) {
                v[0] = f14 / f13 + min_v;
            } else if (i4 == i8) {
                v[1] = f14 / f13 + min_v;
            } else if (i5 == i8) {
                v[2] = max_v - f14 / f13;
            } else if (i6 == i8) {
                v[3] = max_v - f14 / f13;
            }
        } else if (i11 == 1 || i10 > i11) {
            if (i3 == i7) {
                v[0] = f14 / f13 + min_v;
            } else if (i4 == i7) {
                v[1] = f14 / f13 + min_v;
            } else if (i5 == i7) {
                v[2] = max_v - f14 / f13;
            } else if (i6 == i7) {
                v[3] = max_v - f14 / f13;
            }
        } else {
            if ((float)i9 * HEIGHT_SCALE < TALL_STRETCH_LIMIT) {
                return 0;
            }

            float f15 = (float)(i6 - i5) * HEIGHT_SCALE;
            float f16 = GameMath::Sqrt(f15 * f15 + 1.0f);
            float f17 = (float)(i6 - i3) * HEIGHT_SCALE;
            float f18 = GameMath::Sqrt(f17 * f17 + 1.0f);

            if (f16 < (float)STRETCH_LIMIT) {
                f16 = 1.0f;
            }

            if (f18 < (float)STRETCH_LIMIT) {
                f18 = 1.0f;
            }

            if (f16 > (float)TILE_LIMIT) {
                f16 = TILE_LIMIT;
            }

            if (f18 > (float)TILE_LIMIT) {
                f18 = TILE_LIMIT;
            }

            float f19 = (max_u - min_u) * f16;
            float f20 = (max_v - min_v) * f18;
            u[0] = min_u;
            u[1] = min_u + f19;
            u[2] = min_u + f19;
            u[3] = min_u;
            v[0] = min_v + f20;
            v[1] = min_v + f20;
            v[2] = min_v;
            v[3] = min_v;

            float f21 = (float)(i4 - i3) * HEIGHT_SCALE;
            float f22 = GameMath::Sqrt(f21 * f21 + 1.0f);
            float f23 = (float)(i5 - i4) * HEIGHT_SCALE;
            float f24 = GameMath::Sqrt(f23 * f23 + 1.0f);

            if (f22 < (float)STRETCH_LIMIT) {
                f22 = 1.0f;
            }

            if (f24 < (float)STRETCH_LIMIT) {
                f24 = 1.0f;
            }

            if (f22 > (float)TILE_LIMIT) {
                f22 = TILE_LIMIT;
            }

            if (f24 > (float)TILE_LIMIT) {
                f24 = TILE_LIMIT;
            }

            float f25 = (max_u - min_u) * f22;
            float f26 = (max_v - min_v) * f24;
            u[1] = f25 + *u;
            v[1] = f26 + v[3];
        }

        float f27 = 0.0f;
        float f28 = 0.0f;

        for (int i = 0; i < 4; i++) {
            if (f10 - v[i] > f28) {
                f28 = f10 - v[i];
            }
        }

        for (int i = 0; i < 4; i++) {
            v[i] = f28 + v[i];
        }

        float f29 = 0.0f;

        for (int i = 0; i < 4; i++) {
            if (u[i] - f11 > f27) {
                f27 = u[i] - f11;
            }
            if (v[i] - f12 > f29) {
                f29 = v[i] - f12;
            }
        }

        for (int i = 0; i < 4; i++) {
            u[i] = u[i] - f27;
            v[i] = v[i] - f29;
        }
    }

    return true;
}

bool WorldHeightMap::Get_Extra_Alpha_UV_Data(
    int x_index, int y_index, float *const u, float *const v, unsigned char *const alpha, bool *need_flip, bool *cliff)
{
    int ndx = x_index + m_width * y_index;
    *need_flip = false;
    *cliff = false;

    if (ndx >= 0 && ndx < m_dataSize && m_tileNdxes) {
        short tile_ndx = m_extraBlendTileNdxes[ndx];

        if (!tile_ndx) {
            return false;
        }

        *cliff = WorldHeightMap::Get_UV_For_Tile_Index(ndx, m_blendedTiles[tile_ndx].blend_ndx, u, v, false);
        alpha[3] = 0;
        alpha[2] = 0;
        alpha[1] = 0;
        alpha[0] = 0;

        if (m_blendedTiles[tile_ndx].horiz) {
            *need_flip = (m_blendedTiles[tile_ndx].inverted & 2) != 0;
            if ((m_blendedTiles[tile_ndx].inverted & 1) != 0) {
                alpha[3] = 0xFF;
                alpha[0] = 0xFF;
            } else {
                alpha[2] = 0xFF;
                alpha[1] = 0xFF;
            }
        }

        if (m_blendedTiles[tile_ndx].vert) {
            *need_flip = (m_blendedTiles[tile_ndx].inverted & 2) != 0;
            if ((m_blendedTiles[tile_ndx].inverted & 1) != 0) {
                alpha[1] = 0xFF;
                alpha[0] = 0xFF;
            } else {
                alpha[3] = 0xFF;
                alpha[2] = 0xFF;
            }
        }

        if (m_blendedTiles[tile_ndx].right_diagonal) {
            if ((m_blendedTiles[tile_ndx].inverted & 1) != 0) {
                alpha[1] = 0xFF;
                if (m_blendedTiles[tile_ndx].long_diagonal) {
                    alpha[0] = 0xFF;
                    alpha[2] = 0xFF;
                }
            } else {
                *need_flip = true;
                alpha[2] = 0xFF;
                if (m_blendedTiles[tile_ndx].long_diagonal) {
                    alpha[1] = 0xFF;
                    alpha[3] = 0xFF;
                }
            }
        }

        if (m_blendedTiles[tile_ndx].left_diagonal) {
            if ((m_blendedTiles[tile_ndx].inverted & 1) != 0) {
                *need_flip = true;
                alpha[0] = 0xFF;
                if (m_blendedTiles[tile_ndx].long_diagonal) {
                    alpha[1] = 0xFF;
                    alpha[3] = 0xFF;
                }
            } else {
                alpha[3] = 0xFF;
                if (m_blendedTiles[tile_ndx].long_diagonal) {
                    alpha[0] = 0xFF;
                    alpha[2] = 0xFF;
                }
            }
        }

        if (m_blendedTiles[tile_ndx].custom_blend_edge_class >= 0) {
            alpha[3] = 0;
            alpha[2] = 0;
            alpha[1] = 0;
            alpha[0] = 0;
            *need_flip = false;
        }
    }

    return true;
}

void WorldHeightMap::Get_Alpha_UV_Data(
    int x_index, int y_index, float *const u, float *const v, unsigned char *const alpha, bool *need_flip, bool full_tile)
{
    int i1 = m_drawOriginX + x_index;
    int i2 = m_drawOriginY + y_index;
    int ndx = i1 + m_width * i2;
    bool cliff = false;
    bool flip = false;

    if (ndx < m_dataSize) {
        if (m_tileNdxes) {
            short tile_ndx = m_blendTileNdxes[ndx];

            if (full_tile) {
                tile_ndx = 0;
            }

            if (!tile_ndx) {
                cliff = Get_UV_For_Tile_Index(ndx, m_tileNdxes[ndx], u, v, full_tile);
                alpha[3] = 0;
                alpha[2] = 0;
                alpha[1] = 0;
                alpha[0] = 0;
                flip = false;
            } else {
                cliff = Get_UV_For_Tile_Index(ndx, m_blendedTiles[tile_ndx].blend_ndx, u, v, full_tile);
                alpha[3] = 0;
                alpha[2] = 0;
                alpha[1] = 0;
                alpha[0] = 0;

                if (m_blendedTiles[tile_ndx].horiz) {
                    flip = (m_blendedTiles[tile_ndx].inverted & 2) != 0;
                    if ((m_blendedTiles[tile_ndx].inverted & 1) != 0) {
                        alpha[3] = 0xFF;
                        alpha[0] = 0xFF;
                    } else {
                        alpha[2] = 0xFF;
                        alpha[1] = 0xFF;
                    }
                }

                if (m_blendedTiles[tile_ndx].vert) {
                    flip = (m_blendedTiles[tile_ndx].inverted & 2) != 0;
                    if ((m_blendedTiles[tile_ndx].inverted & 1) != 0) {
                        alpha[1] = 0xFF;
                        alpha[0] = 0xFF;
                    } else {
                        alpha[3] = 0xFF;
                        alpha[2] = 0xFF;
                    }
                }

                if (m_blendedTiles[tile_ndx].right_diagonal) {
                    if ((m_blendedTiles[tile_ndx].inverted & 1) != 0) {
                        alpha[1] = 0xFF;
                        if (m_blendedTiles[tile_ndx].long_diagonal) {
                            alpha[0] = 0xFF;
                            alpha[2] = 0xFF;
                        }
                    } else {
                        flip = true;
                        alpha[2] = 0xFF;
                        if (m_blendedTiles[tile_ndx].long_diagonal) {
                            alpha[1] = 0xFF;
                            alpha[3] = 0xFF;
                        }
                    }
                }

                if (m_blendedTiles[tile_ndx].left_diagonal) {
                    if ((m_blendedTiles[tile_ndx].inverted & 1) != 0) {
                        flip = true;
                        alpha[0] = 0xFF;
                        if (m_blendedTiles[tile_ndx].long_diagonal) {
                            alpha[1] = 0xFF;
                            alpha[3] = 0xFF;
                        }
                    } else {
                        alpha[3] = 0xFF;
                        if (m_blendedTiles[tile_ndx].long_diagonal) {
                            alpha[0] = 0xFF;
                            alpha[2] = 0xFF;
                        }
                    }
                }

                if (m_blendedTiles[tile_ndx].custom_blend_edge_class >= 0) {
                    alpha[3] = 0;
                    alpha[2] = 0;
                    alpha[1] = 0;
                    alpha[0] = 0;
                    flip = 0;
                }
            }
        }
    }

    if (cliff) {
        int h1 = Get_Height(i1, i2);
        int h2 = Get_Height(i1 + 1, i2);
        int h3 = Get_Height(i1 + 1, i2 + 1);
        int h4 = Get_Height(i1, i2 + 1);
        int h5 = abs(h1 - h3);
        flip = h5 > abs(h2 - h4);
    }

    *need_flip = flip;
}

void WorldHeightMap::Set_Texture_LOD(int lod)
{
    if (m_terrainTex) {
        m_terrainTex->Set_LOD(lod);
    }
}

TextureClass *WorldHeightMap::Get_Terrain_Texture()
{
    if (!m_terrainTex) {
        int edge_height;
        int max_height = Update_Tile_Texture_Positions(&edge_height);
        int height;

        for (height = 1; height < max_height; height *= 2) {
            ;
        }

        Ref_Ptr_Release(m_terrainTex);
        m_terrainTex = new TerrainTextureClass(height);
        m_terrainTexHeight = m_terrainTex->Update(this);
        captainslog_debug("Base tex height %d", height);
        Ref_Ptr_Release(m_alphaTerrainTex);
        m_alphaTerrainTex = new AlphaTerrainTextureClass(m_terrainTex);

        for (height = 1; height < edge_height; height *= 2) {
            ;
        }

        Ref_Ptr_Release(m_alphaEdgeTex);
        m_alphaEdgeTex = new AlphaEdgeTextureClass(height, MIP_LEVELS_3);
        m_alphaEdgeHeight = m_alphaEdgeTex->Update(this);

        for (int y = 0; y < m_height - 1; y++) {
            for (int x = 0; x < m_width - 1; x++) {
                float u[4];
                float v[4];
                unsigned char alpha[4];
                bool flip;
                Get_Alpha_UV_Data(x, y, u, v, alpha, &flip, false);
                m_cellFlipState[(x / 8) + m_flipStateWidth * y] |= flip << (x & 7);
                captainslog_dbgassert((x / 8) + m_flipStateWidth * y < m_height * m_flipStateWidth, "Bad range");
            }
        }
    }

    return m_terrainTex;
}

TextureClass *WorldHeightMap::Get_Edge_Terrain_Texture()
{
    if (m_alphaEdgeTex == nullptr) {
        Get_Terrain_Texture();
    }

    return m_alphaEdgeTex;
}

TerrainTextureClass *WorldHeightMap::Get_Flat_Texture(int x, int y, int pixels_per_cell, int cell_width)
{
    if (g_theWriteableGlobalData->m_textureReductionFactor) {
        if (g_theWriteableGlobalData->m_textureReductionFactor > 1) {
            cell_width /= 4;
        } else {
            cell_width /= 2;
        }
    }

    int height;

    for (height = 1; height < cell_width * pixels_per_cell; height *= 2) {
        ;
    }

    TerrainTextureClass *tex = new TerrainTextureClass(height, height);
    tex->Update_Flat(this, x, y, pixels_per_cell, cell_width);
    return tex;
}

bool WorldHeightMap::Set_Draw_Origin(int x_origin, int y_origin)
{
    int width = m_drawWidthX;
    int height = m_drawHeightY;

    if (g_theWriteableGlobalData) {
        if (g_theWriteableGlobalData->m_stretchTerrain) {
            width = 65;
            height = 65;
        }
    }

    if (g_theWriteableGlobalData) {
        if (g_theWriteableGlobalData->m_drawEntireTerrain) {
            width = m_width;
            height = m_height;
        }
    }

    if (width > m_width) {
        width = m_width;
    }

    if (height > m_height) {
        height = m_height;
    }

    if (x_origin > m_width - width) {
        x_origin = m_width - width;
    }

    if (x_origin < 0) {
        x_origin = 0;
    }

    if (y_origin > m_height - height) {
        y_origin = m_height - height;
    }

    if (y_origin < 0) {
        y_origin = 0;
    }

    if (m_drawOriginX == x_origin && m_drawOriginY == y_origin && m_drawWidthX == width && m_drawHeightY == height) {
        return false;
    }

    m_drawOriginX = x_origin;
    m_drawOriginY = y_origin;
    m_drawWidthX = width;
    m_drawHeightY = height;
    return true;
}

void WorldHeightMap::Set_Cell_Cliff_Flag_From_Heights(int x_index, int y_index)
{
    float f1 = (float)Get_Height(x_index, y_index) * HEIGHTMAP_SCALE;
    float f2 = (float)Get_Height(x_index + 1, y_index) * HEIGHTMAP_SCALE;
    float f3 = (float)Get_Height(x_index, y_index + 1) * HEIGHTMAP_SCALE;
    float f4 = (float)Get_Height(x_index + 1, y_index + 1) * HEIGHTMAP_SCALE;
    float f5 = f1;

    if (f1 > (float)f2) {
        f5 = f2;
    }

    if (f5 > (float)f3) {
        f5 = f3;
    }

    if (f5 > (float)f4) {
        f5 = f4;
    }

    float f6 = f1;

    if (f1 < (float)f2) {
        f6 = f2;
    }

    if (f6 < (float)f3) {
        f6 = f3;
    }

    if (f6 < (float)f4) {
        f6 = f4;
    }

    Set_Cliff_State(x_index, y_index, f6 - f5 > 9.8f);
}

void WorldHeightMap::Get_Terrain_Color_At(float x, float y, RGBColor *color)
{
    int i1 = GameMath::Fast_To_Int_Floor(x / 10.0f);
    int i2 = GameMath::Fast_To_Int_Floor(y / 10.0f);
    int i3 = m_borderSize + i1;
    int i4 = m_borderSize + i2;
    color->blue = 0.0f;
    color->green = 0.0f;
    color->red = 0.0f;

    if (i3 < 0) {
        i3 = 0;
    }

    if (i4 < 0) {
        i4 = 0;
    }

    if (i3 >= m_width) {
        i3 = m_width - 1;
    }

    if (i4 >= m_height) {
        i4 = m_height - 1;
    }

    int i5 = i3 + m_width * i4;

    if (i5 >= 0 && i5 < m_dataSize) {
        TileData *tile = Get_Source_Tile(m_tileNdxes[i5] / 4);

        if (tile) {
            unsigned char *rgb = tile->Get_RGB_Data_For_Width(1);
            color->red = (float)rgb[2] / 255.0f;
            color->green = (float)rgb[1] / 255.0f;
            color->blue = (float)rgb[0] / 255.0f;
        }
    }
}

Utf8String WorldHeightMap::Get_Terrain_Name_At(float x, float y)
{
    int i1 = GameMath::Fast_To_Int_Floor(x / 10.0f);
    int i2 = GameMath::Fast_To_Int_Floor(y / 10.0f);
    int i3 = m_borderSize + i1;
    int i4 = m_borderSize + i2;

    if (i3 < 0) {
        i3 = 0;
    }

    if (i4 < 0) {
        i4 = 0;
    }

    if (i3 >= m_width) {
        i3 = m_width - 1;
    }

    if (i4 >= m_height) {
        i4 = m_height - 1;
    }

    int i5 = i3 + m_width * i4;

    if (i5 >= 0 && i5 < m_dataSize) {
        int i6 = m_tileNdxes[i5] / 4;

        for (int i = 0; i < m_numTextureClasses; i++) {
            if (i6 >= m_textureClasses[i].first_tile
                && i6 < m_textureClasses[i].first_tile + m_textureClasses[i].num_tiles) {
                return m_textureClasses[i].name;
            }
        }
    }

    return "";
}

void WorldHeightMap::Setup_Alpha_Tiles()
{
    if (!s_alphaTiles[0]) {
        for (int i = 0; i < 12; i++) {
            bool b[16];
            memset(b, false, sizeof(b));
            int i2 = i;

            if (i >= 6) {
                b[8] = true;
                i2 = i - 6;
            }

            switch (i2) {
                case 0:
                    b[4] = true;
                    break;
                case 1:
                    b[5] = true;
                    break;
                case 2:
                    b[7] = true;
                    break;
                case 3:
                    b[6] = true;
                    break;
                case 4:
                    b[7] = true;
                    b[9] = true;
                    break;
                case 5:
                    b[6] = true;
                    b[9] = true;
                    break;
                default:
                    break;
            }

            s_alphaTiles[i] = new TileData();
            unsigned char *ptr = s_alphaTiles[i]->Get_Data_Ptr();

            for (int j = 0; j < TILE_PIXEL_EXTENT; j++) {
                for (int k = 0; k < TILE_PIXEL_EXTENT; k++) {
                    int i3 = k;
                    int i4 = j;
                    int i5 = 255;

                    if (b[4]) {
                        if (!b[8]) {
                            i3 = TILE_PIXEL_EXTENT - k - 1;
                        }

                        i5 = 255 * i3 / (TILE_PIXEL_EXTENT - 1);
                    } else if (b[5]) {
                        if (!b[8]) {
                            i4 = TILE_PIXEL_EXTENT - j - 1;
                        }

                        i5 = 255 * i4 / (TILE_PIXEL_EXTENT - 1);
                    } else if (b[6]) {
                        if (!b[8]) {
                            i4 = TILE_PIXEL_EXTENT - j - 1;
                        }

                        int i6 = TILE_PIXEL_EXTENT - k - 1 + i4;

                        if (b[9]) {
                            i6 -= TILE_PIXEL_EXTENT;
                        }

                        i5 = 255 * i6 / (TILE_PIXEL_EXTENT - 1);
                    } else if (b[7]) {
                        if (!b[8]) {
                            i4 = TILE_PIXEL_EXTENT - j - 1;
                        }

                        int i7 = k + i4;
                        if (b[9]) {
                            i7 -= TILE_PIXEL_EXTENT;
                        }
                        i5 = 255 * i7 / (TILE_PIXEL_EXTENT - 1);
                    }

                    if (i5 > 255) {
                        i5 = 255;
                    }

                    if (i5 < 0) {
                        i5 = 0;
                    }

                    ptr[3] = 255 - i5;
                    ptr += 4;
                }
            }

            s_alphaTiles[i]->Update_Mips();
        }
    }
}

bool WorldHeightMap::Get_Raw_Tile_Data(short tile_ndx, int width, unsigned char *buffer, int size)
{
    TileData *tile = nullptr;

    if (tile_ndx / 4 < 1024) {
        tile = m_sourceTiles[tile_ndx / 4];
    }

    if (size < 4 * width * width) {
        return 0;
    }

    if (!tile || !tile->Has_RGB_Data_For_Width(2 * width)) {
        return 0;
    }

    unsigned char *rgb = tile->Get_RGB_Data_For_Width(2 * width);
    int i1 = 0;
    int i2 = 0;

    if ((tile_ndx & 1) != 0) {
        i1 = width;
    }

    if ((tile_ndx & 2) != 0) {
        i2 = width;
    }

    for (int i = 0; i < width; i++) {
        // TODO investigate this more?
        memcpy(&buffer[4 * width * i], &rgb[8 * width * (i2 + i) + 4 * i1], 4 * width);
    }

    return true;
}

unsigned char *WorldHeightMap::Get_Pointer_To_Tile_Data(int x, int y, int width)
{
    int tile_ndx = x + m_width * y;

    if (y < 0 || x < 0 || x >= m_width || y >= m_height) {
        return nullptr;
    }

    if (tile_ndx < 0 || tile_ndx >= m_dataSize) {
        return nullptr;
    }

    if (!Get_Raw_Tile_Data(m_tileNdxes[tile_ndx], width, s_buffer, sizeof(s_buffer))) {
        return nullptr;
    }

    int blend_ndx = m_blendTileNdxes[tile_ndx];

    if (blend_ndx > 0 && blend_ndx < 16192
        && Get_Raw_Tile_Data(m_blendedTiles[blend_ndx].blend_ndx, width, s_blendBuffer, sizeof(s_blendBuffer))) {
        unsigned char *alpha = Get_RGB_Alpha_Data_For_Width(width, &m_blendedTiles[blend_ndx]) + 3;
        unsigned char *blendbuf = s_blendBuffer;
        unsigned char *buf = s_buffer;

        for (int i = 0; i < width * width; i++) {
            int i3 = *blendbuf;
            unsigned char *blendbuf2 = blendbuf + 1;
            int i4 = *blendbuf2++;
            int i5 = *blendbuf2;
            blendbuf = blendbuf2 + 2;
            int i6 = *alpha;
            alpha += 4;
            *buf = (255 - i6) * *buf / 255 + i6 * i3 / 255;
            unsigned char *buf2 = buf + 1;
            *buf2 = (255 - i6) * *buf2 / 255 + i6 * i4 / 255;
            buf2++;
            *buf2 = (255 - i6) * *buf2 / 255 + i6 * i5 / 255;
            buf2[1] = 255;
            buf = buf2 + 2;
        }
    }

    return s_buffer;
}

unsigned char *WorldHeightMap::Get_RGB_Alpha_Data_For_Width(int width, TBlendTileInfo *info)
{
    int index = 0;

    if (info->horiz) {
        index = 0;
    } else if (info->vert) {
        index = 1;
    } else if (info->right_diagonal) {
        index = 3;
        if (info->long_diagonal) {
            index += 2;
        }
    } else if (info->left_diagonal) {
        index = 2;
        if (info->long_diagonal) {
            index += 2;
        }
    }

    if (info->inverted) {
        index += 6;
    }

    return s_alphaTiles[index]->Get_RGB_Data_For_Width(width);
}

void WorldHeightMap::Clear_Seismic_Update_Flags()
{
    if (m_seismicUpdateFlag) {
        memset(m_seismicUpdateFlag, 0, m_height * m_seismicUpdateWidth);
    }
}

void WorldHeightMap::Fill_Seismic_Z_Velocities(float value)
{
    if (m_seismicZVelocities) {
        for (int i = 0; i < m_height * m_width; i++) {
            m_seismicZVelocities[i] = value;
        }
    }
}

void WorldHeightMap::Init_Cliff_Flags_From_Heights()
{
    for (int w = 0; w < m_width - 1; ++w) {
        for (int h = 0; h < m_height - 1; ++h) {
            Set_Cell_Cliff_Flag_From_Heights(w, h);
        }
    }
}

int WorldHeightMap::Get_Texture_Class_From_Ndx(int tile_ndx)
{
    int i1 = tile_ndx / 4;

    for (int i = 0; i < m_numTextureClasses; ++i) {
        if (m_textureClasses[i].first_tile >= 0 && i1 >= m_textureClasses[i].first_tile
            && i1 < m_textureClasses[i].first_tile + m_textureClasses[i].num_tiles) {
            return m_textureClasses[i].global_texture_class;
        }
    }

    return -1;
}

void WorldHeightMap::Set_Cliff_State(int x_index, int y_index, bool state)
{
    if (x_index >= 0 && y_index >= 0 && y_index < m_height && x_index < m_width) {
        if (m_cliffState) {
            unsigned char c1 = m_cliffState[(x_index / 8) + m_flipStateWidth * y_index];
            unsigned char c2 = 1 << (x_index & 7);
            unsigned char c3;
            if (state) {
                c3 = c2 | c1;
            } else {
                c3 = ~c2 & c1;
            }
            m_cliffState[(x_index / 8) + m_flipStateWidth * y_index] = c3;
        }
    }
}

bool WorldHeightMap::Get_Seismic_Update_Flag(int x_index, int y_index) const
{
    if (x_index < 0 || y_index < 0) {
        return false;
    }

    if (y_index >= m_height) {
        return false;
    }

    if (x_index >= m_width) {
        return false;
    }

    if (m_seismicUpdateFlag) {
        return ((1 << (x_index & 7)) & m_seismicUpdateFlag[(x_index / 8) + m_seismicUpdateWidth * y_index]) != 0;
    }

    return false;
}

void WorldHeightMap::Set_Seismic_Update_Flag(int x_index, int y_index, bool flag)
{
    if (x_index >= 0 && y_index >= 0 && y_index < m_height && x_index < m_width) {
        if (m_seismicUpdateFlag) {
            unsigned char *c = &m_seismicUpdateFlag[(x_index / 8) + m_seismicUpdateWidth * y_index];
            if (flag) {
                *c |= 1 << (x_index & 7);
            } else {
                *c &= ~(unsigned char)(1 << (x_index & 7));
            }
        }
    }
}

void WorldHeightMap::Get_UV_For_Blend(int edge_class, Region2D *range)
{
    int x = m_edgeTextureClasses[edge_class].position_in_texture.x;
    int y = m_edgeTextureClasses[edge_class].position_in_texture.y;
    int width = m_edgeTextureClasses[edge_class].width;
    range->lo.x = (float)x / 2048.0f;
    range->lo.y = (float)y / (float)m_alphaEdgeHeight;
    range->hi.x = ((float)x + (float)(width * TILE_PIXEL_EXTENT)) / 2048.0f;
    range->hi.y = ((float)y + (float)(width * TILE_PIXEL_EXTENT)) / (float)m_alphaEdgeHeight;
}

TextureClass *WorldHeightMap::Get_Alpha_Terrain_Texture()
{
    if (m_alphaTerrainTex == nullptr) {
        Get_Terrain_Texture();
    }

    return m_alphaTerrainTex;
}

int WorldHeightMap::Get_Texture_Class(int x_index, int y_index, bool base_class)
{
    int i = x_index + m_width * y_index;
    captainslog_dbgassert((i > 0 && i < m_dataSize), "oops");

    if (i < 0 || i >= m_dataSize) {
        return -1;
    }

    if (!base_class && (m_blendTileNdxes[i] || m_extraBlendTileNdxes[i])) {
        return -1;
    }

    return Get_Texture_Class_From_Ndx(m_tileNdxes[i]);
}

TXTextureClass WorldHeightMap::Get_Texture_From_Index(int index)
{
    return m_textureClasses[index];
}
