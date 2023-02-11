/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Flat Height Map Render Object
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "flatheightmap.h"
#include "w3dterrainbackground.h"

#ifndef GAME_DLL
FlatHeightMapRenderObjClass *g_theFlatHeightMap;
#endif

FlatHeightMapRenderObjClass::FlatHeightMapRenderObjClass() :
    m_tiles(nullptr), m_tileCount(0), m_tileWidth(0), m_tileHeight(0), m_state(0)
{
    g_theFlatHeightMap = this;
}

FlatHeightMapRenderObjClass::~FlatHeightMapRenderObjClass()
{
    Release_Tiles();
    g_theFlatHeightMap = nullptr;
}

void FlatHeightMapRenderObjClass::Render(RenderInfoClass &rinfo)
{
    // heightmap TODO
#ifdef GAME_DLL
    Call_Method<void, FlatHeightMapRenderObjClass, RenderInfoClass &>(PICK_ADDRESS(0x00798740, 0x0066B759), this, rinfo);
#endif
}

void FlatHeightMapRenderObjClass::On_Frame_Update()
{
    BaseHeightMapRenderObjClass::On_Frame_Update();

    if (m_state != 0) {
        if (m_state == 1) {
            m_state = 2;
        } else if (m_state == 2) {
            for (int i = 0; i < m_tileWidth; i++) {
                for (int j = 0; j < m_tileHeight; j++) {
                    m_tiles[m_tileWidth * j + i].Update_Texture();
                }
            }
        }
    }
}

int FlatHeightMapRenderObjClass::Init_Height_Data(
    int x, int y, WorldHeightMap *map, RefMultiListIterator<RenderObjClass> *lights, bool update_shoreline)
{
    BaseHeightMapRenderObjClass::Init_Height_Data(x, y, map, lights, true);
    int width = (map->Get_X_Extent() + 14) / 16;
    int height = (map->Get_Y_Extent() + 14) / 16;
    int size = width * height;
    map->Clear_Flip_States();

    if (m_tiles != nullptr && m_tileWidth == width && m_tileHeight == height) {
        for (int i = 0; i < m_tileWidth; i++) {
            for (int j = 0; j < m_tileHeight; j++) {
                m_tiles[m_tileWidth * j + i].Set_Flip(map);
            }
        }
    } else {
        Release_Tiles();
        m_tiles = new W3DTerrainBackground[size];
        m_tileCount = size;
        m_tileWidth = width;
        m_tileHeight = height;

        for (int i = 0; i < m_tileWidth; i++) {
            for (int j = 0; j < m_tileHeight; j++) {
                m_tiles[m_tileWidth * j + i].Allocate_Terrain_Buffers(map, 16 * i, 16 * j, 16);
                m_tiles[m_tileWidth * j + i].Set_Flip(map);
            }
        }
    }

    IRegion2D region;
    region.lo.x = 0;
    region.lo.y = 0;
    region.hi.x = map->Get_X_Extent();
    region.hi.y = map->Get_Y_Extent();

    for (int i = 0; i < m_tileWidth; i++) {
        for (int j = 0; j < m_tileHeight; j++) {
            m_tiles[m_tileWidth * j + i].Do_Partial_Update(region, map, true);
        }
    }

    return 0;
}

int FlatHeightMapRenderObjClass::Free_Map_Resources()
{
    BaseHeightMapRenderObjClass::Free_Map_Resources();
    return 0;
}

void FlatHeightMapRenderObjClass::Update_Center(CameraClass *camera, RefMultiListIterator<RenderObjClass> *lights)
{
    static int prevCulled = 0;
    static int prevT2X = 0;
    static int prevT4X = 0;

    BaseHeightMapRenderObjClass::Update_Center(camera, lights);
    m_needFullUpdate = false;
    int culled = 0;
    int t2x = 0;
    int t4x = 0;

    for (int i = 0; i < m_tileWidth; i++) {
        for (int j = 0; j < m_tileHeight; j++) {
            m_tiles[m_tileWidth * j + i].Update_Center(camera);

            if (m_tiles[m_tileWidth * j + i].Is_Culled_Out()) {
                culled++;
            }

            int state = m_tiles[m_tileWidth * j + i].Get_State();

            if (state == 4) {
                t4x++;
            } else if (state == 2) {
                t2x++;
            }
        }
    }

    if (culled != prevCulled || t4x != prevT4X || t2x != prevT2X) {
        captainslog_debug("%d of %d culled, %d 4X, %d 2X.", culled, m_tileCount, t4x, t2x);
        prevCulled = culled;
        prevT2X = t2x;
        prevT4X = t4x;
    }

    m_state = 1;
}

void FlatHeightMapRenderObjClass::Adjust_Terrain_LOD(int adj)
{
    BaseHeightMapRenderObjClass::Adjust_Terrain_LOD(adj);
}

void FlatHeightMapRenderObjClass::Do_Partial_Update(
    IRegion2D &partial_range, WorldHeightMap *map, RefMultiListIterator<RenderObjClass> *lights)
{
    Ref_Ptr_Set(m_map, map);

    for (int i = 0; i < m_tileWidth; i++) {
        for (int j = 0; j < m_tileHeight; j++) {
            m_tiles[m_tileWidth * j + i].Do_Partial_Update(partial_range, map, true);
        }
    }
}

void FlatHeightMapRenderObjClass::Static_Lighting_Changed()
{
    BaseHeightMapRenderObjClass::Static_Lighting_Changed();

    if (m_map != nullptr) {
        IRegion2D region;
        region.lo.x = 0;
        region.lo.y = 0;
        region.hi.x = 16 * m_tileWidth;
        region.hi.y = 16 * m_tileHeight;

        for (int i = 0; i < m_tileWidth; i++) {
            for (int j = 0; j < m_tileHeight; j++) {
                m_tiles[m_tileWidth * j + i].Do_Partial_Update(region, m_map, true);
            }
        }
    }
}

void FlatHeightMapRenderObjClass::Reset()
{
    BaseHeightMapRenderObjClass::Reset();
}

void FlatHeightMapRenderObjClass::Release_Resources()
{
    BaseHeightMapRenderObjClass::Release_Resources();
}

void FlatHeightMapRenderObjClass::Re_Acquire_Resources()
{
    if (m_map != nullptr) {
        int width = (m_map->Get_X_Extent() + 14) / 16;
        int height = (m_map->Get_Y_Extent() + 14) / 16;
        m_tiles = new W3DTerrainBackground[width * height];
        m_tileCount = width * height;
        m_tileWidth = width;
        m_tileHeight = height;

        for (int i = 0; i < m_tileWidth; i++) {
            for (int j = 0; j < m_tileHeight; j++) {
                m_tiles[m_tileWidth * j + i].Allocate_Terrain_Buffers(m_map, 16 * i, 16 * j, 16);
                m_tiles[m_tileWidth * j + i].Set_Flip(m_map);
            }
        }

        IRegion2D region;
        region.lo.x = 0;
        region.lo.y = 0;
        region.hi.x = m_map->Get_X_Extent();
        region.hi.y = m_map->Get_Y_Extent();

        for (int i = 0; i < m_tileWidth; i++) {
            for (int j = 0; j < m_tileHeight; j++) {
                m_tiles[m_tileWidth * j + i].Do_Partial_Update(region, m_map, true);
            }
        }
    }

    BaseHeightMapRenderObjClass::Re_Acquire_Resources();
}

void FlatHeightMapRenderObjClass::Release_Tiles()
{
    delete[] m_tiles;
    m_tiles = nullptr;
    m_tileWidth = 0;
    m_tileHeight = 0;
    m_tileCount = 0;
}
