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
#include "globaldata.h"
#include "rinfo.h"
#include "shadermanager.h"
#include "terraintex.h"
#include "w3dbibbuffer.h"
#include "w3dbridgebuffer.h"
#include "w3droadbuffer.h"
#include "w3dscene.h"
#include "w3dshroud.h"
#include "w3dterrainbackground.h"
#include "w3dterraintracks.h"
#include "w3dtreebuffer.h"
#include "w3dwaypointbuffer.h"

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
#ifdef BUILD_WITH_D3D8
    bool use_cloud_map = g_theWriteableGlobalData->m_useCloudMap;
    Matrix3D tm(m_transform);

    if (m_treeBuffer != nullptr) {
        m_treeBuffer->Set_Is_Terrain();
    }

    DX8Wrapper::Set_Light_Environment(rinfo.m_lightEnvironment);
    m_stageTwoTexture->Restore();
    DX8Wrapper::Set_Texture(0, nullptr);
    DX8Wrapper::Set_Texture(1, nullptr);
    ShaderClass::Invalidate();
    DX8Wrapper::Set_Transform(D3DTS_WORLD, tm);
    DX8Wrapper::Set_Material(m_vertexMaterialClass);
    DX8Wrapper::Set_Shader(m_shaderClass);

    if (g_theWriteableGlobalData->m_timeOfDay == TIME_OF_DAY_NIGHT) {
        use_cloud_map = false;
    }

    W3DShaderManager::ShaderTypes type = W3DShaderManager::ST_FLAT_TERRAIN;

    if (g_theWriteableGlobalData->m_useLightMap && use_cloud_map) {
        type = W3DShaderManager::ST_FLAT_TERRAIN_NOISE12;
    } else if (g_theWriteableGlobalData->m_useLightMap) {
        type = W3DShaderManager::ST_FLAT_TERRAIN_NOISE2;
    } else if (use_cloud_map) {
        type = W3DShaderManager::ST_FLAT_TERRAIN_NOISE1;
    }

    int passes = W3DShaderManager::Get_Shader_Passes(type);

    if (m_disableTextures) {
        passes = 1;
    }

    W3DShaderManager::Set_Texture(0, m_stageZeroTexture);

    if (m_shroud && rinfo.Additional_Pass_Count() && !m_disableTextures) {
        W3DShaderManager::Set_Texture(0, g_theTerrainRenderObject->Get_Shroud()->Get_Shroud_Texture());
    }

    W3DShaderManager::Set_Texture(1, nullptr);
    W3DShaderManager::Set_Texture(2, m_stageTwoTexture);
    W3DShaderManager::Set_Texture(3, m_stageThreeTexture);

    if (DX8Wrapper::Get_Back_Buffer_Format() == WW3D_FORMAT_A8R8G8B8) {
        DX8Wrapper::Set_DX8_Render_State(D3DRS_COLORWRITEENABLE, 7);
    }

    int min_x = 0;
    int max_x = m_map->Get_X_Extent();
    int min_y = 0;
    int max_y = m_map->Get_Y_Extent();
    int pass;

    for (pass = 0; pass < passes; pass++) {
        bool keep_textures = m_disableTextures;

        if (m_disableTextures) {
            DX8Wrapper::Set_Shader(ShaderClass::s_presetOpaque2DShader);
            DX8Wrapper::Set_Texture(0, nullptr);
        } else {
            W3DShaderManager::Set_Shader(type, pass);
        }

        for (int w = 0; w < m_tileWidth; w++) {
            for (int h = 0; h < m_tileHeight; h++) {
                W3DTerrainBackground *tile = &m_tiles[m_tileWidth * h + w];

                if (pass > 0) {
                    keep_textures = 1;
                }

                if (!tile->Is_Culled_Out()) {
                    tile->Draw_Visible_Polys(&rinfo, keep_textures);

                    if (16 * w < max_y) {
                        max_y = 16 * w;
                    }

                    if (16 * h < max_x) {
                        max_x = 16 * h;
                    }

                    if (16 * (w + 1) > min_y) {
                        min_y = 16 * (w + 1);
                    }

                    if (16 * (h + 1) > min_x) {
                        min_x = 16 * (h + 1);
                    }
                }
            }
        }
    }

    if (pass) {
        W3DShaderManager::Reset_Shader(type);
    }

    Render_Shore_Lines(&rinfo.m_camera);
    DX8Wrapper::Set_Texture(0, nullptr);
    DX8Wrapper::Set_Texture(1, nullptr);
    m_stageTwoTexture->Restore();
    ShaderClass::Invalidate();

    if (!ShaderClass::Is_Backface_Culling_Inverted()) {
        DX8Wrapper::Set_Material(m_vertexMaterialClass);

        if (m_scene != nullptr) {
            RTS3DScene *scene = static_cast<RTS3DScene *>(m_scene);
            RefMultiListIterator<RenderObjClass> lights(scene->Get_Dynamic_Lights());
            TextureClass *noise_texture;

            if (g_theWriteableGlobalData->m_useLightMap) {
                noise_texture = m_stageThreeTexture;
            } else {
                noise_texture = nullptr;
            }

            TextureClass *cloud_texture;

            if (use_cloud_map) {
                cloud_texture = m_stageTwoTexture;
            } else {
                cloud_texture = nullptr;
            }

            m_roadBuffer->Draw_Roads(&rinfo.m_camera,
                cloud_texture,
                noise_texture,
                m_disableTextures,
                max_y - m_map->Border_Size(),
                min_y - m_map->Border_Size(),
                max_x - m_map->Border_Size(),
                min_x - m_map->Border_Size(),
                &lights);
        }
    }

    DX8Wrapper::Set_Texture(0, nullptr);
    DX8Wrapper::Set_Texture(1, nullptr);
    m_stageTwoTexture->Restore();
    ShaderClass::Invalidate();

    if (!ShaderClass::Is_Backface_Culling_Inverted()) {
        Draw_Scorches();
    }

    DX8Wrapper::Set_Texture(0, nullptr);
    DX8Wrapper::Set_Texture(1, nullptr);
    m_stageTwoTexture->Restore();
    ShaderClass::Invalidate();
    DX8Wrapper::Apply_Render_State_Changes();
    m_bridgeBuffer->Draw_Bridges(&rinfo.m_camera, m_disableTextures, m_stageTwoTexture);

    if (g_theTerrainTracksRenderObjClassSystem != nullptr) {
        g_theTerrainTracksRenderObjClassSystem->Flush();
    }

    ShaderClass::Invalidate();
    DX8Wrapper::Apply_Render_State_Changes();
    m_waypointBuffer->Draw_Waypoints(rinfo);
    m_bibBuffer->Render_Bibs();
    DX8Wrapper::Set_Texture(0, nullptr);
    DX8Wrapper::Set_Texture(1, nullptr);
    m_stageTwoTexture->Restore();
    ShaderClass::Invalidate();
    DX8Wrapper::Set_Material(nullptr);
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
