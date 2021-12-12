/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief World Height Map
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
#include "coord.h"
#include "refcount.h"
#include "tiledata.h"
#include <vector>

class TerrainTextureClass;
class AlphaTerrainTextureClass;
class AlphaEdgeTextureClass;

class WorldHeightMapInterfaceClass
{
public:
    virtual int Get_Border_Size() = 0;
    virtual float Get_Seismic_Z_Velocity(int x, int y) = 0;
    virtual void Set_Seismic_Z_Velocity(int x, int y, float velocity) = 0;
    virtual float Get_Bilinear_Sample_Seismic_Z_Velocity(int x, int y) = 0;
};

struct TBlendTileInfo
{
    int blend_ndx;
    char horiz;
    char vert;
    char right_diagonal;
    char left_diagonal;
    char inverted;
    char long_diagonal;
    int custom_blend_edge_class;
};

struct TCliffInfo
{
    float u0;
    float v0;
    float u1;
    float v1;
    float u2;
    float v2;
    float u3;
    float v3;
    bool flip;
    bool mutant;
    short tile_index;
};

struct TXTextureClass
{
    int global_texture_class;
    int first_tile;
    int num_tiles;
    int width;
    int is_blend_edge_tile;
    Utf8String name;
    ICoord2D position_in_texture;
};

class WorldHeightMap : public RefCountClass, public WorldHeightMapInterfaceClass
{
    friend class TerrainTextureClass;
    friend class AlphaEdgeTextureClass;

public:
    virtual ~WorldHeightMap();
    virtual int Get_Border_Size();
    virtual float Get_Seismic_Z_Velocity(int x, int y);
    virtual void Set_Seismic_Z_Velocity(int x, int y, float velocity);
    virtual float Get_Bilinear_Sample_Seismic_Z_Velocity(int x, int y);

    bool Get_Flip_State(int x, int y);

    static int Get_Min_Height_Value() { return 0; }
    static int Get_Max_Height_Value() { return 255; }
    int Get_Width() { return m_width; }
    int Get_Height() { return m_height; }
    int Border_Size() { return m_borderSize; }
    int Get_Draw_Width() { return m_drawWidthX; }
    int Get_Draw_Height() { return m_drawHeightY; }
    int Get_Draw_Origin_X() { return m_drawOriginX; }
    int Get_Draw_Origin_Y() { return m_drawOriginY; }

    unsigned char Get_Height(int x, int y)
    {
        int i = x + this->m_width * y;

        if (i >= 0 && i < m_dataSize && m_data) {
            return m_data[i];
        } else {
            return 0;
        }
    }

protected:
    enum
    {
        TEXTURE_COUNT = 256,
        TILE_COUNT = 1024,
        BLEND_TILE_COUNT = 16192,
        CLIFF_COUNT = 32384,
    };

    TileData *Get_Source_Tile(int tile)
    {
        if (tile >= TILE_COUNT) {
            return nullptr;
        } else {
            return m_sourceTiles[tile];
        }
    }

    TileData *Get_Edge_Tile(int tile)
    {
        if (tile >= TILE_COUNT) {
            return nullptr;
        } else {
            return m_edgeTiles[tile];
        }
    }

    unsigned char *Get_Pointer_To_Tile_Data(int x, int y, int width);

    int m_width;
    int m_height;
    int m_borderSize;
    std::vector<ICoord2D> m_boundaries;
    int m_dataSize;
    unsigned char *m_data;
    unsigned char *m_siesmicUpdateFlags;
    int m_siesmicUpdateWidth;
    float *m_siesmicZVelocity;
    char *m_cellFlipState;
    int m_flipStateWidth;
    unsigned char *m_cliffState;
    short *m_tileNdxes;
    short *m_blendTileNdxes;
    short *m_cliffInfoNdxes;
    short *m_extraBlendTileNdxes;
    int m_numBitmapTiles;
    int m_numEdgeTiles;
    int m_numBlendedTiles;
    TileData *m_sourceTiles[TILE_COUNT];
    TileData *m_edgeTiles[TILE_COUNT];
    TBlendTileInfo m_blendedTiles[BLEND_TILE_COUNT];
    TBlendTileInfo m_extraBlendedTiles[BLEND_TILE_COUNT];
    TCliffInfo m_cliffInfo[CLIFF_COUNT];
    int m_numCliffInfo;
    int m_numTextureClasses;
    TXTextureClass m_textureClasses[TEXTURE_COUNT];
    int m_numEdgeTextureClasses;
    TXTextureClass m_edgeTextureClasses[TEXTURE_COUNT];
    TerrainTextureClass *m_terrainTex;
    int m_terrainTexHeight;
    AlphaTerrainTextureClass *m_alphaTerrainTex;
    int m_alphaTexHeight;
    AlphaEdgeTextureClass *m_alphaEdgeTex;
    int m_alphaEdgeHeight;
    int m_drawOriginX;
    int m_drawOriginY;
    int m_drawWidthX;
    int m_drawHeightY;
};
