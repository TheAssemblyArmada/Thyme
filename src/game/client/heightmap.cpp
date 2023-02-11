/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Height Map Render Object
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "heightmap.h"

#ifndef GAME_DLL
HeightMapRenderObjClass *g_theHeightMap;
#endif

HeightMapRenderObjClass::HeightMapRenderObjClass() :
    m_extraBlendTilePositions(nullptr),
    m_numExtraBlendTiles(0),
    m_extraBlendTilesRendered(0),
    m_extraBlendTilePositionsSize(0),
    m_vertexBufferTiles(nullptr),
    m_vertexBufferBackup(nullptr),
    m_originX(0),
    m_originY(0),
    m_indexBuffer(nullptr),
    m_numVBTilesX(0),
    m_numVBTilesY(0),
    m_numVertexBufferTiles(0),
    m_numBlockColumnsInLastVB(0),
    m_numBlockRowsInLastVB(0)
{
    g_theHeightMap = this;
}

HeightMapRenderObjClass::~HeightMapRenderObjClass()
{
    Free_Map_Resources();

    delete[] m_extraBlendTilePositions;
    m_extraBlendTilePositions = nullptr;
}

void HeightMapRenderObjClass::Render(RenderInfoClass &rinfo)
{
    // heightmap TODO
#ifdef GAME_DLL
    Call_Method<void, HeightMapRenderObjClass, RenderInfoClass &>(PICK_ADDRESS(0x0079D6D0, 0x0060BC63), this, rinfo);
#endif
}

void HeightMapRenderObjClass::On_Frame_Update()
{
    // heightmap TODO
#ifdef GAME_DLL
    Call_Method<void, HeightMapRenderObjClass>(PICK_ADDRESS(0x0079C510, 0x00609F43), this);
#endif
}

int HeightMapRenderObjClass::Init_Height_Data(
    int x, int y, WorldHeightMap *map, RefMultiListIterator<RenderObjClass> *lights, bool update_shoreline)
{
    // heightmap TODO
#ifdef GAME_DLL
    return Call_Method<int,
        HeightMapRenderObjClass,
        int,
        int,
        WorldHeightMap *,
        RefMultiListIterator<RenderObjClass> *,
        bool>(PICK_ADDRESS(0x0079C0B0, 0x00609878), this, x, y, map, lights, update_shoreline);
#else
    return 0;
#endif
}

int HeightMapRenderObjClass::Free_Map_Resources()
{
    BaseHeightMapRenderObjClass::Free_Map_Resources();
    Free_Index_Vertex_Buffers();
    return 0;
}

void HeightMapRenderObjClass::Update_Center(CameraClass *camera, RefMultiListIterator<RenderObjClass> *lights)
{
    // heightmap TODO
#ifdef GAME_DLL
    Call_Method<void, HeightMapRenderObjClass, CameraClass *, RefMultiListIterator<RenderObjClass> *>(
        PICK_ADDRESS(0x0079CA70, 0x0060A87C), this, camera, lights);
#endif
}

void HeightMapRenderObjClass::Adjust_Terrain_LOD(int adj)
{
    BaseHeightMapRenderObjClass::Adjust_Terrain_LOD(adj);
}

void HeightMapRenderObjClass::Do_Partial_Update(
    IRegion2D &partial_range, WorldHeightMap *map, RefMultiListIterator<RenderObjClass> *lights)
{
    // heightmap TODO
#ifdef GAME_DLL
    Call_Method<void, HeightMapRenderObjClass, IRegion2D, WorldHeightMap *, RefMultiListIterator<RenderObjClass> *>(
        PICK_ADDRESS(0x0079B960, 0x00608DD3), this, partial_range, map, lights);
#endif
}

void HeightMapRenderObjClass::Static_Lighting_Changed()
{
    BaseHeightMapRenderObjClass::Static_Lighting_Changed();
}

void HeightMapRenderObjClass::Oversize_Terrain(int tiles_to_oversize)
{
    // heightmap TODO
#ifdef GAME_DLL
    Call_Method<void, HeightMapRenderObjClass, int>(PICK_ADDRESS(0x0079BFA0, 0x006096B5), this, tiles_to_oversize);
#endif
}

void HeightMapRenderObjClass::Reset()
{
    BaseHeightMapRenderObjClass::Reset();
}

int HeightMapRenderObjClass::Update_Block(
    int x0, int y0, int x, int y, WorldHeightMap *map, RefMultiListIterator<RenderObjClass> *lights)
{
    // heightmap TODO
#ifdef GAME_DLL
    return Call_Method<int,
        HeightMapRenderObjClass,
        int,
        int,
        int,
        int,
        WorldHeightMap *,
        RefMultiListIterator<RenderObjClass> *>(PICK_ADDRESS(0x0079BCA0, 0x006092B0), this, x0, y0, x, y, map, lights);
#else
    return 0;
#endif
}

void HeightMapRenderObjClass::Release_Resources()
{
    BaseHeightMapRenderObjClass::Release_Resources();
}

void HeightMapRenderObjClass::Re_Acquire_Resources()
{
    BaseHeightMapRenderObjClass::Re_Acquire_Resources();
}

void HeightMapRenderObjClass::Free_Index_Vertex_Buffers()
{
    Ref_Ptr_Release(m_indexBuffer);

    if (m_vertexBufferTiles != nullptr) {
        for (int i = 0; i < m_numVertexBufferTiles; i++) {
            Ref_Ptr_Release(m_vertexBufferTiles[i]);
        }

        delete[] m_vertexBufferTiles;
        m_vertexBufferTiles = 0;
    }

    if (m_vertexBufferBackup != nullptr) {
        for (int i = 0; i < m_numVertexBufferTiles; i++) {
            delete[] m_vertexBufferBackup[i];
        }

        delete[] m_vertexBufferBackup;
        m_vertexBufferBackup = nullptr;
    }

    m_numVertexBufferTiles = 0;
}

void HeightMapRenderObjClass::Do_The_Dynamic_Light(VertexFormatXYZDUV2 *vb,
    VertexFormatXYZDUV2 *vb_mirror,
    Vector3 *light,
    Vector3 *normal,
    W3DDynamicLight *lights,
    int num_lights)
{
    // heightmap TODO
#ifdef GAME_DLL
    Call_Method<void,
        HeightMapRenderObjClass,
        VertexFormatXYZDUV2 *,
        VertexFormatXYZDUV2 *,
        Vector3 *,
        Vector3 *,
        W3DDynamicLight *,
        int>(PICK_ADDRESS(0x00799150, 0x00605D83), this, vb, vb_mirror, light, normal, lights, num_lights);
#endif
}

int HeightMapRenderObjClass::Get_X_With_Origin(int x)
{
    int ret = x - m_originX;

    if (ret < 0) {
        ret = ret + m_x - 1;
    }

    if (ret >= m_x - 1) {
        ret -= m_x - 1;
    }

    if (ret < 0) {
        ret = 0;
    }

    if (ret >= m_x - 1) {
        return m_x - 1;
    }

    return ret;
}

int HeightMapRenderObjClass::Get_Y_With_Origin(int y)
{
    int ret = y - m_originY;

    if (ret < 0) {
        ret = ret + m_y - 1;
    }

    if (ret >= m_y - 1) {
        ret -= m_y - 1;
    }

    if (ret < 0) {
        ret = 0;
    }

    if (ret >= m_y - 1) {
        return m_y - 1;
    }

    return ret;
}

int HeightMapRenderObjClass::Update_VB(DX8VertexBufferClass *vb,
    char *data,
    int x0,
    int y0,
    int x1,
    int y1,
    int origin_x,
    int origin_y,
    WorldHeightMap *map,
    RefMultiListIterator<RenderObjClass> *lights)
{
    // heightmap TODO
#ifdef GAME_DLL
    return Call_Method<int,
        HeightMapRenderObjClass,
        DX8VertexBufferClass *,
        char *,
        int,
        int,
        int,
        int,
        int,
        int,
        WorldHeightMap *,
        RefMultiListIterator<RenderObjClass> *>(
        PICK_ADDRESS(0x00799610, 0x0060622E), this, vb, data, x0, y0, x1, y1, origin_x, origin_y, map, lights);
#else
    return 0;
#endif
}

int HeightMapRenderObjClass::Update_VB_For_Light(DX8VertexBufferClass *vb,
    char *data,
    int x0,
    int y0,
    int x1,
    int y1,
    int origin_x,
    int origin_y,
    W3DDynamicLight **lights,
    int num_lights)
{
    // heightmap TODO
#ifdef GAME_DLL
    return Call_Method<int,
        HeightMapRenderObjClass,
        DX8VertexBufferClass *,
        char *,
        int,
        int,
        int,
        int,
        int,
        int,
        W3DDynamicLight **,
        int>(PICK_ADDRESS(0x0079ACA0, 0x00607629), this, vb, data, x0, y0, x1, y1, origin_x, origin_y, lights, num_lights);
#else
    return 0;
#endif
}

int HeightMapRenderObjClass::Update_VB_For_Light_Optimized(DX8VertexBufferClass *vb,
    char *data,
    int x0,
    int y0,
    int x1,
    int y1,
    int origin_x,
    int origin_y,
    W3DDynamicLight **lights,
    int num_lights)
{
    // heightmap TODO
#ifdef GAME_DLL
    return Call_Method<int,
        HeightMapRenderObjClass,
        DX8VertexBufferClass *,
        char *,
        int,
        int,
        int,
        int,
        int,
        int,
        W3DDynamicLight **,
        int>(PICK_ADDRESS(0x0, 0x006080DB),
        this,
        vb,
        data,
        x0,
        y0,
        x1,
        y1,
        origin_x,
        origin_y,
        lights,
        num_lights); // does not exist in game exe
#else
    return 0;
#endif
}

void HeightMapRenderObjClass::Render_Terrain_Pass(CameraClass *camera)
{
#ifdef BUILD_WITH_D3D8
    Matrix3D tm(true);
    DX8Wrapper::Set_Transform(D3DTS_WORLD, tm);
    DX8Wrapper::Set_Index_Buffer(m_indexBuffer, 0);
    static int count = 0;

    for (int i = 0; i < m_numVBTilesY; i++) {
        for (int j = 0; j < m_numVBTilesX; j++) {
            count++;
            DX8Wrapper::Set_Vertex_Buffer(m_vertexBufferTiles[j + m_numVBTilesX * i], 0);

            if (!Is_Hidden()) {
                DX8Wrapper::Draw_Triangles(0, 2048, 0, 4096);
            }
        }
    }
#endif
}

void HeightMapRenderObjClass::Render_Extra_Blend_Tiles()
{
    // heightmap TODO
#ifdef GAME_DLL
    Call_Method<void, HeightMapRenderObjClass>(PICK_ADDRESS(0x0079E250, 0x0060C5A4), this);
#endif
}
