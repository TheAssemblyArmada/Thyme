/**
 * @file
 *
 * @author tomsons26
 *
 * @brief W3D Terrain Background
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
#include "aabox.h"
#include <new>

class ICoord2D;
class IndexBufferClass;
class VertexBufferClass;
class TextureClass;
class TerrainTextureClass;
class IRegion2D;
class RenderInfoClass;
class CameraClass;
class WorldHeightMap;

class W3DTerrainBackground
{
public:
    enum CullResult
    {
        CULL_RES_INVALID,
        CULL_RES_INSIDE,
        CULL_RES_OUTSIDE,
    };

    W3DTerrainBackground();
    ~W3DTerrainBackground();

    void Allocate_Terrain_Buffers(WorldHeightMap *map, int x, int y, int pixels_per_cell);
    void Free_Terrain_Buffers();

    void Update_Center(CameraClass *camera);
    void Update_Texture();

    void Draw_Visible_Polys(RenderInfoClass *rinfo, bool keep_textures);

    void Do_Partial_Update(IRegion2D const &region, WorldHeightMap *map, bool set_textures);

    void Set_Flip(WorldHeightMap *map);

    bool Is_Culled_Out() { return m_cullResult == CULL_RES_OUTSIDE; }
    int Get_State() { return m_state; }

private:
    bool Advance_Left(ICoord2D &pos, int x, int y, int pixels_per_cell);
    bool Advance_Right(ICoord2D &pos, int x, int y, int pixels_per_cell);

    void Fill_VB_Recursive(
        unsigned short *out_buffer, int x, int y, int pixels_per_cell, unsigned short *in_buffer, int &count);

    void Set_Flip_Recursive(int x, int y, int pixels_per_cell);

    void Do_Tesselated_Update(IRegion2D const &region, WorldHeightMap *map, bool set_textures);

#ifdef GAME_DLL
    W3DTerrainBackground *Hook_Ctor() { return ::new (this) W3DTerrainBackground(); }
    void Hook_Dtor() { W3DTerrainBackground::~W3DTerrainBackground(); }
#endif

private:
    CullResult m_cullResult;
    AABoxClass m_boundingBox;
    VertexBufferClass *m_vertexBuffer;
    int m_maxVertex;
    IndexBufferClass *m_indexBuffer;
    int m_maxIndex;
    TerrainTextureClass *m_texture8;
    TerrainTextureClass *m_texture16;
    TerrainTextureClass *m_texture32;
    int m_state;
    int m_curNumTerrainVertices;
    int m_curNumTerrainIndices;
    int m_x;
    int m_y;
    int m_pixelsPerCell;
    WorldHeightMap *m_map;
    bool m_unk1;
    bool m_allocated;
};
