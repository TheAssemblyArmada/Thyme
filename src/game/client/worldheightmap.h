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
    virtual int32_t Get_Border_Size() = 0;
    virtual float Get_Seismic_Z_Velocity(int32_t x, int32_t y) const = 0;
    virtual void Set_Seismic_Z_Velocity(int32_t x, int32_t y, float velocity) = 0;
    virtual float Get_Bilinear_Sample_Seismic_Z_Velocity(int32_t x, int32_t y) = 0;
};

struct TBlendTileInfo
{
    int32_t blend_ndx;
    char horiz;
    char vert;
    char right_diagonal;
    char left_diagonal;
    char inverted;
    char long_diagonal;
    int32_t custom_blend_edge_class;
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
    int32_t global_texture_class;
    int32_t first_tile;
    int32_t num_tiles;
    int32_t width;
    int32_t is_blend_edge_tile;
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

    virtual int32_t Get_Border_Size() override { return m_borderSize; }
    virtual float Get_Seismic_Z_Velocity(int32_t x, int32_t y) const override;
    virtual void Set_Seismic_Z_Velocity(int32_t x, int32_t y, float velocity) override;
    virtual float Get_Bilinear_Sample_Seismic_Z_Velocity(int32_t x, int32_t y) override;

    bool Get_Cliff_State(int32_t x_index, int32_t y_index) const;
    bool Get_Flip_State(int32_t x, int32_t y) const;
    bool Get_Seismic_Update_Flag(int32_t x_index, int32_t y_index) const;
    void Get_Terrain_Color_At(float x, float y, RGBColor *color);
    Utf8String Get_Terrain_Name_At(float x, float y);

    TextureClass *Get_Alpha_Terrain_Texture();
    TextureClass *Get_Edge_Terrain_Texture();
    TextureClass *Get_Flat_Texture(int32_t x, int32_t y, int32_t pixels_per_cell, int32_t cell_width);
    TextureClass *Get_Terrain_Texture();
    int32_t Get_Texture_Class(int32_t x_index, int32_t y_index, bool base_class);
    TXTextureClass Get_Texture_From_Index(int32_t index);

    void Get_Alpha_UV_Data(
        int32_t x_index, int32_t y_index, float *const u, float *const v, unsigned char *const alpha, bool *flip, bool full_tile);
    bool Get_Extra_Alpha_UV_Data(
        int32_t x_index, int32_t y_index, float *const u, float *const v, unsigned char *const alpha, bool *need_flip, bool *cliff);
    bool Get_UV_Data(int32_t x_index, int32_t y_index, float *const u, float *const v, bool full_tile);
    void Get_UV_For_Blend(int32_t edge_class, Region2D *range);

    void Clear_Flip_States();
    bool Is_Cliff_Mapped_Texture(int32_t x, int32_t y);

    bool Set_Draw_Origin(int32_t x_origin, int32_t y_origin);
    void Set_Flip_State(int32_t x_index, int32_t y_index, bool state);
    void Set_Seismic_Update_Flag(int32_t x_index, int32_t y_index, bool flag);
    void Set_Texture_LOD(int32_t lod);

    static int32_t Get_Min_Height_Value() { return 0; }
    static int32_t Get_Max_Height_Value() { return 255; }
    int32_t Get_X_Extent() { return m_width; }
    int32_t Get_Y_Extent() { return m_height; }
    int32_t Border_Size() { return m_borderSize; }
    int32_t Get_Draw_Width() { return m_drawWidthX; }
    int32_t Get_Draw_Height() { return m_drawHeightY; }
    int32_t Get_Draw_Origin_X() { return m_drawOriginX; }
    int32_t Get_Draw_Origin_Y() { return m_drawOriginY; }
    unsigned char *Get_Data_Ptr() { return m_data; }

    void Set_Draw_Width(int32_t width)
    {
        m_drawWidthX = width;

        if (m_drawWidthX > m_width) {
            m_drawWidthX = m_width;
        }
    }

    void Set_Draw_Height(int32_t height)
    {
        m_drawHeightY = height;

        if (m_drawHeightY > m_height) {
            m_drawHeightY = m_height;
        }
    }

    const std::vector<ICoord2D> &Get_All_Boundaries() const { return m_boundaries; }

    unsigned char Get_Display_Height(int32_t x, int32_t y) const
    {
        return m_data[(m_drawOriginY + y) * m_width + m_drawOriginX + x];
    }

    unsigned char Get_Height(int32_t x, int32_t y) const
    {
        int32_t i = x + this->m_width * y;

        if (i >= 0 && i < m_dataSize && m_data) {
            return m_data[i];
        } else {
            return 0;
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

    TileData *Get_Source_Tile(uint32_t tile) const
    {
        if (tile >= TILE_COUNT) {
            return nullptr;
        } else {
            return m_sourceTiles[tile];
        }
    }

    TileData *Get_Edge_Tile(uint32_t tile) const
    {
        if (tile >= TILE_COUNT) {
            return nullptr;
        } else {
            return m_edgeTiles[tile];
        }
    }

    unsigned char *Get_Pointer_To_Tile_Data(int32_t x, int32_t y, int32_t width);
    void Get_UV_For_Ndx(int32_t tile_ndx, float *min_u, float *max_u, float *min_v, float *max_v, bool full_tile);
    bool Get_UV_For_Tile_Index(int32_t ndx, short tile_ndx, float *const v, float *const u, bool full_tile);
    int32_t Get_Texture_Class_From_Ndx(int32_t tile_ndx);
    unsigned char *Get_RGB_Alpha_Data_For_Width(int32_t width, TBlendTileInfo *info);
    bool Get_Raw_Tile_Data(short tile_ndx, int32_t width, unsigned char *buffer, int32_t size);

    void Set_Cell_Cliff_Flag_From_Heights(int32_t x_index, int32_t y_index);
    void Set_Cliff_State(int32_t x_index, int32_t y_index, bool state);

    void Clear_Seismic_Update_Flags();
    int32_t Update_Tile_Texture_Positions(int32_t *edge_height);
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

    static int32_t Count_Tiles(InputStream *str, bool *b);
    static void Free_List_Of_Map_Objects();
    static bool Read_Tiles(InputStream *str, TileData **tiles, int32_t num_rows);
    void Read_Tex_Class(TXTextureClass *tex_class, TileData **tile_data);

    int32_t m_width;
    int32_t m_height;
    int32_t m_borderSize;
    std::vector<ICoord2D> m_boundaries;
    int32_t m_dataSize;
    unsigned char *m_data;
    unsigned char *m_seismicUpdateFlag;
    int32_t m_seismicUpdateWidth;
    float *m_seismicZVelocities;
    unsigned char *m_cellFlipState;
    int32_t m_flipStateWidth;
    unsigned char *m_cliffState;
    short *m_tileNdxes;
    short *m_blendTileNdxes;
    short *m_cliffInfoNdxes;
    short *m_extraBlendTileNdxes;
    int32_t m_numBitmapTiles;
    int32_t m_numEdgeTiles;
    int32_t m_numBlendedTiles;
    TileData *m_sourceTiles[TILE_COUNT];
    TileData *m_edgeTiles[TILE_COUNT];
    TBlendTileInfo m_blendedTiles[BLEND_TILE_COUNT];
    TBlendTileInfo m_extraBlendedTiles[BLEND_TILE_COUNT];
    TCliffInfo m_cliffInfo[CLIFF_COUNT];
    int32_t m_numCliffInfo;
    int32_t m_numTextureClasses;
    TXTextureClass m_textureClasses[TEXTURE_COUNT];
    int32_t m_numEdgeTextureClasses;
    TXTextureClass m_edgeTextureClasses[TEXTURE_COUNT];
    TerrainTextureClass *m_terrainTex;
    int32_t m_terrainTexHeight;
    AlphaTerrainTextureClass *m_alphaTerrainTex;
    int32_t m_alphaTexHeight;
    AlphaEdgeTextureClass *m_alphaEdgeTex;
    int32_t m_alphaEdgeHeight;
    int32_t m_drawOriginX;
    int32_t m_drawOriginY;
    int32_t m_drawWidthX;
    int32_t m_drawHeightY;
};
