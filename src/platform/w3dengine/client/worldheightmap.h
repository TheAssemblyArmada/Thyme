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
#ifdef GAME_DLL
#include "hooker.h"
#endif

class TerrainTextureClass;
class AlphaTerrainTextureClass;
class AlphaEdgeTextureClass;
class DataChunkInput;
struct DataChunkInfo;
class ChunkInputStream;
class InputStream;
class TextureClass;
class RGBColor;

class WorldHeightMapInterfaceClass
{
public:
    virtual int Get_Border_Size() = 0;
    virtual float Get_Seismic_Z_Velocity(int x, int y) const = 0;
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
    WorldHeightMap();
    WorldHeightMap(ChunkInputStream *strm, bool logical_data_only);

#ifdef GAME_DLL
    WorldHeightMap *Hook_Ctor() { return new (this) WorldHeightMap(); }
    WorldHeightMap *Hook_Ctor2(ChunkInputStream *strm, bool logical_data_only)
    {
        return new (this) WorldHeightMap(strm, logical_data_only);
    }
    void Hook_Dtor() { WorldHeightMap::~WorldHeightMap(); }
#endif

    virtual ~WorldHeightMap() override;

    virtual int Get_Border_Size() override { return m_borderSize; }
    virtual float Get_Seismic_Z_Velocity(int x, int y) const override;
    virtual void Set_Seismic_Z_Velocity(int x, int y, float velocity) override;
    virtual float Get_Bilinear_Sample_Seismic_Z_Velocity(int x, int y) override;

    bool Get_Cliff_State(int x_index, int y_index) const;
    bool Get_Flip_State(int x, int y) const;
    bool Get_Seismic_Update_Flag(int x_index, int y_index) const;
    void Get_Terrain_Color_At(float x, float y, RGBColor *color);
    Utf8String Get_Terrain_Name_At(float x, float y);

    TextureClass *Get_Alpha_Terrain_Texture();
    TextureClass *Get_Edge_Terrain_Texture();
    TerrainTextureClass *Get_Flat_Texture(int x, int y, int pixels_per_cell, int cell_width);
    TextureClass *Get_Terrain_Texture();
    int Get_Texture_Class(int x_index, int y_index, bool base_class);
    TXTextureClass Get_Texture_From_Index(int index);

    void Get_Alpha_UV_Data(
        int x_index, int y_index, float *const u, float *const v, unsigned char *const alpha, bool *flip, bool full_tile);
    bool Get_Extra_Alpha_UV_Data(
        int x_index, int y_index, float *const u, float *const v, unsigned char *const alpha, bool *need_flip, bool *cliff);
    bool Get_UV_Data(int x_index, int y_index, float *const u, float *const v, bool full_tile);
    void Get_UV_For_Blend(int edge_class, Region2D *range);

    void Clear_Flip_States();
    bool Is_Cliff_Mapped_Texture(int x, int y);

    bool Set_Draw_Origin(int x_origin, int y_origin);
    void Set_Flip_State(int x_index, int y_index, bool state);
    void Set_Seismic_Update_Flag(int x_index, int y_index, bool flag);
    void Set_Texture_LOD(int lod);

    static int Get_Min_Height_Value() { return 0; }
    static int Get_Max_Height_Value() { return 255; }
    int Get_X_Extent() { return m_width; }
    int Get_Y_Extent() { return m_height; }
    int Border_Size() { return m_borderSize; }
    int Get_Draw_Width() { return m_drawWidthX; }
    int Get_Draw_Height() { return m_drawHeightY; }
    int Get_Draw_Origin_X() { return m_drawOriginX; }
    int Get_Draw_Origin_Y() { return m_drawOriginY; }
    unsigned char *Get_Data_Ptr() { return m_data; }

    void Set_Draw_Width(int width)
    {
        m_drawWidthX = width;

        if (m_drawWidthX > m_width) {
            m_drawWidthX = m_width;
        }
    }

    void Set_Draw_Height(int height)
    {
        m_drawHeightY = height;

        if (m_drawHeightY > m_height) {
            m_drawHeightY = m_height;
        }
    }

    bool Is_Cell_Flipped(int x, int y) { return ((1 << (x & 7)) & m_cellFlipState[(x >> 3) + m_flipStateWidth * y]) != 0; }

    const std::vector<ICoord2D> &Get_All_Boundaries() const { return m_boundaries; }

    unsigned char Get_Display_Height(int x, int y) const
    {
        return m_data[(m_drawOriginY + y) * m_width + m_drawOriginX + x];
    }

    unsigned char Get_Height(int x, int y) const
    {
        int i = x + m_width * y;

        if (i >= 0 && i < m_dataSize && m_data != nullptr) {
            return m_data[i];
        } else {
            return 0;
        }
    }

    void Set_Height(int x, int y, unsigned char height) const
    {
        int i = x + m_width * y;

        if (i >= 0 && i < m_dataSize && m_data != nullptr) {
            m_data[i] = height;
        }
    }

#ifdef GAME_DLL
    static ARRAY_DEC(TileData *, s_alphaTiles, 12);
#else
    static TileData *s_alphaTiles[12];
#endif

protected:
    enum
    {
        TEXTURE_COUNT = 256,
        TILE_COUNT = 1024,
        BLEND_TILE_COUNT = 16192,
        CLIFF_COUNT = 32384,
        TILE_PIXEL_EXTENT = 64,
        FLAG_VAL = 0x7ADA0000,
    };

    TileData *Get_Source_Tile(unsigned int tile) const
    {
        if (tile >= TILE_COUNT) {
            return nullptr;
        } else {
            return m_sourceTiles[tile];
        }
    }

    TileData *Get_Edge_Tile(unsigned int tile) const
    {
        if (tile >= TILE_COUNT) {
            return nullptr;
        } else {
            return m_edgeTiles[tile];
        }
    }

    unsigned char *Get_Pointer_To_Tile_Data(int x, int y, int width);
    void Get_UV_For_Ndx(int tile_ndx, float *min_u, float *max_u, float *min_v, float *max_v, bool full_tile);
    bool Get_UV_For_Tile_Index(int ndx, short tile_ndx, float *const v, float *const u, bool full_tile);
    int Get_Texture_Class_From_Ndx(int tile_ndx);
    unsigned char *Get_RGB_Alpha_Data_For_Width(int width, TBlendTileInfo *info);
    bool Get_Raw_Tile_Data(short tile_ndx, int width, unsigned char *buffer, int size);

    void Set_Cell_Cliff_Flag_From_Heights(int x_index, int y_index);
    void Set_Cliff_State(int x_index, int y_index, bool state);

    void Clear_Seismic_Update_Flags();
    int Update_Tile_Texture_Positions(int *edge_height);
    void Fill_Seismic_Z_Velocities(float value);
    void Init_Cliff_Flags_From_Heights();
    void Setup_Alpha_Tiles();

    bool Parse_Blend_Tile_Data(DataChunkInput &file, DataChunkInfo *info, void *user_data);
    bool Parse_Height_Map_Data(DataChunkInput &file, DataChunkInfo *info, void *user_data);
    bool Parse_Object_Data(DataChunkInput &file, DataChunkInfo *info, void *user_data, bool read_dict);
    bool Parse_Size_Only(DataChunkInput &file, DataChunkInfo *info, void *user_data);

    static bool Parse_Blend_Tile_Data_Chunk(DataChunkInput &file, DataChunkInfo *info, void *user_data);
    static bool Parse_Height_Map_Data_Chunk(DataChunkInput &file, DataChunkInfo *info, void *user_data);
    static bool Parse_Lighting_Data_Chunk(DataChunkInput &file, DataChunkInfo *info, void *user_data);
    static bool Parse_Object_Data_Chunk(DataChunkInput &file, DataChunkInfo *info, void *user_data);
    static bool Parse_Objects_Data_Chunk(DataChunkInput &file, DataChunkInfo *info, void *user_data);
    static bool Parse_Size_Only_In_Chunk(DataChunkInput &file, DataChunkInfo *info, void *user_data);
    static bool Parse_World_Dict_Data_Chunk(DataChunkInput &file, DataChunkInfo *info, void *user_data);

    static int Count_Tiles(InputStream *str, bool *b);
    static void Free_List_Of_Map_Objects();
    static bool Read_Tiles(InputStream *str, TileData **tiles, int num_rows);
    void Read_Tex_Class(TXTextureClass *tex_class, TileData **tile_data);

    int m_width;
    int m_height;
    int m_borderSize;
    std::vector<ICoord2D> m_boundaries;
    int m_dataSize;
    unsigned char *m_data;
    unsigned char *m_seismicUpdateFlag;
    int m_seismicUpdateWidth;
    float *m_seismicZVelocities;
    unsigned char *m_cellFlipState;
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
    friend class W3DTreeBuffer;
    friend class W3DTerrainLogic;
};

extern const float HEIGHTMAP_SCALE;
