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
#include "camera.h"
#include "globaldata.h"
#include "rinfo.h"
#include "shadermanager.h"
#include "terraintex.h"
#include "w3d.h"
#include "w3dbibbuffer.h"
#include "w3dbridgebuffer.h"
#include "w3ddynamiclight.h"
#include "w3dpropbuffer.h"
#include "w3droadbuffer.h"
#include "w3dscene.h"
#include "w3dshroud.h"
#include "w3dterrainbackground.h"
#include "w3dterraintracks.h"
#include "w3dtreebuffer.h"
#include "w3dwaypointbuffer.h"

int s_visMinX = 0;
int s_visMinY = 0;
int s_visMaxX = 0;
int s_visMaxY = 0;

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
#ifdef BUILD_WITH_D3D8
    static int count = 0;
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
    DX8Wrapper::Set_Index_Buffer(m_indexBuffer, 0);
    bool b = false;
    RTS3DScene *scene = static_cast<RTS3DScene *>(rinfo.m_camera.Get_User_Data());
    int passes;
    W3DShaderManager::ShaderTypes type;

    if (scene->Get_Custom_Scene_Pass_Mode() == MODE_MASK
        && static_cast<SceneClass *>(rinfo.m_camera.Get_User_Data())->Get_Extra_Pass_Polygon_Mode()
            == SceneClass::EXTRA_PASS_CLEAR_LINE) {
        if (!W3D::Is_Texturing_Enabled()) {
            DX8Wrapper::Set_Material(m_vertexMaterialClass);
            DX8Wrapper::Set_Shader(ShaderClass::s_presetOpaqueSolidShader);
            passes = 1;
            DX8Wrapper::Apply_Render_State_Changes();
            DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_TEXTUREFACTOR, 0xFF808080);
            b = true;
            Render_Terrain_Pass(&rinfo.m_camera);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_TEXTUREFACTOR, 0xFF008000);
            return;
        }

        passes = 1;
        b = true;

        if (rinfo.Additional_Pass_Count()) {
            rinfo.Peek_Additional_Pass(0)->Install_Materials();
            Render_Terrain_Pass(&rinfo.m_camera);
            rinfo.Peek_Additional_Pass(0)->UnInstall_Materials();
            return;
        }
    } else {
        DX8Wrapper::Set_Material(m_vertexMaterialClass);
        DX8Wrapper::Set_Shader(m_shaderClass);

        if (g_theWriteableGlobalData->m_timeOfDay == TIME_OF_DAY_NIGHT) {
            use_cloud_map = false;
        }

        type = W3DShaderManager::ST_TERRAIN;

        if (ShaderClass::Is_Backface_Culling_Inverted()) {
            type = W3DShaderManager::ST_TERRAIN;
        } else if (g_theWriteableGlobalData->m_useLightMap && use_cloud_map) {
            type = W3DShaderManager::ST_TERRAIN_NOISE12;
        } else if (g_theWriteableGlobalData->m_useLightMap) {
            type = W3DShaderManager::ST_TERRAIN_NOISE2;
        } else if (use_cloud_map) {
            type = W3DShaderManager::ST_TERRAIN_NOISE1;
        }

        passes = W3DShaderManager::Get_Shader_Passes(type);

        if (m_disableTextures) {
            passes = 1;
        }

        W3DShaderManager::Set_Texture(0, m_stageZeroTexture);
        W3DShaderManager::Set_Texture(1, m_stageZeroTexture);
        W3DShaderManager::Set_Texture(2, m_stageTwoTexture);
        W3DShaderManager::Set_Texture(3, m_stageThreeTexture);

        if (DX8Wrapper::Get_Back_Buffer_Format() == WW3D_FORMAT_A8R8G8B8) {
            DX8Wrapper::Set_DX8_Render_State(D3DRS_COLORWRITEENABLE, 7);
        }
    }

    int pass;

    for (pass = 0; pass < passes; pass++) {
        if (!b) {
            if (m_disableTextures) {
                DX8Wrapper::Set_Shader(ShaderClass::s_presetOpaque2DShader);
                DX8Wrapper::Set_Texture(0, nullptr);
            } else {
                W3DShaderManager::Set_Shader(type, pass);
            }
        }

        for (int y_tile = 0; y_tile < m_numVBTilesY; y_tile++) {
            for (int x_tile = 0; x_tile < m_numVBTilesX; x_tile++) {
                count++;
                int polygon_count = 2048;
                int vertex_count = 4096;
                DX8Wrapper::Set_Vertex_Buffer(m_vertexBufferTiles[x_tile + m_numVBTilesX * y_tile], 0);

                if (!Is_Hidden()) {
                    DX8Wrapper::Draw_Triangles(0, polygon_count, 0, vertex_count);
                }
            }
        }
    }

    if (b) {
        m_bridgeBuffer->Draw_Bridges(&rinfo.m_camera, m_disableTextures, m_stageTwoTexture);
    } else {
        if (pass) {
            W3DShaderManager::Reset_Shader(type);
        }

        Render_Shore_Lines(&rinfo.m_camera);

        if (g_theWriteableGlobalData->m_use3WayTerrainBlends != 0) {
            Render_Extra_Blend_Tiles();
        }

        int y_origin = m_map->Get_Draw_Origin_Y();
        int y = m_y + m_map->Get_Draw_Origin_Y() - 1;
        int x_origin = m_map->Get_Draw_Origin_X();
        int x = m_x + m_map->Get_Draw_Origin_X() - 1;
        DX8Wrapper::Set_Texture(0, nullptr);
        DX8Wrapper::Set_Texture(1, nullptr);
        m_stageTwoTexture->Restore();
        ShaderClass::Invalidate();

        if (!ShaderClass::Is_Backface_Culling_Inverted()) {
            DX8Wrapper::Set_Material(m_vertexMaterialClass);

            if (m_scene != nullptr) {
                RTS3DScene *rts_scene = static_cast<RTS3DScene *>(m_scene);
                RefMultiListIterator<RenderObjClass> lights(rts_scene->Get_Dynamic_Lights());
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
                    x_origin - m_map->Border_Size(),
                    x - m_map->Border_Size(),
                    y_origin - m_map->Border_Size(),
                    y - m_map->Border_Size(),
                    &lights);
            }
        }

        if (m_propBuffer != nullptr) {
            m_propBuffer->Draw_Props(rinfo);
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
        TextureClass *cloud_texture;

        if (use_cloud_map) {
            cloud_texture = m_stageTwoTexture;
        } else {
            cloud_texture = nullptr;
        }

        m_bridgeBuffer->Draw_Bridges(&rinfo.m_camera, m_disableTextures, cloud_texture);

        if (g_theTerrainTracksRenderObjClassSystem != nullptr) {
            g_theTerrainTracksRenderObjClassSystem->Flush();
        }

        if (m_shroud && rinfo.Additional_Pass_Count() != 0) {
            rinfo.Peek_Additional_Pass(0)->Install_Materials();
            Render_Terrain_Pass(&rinfo.m_camera);
            rinfo.Peek_Additional_Pass(0)->UnInstall_Materials();
        }

        ShaderClass::Invalidate();
        DX8Wrapper::Apply_Render_State_Changes();
    }

    if (m_waypointBuffer != nullptr) {
        m_waypointBuffer->Draw_Waypoints(rinfo);
    }

    m_bibBuffer->Render_Bibs();
    DX8Wrapper::Set_Texture(0, nullptr);
    DX8Wrapper::Set_Texture(1, nullptr);
    m_stageTwoTexture->Restore();
    ShaderClass::Invalidate();
    DX8Wrapper::Set_Material(nullptr);
#endif
}

void HeightMapRenderObjClass::On_Frame_Update()
{
    BaseHeightMapRenderObjClass::On_Frame_Update();

    if (m_scene != nullptr) {
        RTS3DScene *scene = static_cast<RTS3DScene *>(m_scene);

        RefMultiListIterator<RenderObjClass> lights(scene->Get_Dynamic_Lights());
        if (m_map != nullptr) {
            int num_lights = 0;
            W3DDynamicLight *dynamic_lights[20];
            int y_min = m_map->Get_Draw_Origin_Y();
            int y_max = m_map->Get_Draw_Origin_Y() + m_y;
            int x_min = m_map->Get_Draw_Origin_X();
            int x_max = m_map->Get_Draw_Origin_X() + m_x;

            for (lights.First(); !lights.Is_Done(); lights.Next()) {
                W3DDynamicLight *light = static_cast<W3DDynamicLight *>(lights.Peek_Obj());
                light->m_processMe = false;

                if (light->m_enabled || light->m_priorEnable) {
                    float range = light->Get_Attenuation_Range();

                    if (light->m_priorEnable) {
                        light->m_prevMinX = light->m_minX;
                        light->m_prevMinY = light->m_minY;
                        light->m_prevMaxX = light->m_maxX;
                        light->m_prevMaxY = light->m_maxY;
                    }

                    Vector3 pos = light->Get_Position();
                    light->m_minX = (pos.X - range) / 10.0f;
                    light->m_maxX = (pos.X + range) / 10.0f + 1.0f;
                    light->m_minY = (pos.Y - range) / 10.0f;
                    light->m_maxY = (pos.Y + range) / 10.0f + 1.0f;

                    if (!light->m_priorEnable) {
                        light->m_prevMinX = light->m_minX;
                        light->m_prevMinY = light->m_minY;
                        light->m_prevMaxX = light->m_maxX;
                        light->m_prevMaxY = light->m_maxY;
                    }

                    light->m_processMe =
                        (light->m_minX < x_max && light->m_minY < y_max && light->m_maxX > x_min && light->m_maxY > y_min)
                        || (light->m_prevMinX < x_max && light->m_prevMinY < y_max && light->m_prevMaxX > x_min
                            && light->m_prevMaxY > y_min);

                    if (light->m_processMe) {
                        dynamic_lights[num_lights] = light;
                        num_lights++;

                        if (num_lights == 20) {
                            break;
                        }
                    }
                }

                light->m_priorEnable = light->m_enabled;
            }

            if (num_lights > 0) {
                for (int y_tile = 0; y_tile < m_numVBTilesY; y_tile++) {
                    int origin_y = 32 * y_tile;
                    int y0 = 32 * y_tile;
                    int y1 = 32 * y_tile;
                    bool y_contains_light = false;
                    int y_max_origin = Get_Y_With_Origin(y0) + m_map->Get_Draw_Origin_Y() - m_map->Border_Size();
                    int y_min_origin = Get_Y_With_Origin(y1 - 1) + m_map->Get_Draw_Origin_Y() + 1 - m_map->Border_Size();

                    if (y_min_origin <= y_max_origin) {
                        for (int j = 0; j < num_lights; j++) {
                            if (dynamic_lights[j]->m_minY <= y_min_origin || dynamic_lights[j]->m_maxY >= y_max_origin) {
                                y_contains_light = true;
                                break;
                            }

                            if (dynamic_lights[j]->m_prevMinY <= y_min_origin
                                || dynamic_lights[j]->m_prevMaxY >= y_max_origin) {
                                y_contains_light = true;
                                break;
                            }
                        }
                    } else {
                        for (int j = 0; j < num_lights; j++) {
                            if (dynamic_lights[j]->m_minY < y_min_origin && dynamic_lights[j]->m_maxY > y_max_origin) {
                                y_contains_light = true;
                                break;
                            }

                            if (dynamic_lights[j]->m_prevMinY < y_min_origin
                                && dynamic_lights[j]->m_prevMaxY > y_max_origin) {
                                y_contains_light = true;
                                break;
                            }
                        }
                    }

                    if (y_contains_light) {
                        for (int x_tile = 0; x_tile < m_numVBTilesX; x_tile++) {
                            int origin_x = 32 * x_tile;
                            int x0 = 32 * x_tile;
                            int x1 = 32 * x_tile + 32;
                            bool x_contains_light = false;
                            int x_max_origin = Get_X_With_Origin(x0) + m_map->Get_Draw_Origin_X() - m_map->Border_Size();
                            int x_min_origin =
                                Get_X_With_Origin(x1 - 1) + m_map->Get_Draw_Origin_X() + 1 - m_map->Border_Size();

                            if (x_min_origin <= x_max_origin) {
                                for (int j = 0; j < num_lights; j++) {
                                    if (dynamic_lights[j]->m_minX <= x_min_origin
                                        || dynamic_lights[j]->m_maxX >= x_max_origin) {
                                        x_contains_light = true;
                                        break;
                                    }

                                    if (dynamic_lights[j]->m_prevMinX <= x_min_origin
                                        || dynamic_lights[j]->m_prevMaxX >= x_max_origin) {
                                        x_contains_light = true;
                                        break;
                                    }
                                }
                            } else {
                                for (int j = 0; j < num_lights; j++) {
                                    if (dynamic_lights[j]->m_minX < x_min_origin
                                        && dynamic_lights[j]->m_maxX > x_max_origin) {
                                        x_contains_light = true;
                                        break;
                                    }

                                    if (dynamic_lights[j]->m_prevMinX < x_min_origin
                                        && dynamic_lights[j]->m_prevMaxX > x_max_origin) {
                                        x_contains_light = true;
                                        break;
                                    }
                                }
                            }

                            if (x_contains_light) {
                                Update_VB_For_Light(m_vertexBufferTiles[m_numVBTilesX * y_tile + x_tile],
                                    m_vertexBufferBackup[m_numVBTilesX * y_tile + x_tile],
                                    x0,
                                    y0,
                                    x1,
                                    y1,
                                    origin_x,
                                    origin_y,
                                    dynamic_lights,
                                    num_lights);
                            }
                        }
                    }
                }
            }
        }
    }
}

int HeightMapRenderObjClass::Init_Height_Data(
    int x, int y, WorldHeightMap *map, RefMultiListIterator<RenderObjClass> *lights, bool update_shoreline)
{
    BaseHeightMapRenderObjClass::Init_Height_Data(x, y, map, lights, update_shoreline);
    unsigned char *data = nullptr;

    if (map != nullptr) {
        data = map->Get_Data_Ptr();
    }

    if (update_shoreline) {
        m_numExtraBlendTiles = 0;

        if (map != nullptr) {
            int x_extent = map->Get_X_Extent();
            int y_extent = map->Get_Y_Extent();

            if (m_extraBlendTilePositions == nullptr) {
                m_extraBlendTilePositions = new unsigned int[2048];
                m_extraBlendTilePositionsSize = 2048;
            }

            for (int y_pos = 0; y_pos < y_extent - 1; y_pos++) {
                for (int x_pos = 0; x_pos < x_extent - 1; x_pos++) {
                    float u[4];
                    float v[4];
                    unsigned char alpha[4];
                    bool need_flip;
                    bool is_cliff;

                    if (map->Get_Extra_Alpha_UV_Data(x_pos, y_pos, u, v, alpha, &need_flip, &is_cliff)) {
                        if (m_numExtraBlendTiles >= m_extraBlendTilePositionsSize) {
                            unsigned int *tiles = new unsigned int[m_extraBlendTilePositionsSize + 512];
                            memcpy(tiles, m_extraBlendTilePositions, sizeof(unsigned int) * m_extraBlendTilePositionsSize);
                            delete[] m_extraBlendTilePositions;
                            m_extraBlendTilePositions = tiles;
                            m_extraBlendTilePositionsSize += 512;
                        }

                        m_extraBlendTilePositions[m_numExtraBlendTiles] = (y_pos << 16) | x_pos;
                        m_numExtraBlendTiles++;
                    }
                }
            }
        }
    }

    m_originX = 0;
    m_originY = 0;
    m_needFullUpdate = true;
    bool create_tiles = x != m_x || y != m_y;

    if (m_stageOneTexture == nullptr) {
        create_tiles = true;
    }

    if (data != nullptr && create_tiles) {
        Free_Index_Vertex_Buffers();
        m_indexBuffer = new DX8IndexBufferClass(6144, DX8IndexBufferClass::USAGE_DEFAULT);
        IndexBufferClass::WriteLockClass ib_lock(m_indexBuffer, 0);
        unsigned short *indices = ib_lock.Get_Index_Array();

        for (int i = 0; i < 4096; i += 128) {
            for (int j = i; j < i + 128; j += 4) {
                indices[0] = j;
                indices[1] = j + 2;
                indices[2] = j + 3;
                indices[3] = j;
                indices[4] = j + 1;
                indices[5] = j + 2;
                indices += 6;
            }
        }

        m_numVBTilesX = 1;

        for (int j = 33; j < x; j += 32) {
            m_numVBTilesX++;
        }

        m_numVBTilesY = 1;

        for (int i = 33; i < y; i += 32) {
            m_numVBTilesY++;
        }

        m_numBlockColumnsInLastVB = (x - 1) % 32;
        m_numBlockRowsInLastVB = (y - 1) % 32;
        m_numVertexBufferTiles = m_numVBTilesX * m_numVBTilesY;
        m_x = x;
        m_y = y;
        m_vertexBufferTiles = new DX8VertexBufferClass *[m_numVertexBufferTiles];
        m_vertexBufferBackup = new char *[m_numVertexBufferTiles];

        for (int j = 0; j < m_numVertexBufferTiles; j++) {
            m_vertexBufferTiles[j] = new DX8VertexBufferClass(DX8_FVF_XYZDUV2, 4096, DX8VertexBufferClass::USAGE_DEFAULT, 0);
            m_vertexBufferBackup[j] = new char[0x20000];
        }
    }

    Update_Block(0, 0, x - 1, y - 1, map, lights);
    return 0;
}

int HeightMapRenderObjClass::Free_Map_Resources()
{
    BaseHeightMapRenderObjClass::Free_Map_Resources();
    Free_Index_Vertex_Buffers();
    return 0;
}

bool Check(const FrustumClass &frustum, WorldHeightMap *map, int x, int y)
{
    if (x < 0 || y < 0) {
        return false;
    }

    if (x >= map->Get_X_Extent() || y >= map->Get_Y_Extent()) {
        return false;
    }

    if (x >= s_visMinX && y >= s_visMinY && x <= s_visMaxX && y <= s_visMaxY) {
        return true;
    }

    Vector3 v1((x - map->Border_Size()) * 10.0f, (y - map->Border_Size()) * 10.0f, map->Get_Height(x, y) * HEIGHTMAP_SCALE);

    if (CollisionMath::Overlap_Test(frustum, v1) != CollisionMath::INSIDE) {
        return false;
    }

    if (x < s_visMinX) {
        s_visMinX = x;
    }

    if (x > s_visMaxX) {
        s_visMaxX = x;
    }

    if (y < s_visMinY) {
        s_visMinY = y;
    }

    if (y > s_visMaxY) {
        s_visMaxY = y;
    }

    return true;
}

void Calc_Vis(const FrustumClass &frustum, WorldHeightMap *map, int min_x, int min_y, int max_x, int max_y, int limit)
{
    if (max_x - min_x >= 2 && max_y - min_y >= 2
        && (min_x < s_visMinX || min_y < s_visMinY || max_x > s_visMaxX || max_y > s_visMaxY)) {
        int x = (max_x + min_x) / 2;
        int y = (max_y + min_y) / 2;
        bool recurse1 = max_x - min_x >= limit;
        bool recurse2 = recurse1;
        bool recurse3 = recurse1;
        bool recurse4 = recurse1;

        if (Check(frustum, map, x, max_y)) {
            recurse1 = true;
            recurse2 = true;
        }

        if (Check(frustum, map, x, min_y)) {
            recurse3 = true;
            recurse4 = true;
        }

        if (Check(frustum, map, x, y)) {
            recurse1 = true;
            recurse2 = true;
            recurse3 = true;
            recurse4 = true;
        }

        if (Check(frustum, map, min_x, y)) {
            recurse1 = true;
            recurse3 = true;
        }

        if (Check(frustum, map, max_x, y)) {
            recurse2 = true;
            recurse4 = true;
        }

        if (recurse1) {
            Calc_Vis(frustum, map, min_x, y, x, max_y, limit);
        }
        if (recurse2) {
            Calc_Vis(frustum, map, x, y, max_x, max_y, limit);
        }
        if (recurse3) {
            Calc_Vis(frustum, map, min_x, min_y, x, y, limit);
        }
        if (recurse4) {
            Calc_Vis(frustum, map, x, min_y, max_x, y, limit);
        }
    }
}

int Invert_Value(int value)
{
    if (value < 0) {
        return -value;
    } else {
        return value;
    }
}

void HeightMapRenderObjClass::Update_Center(CameraClass *camera, RefMultiListIterator<RenderObjClass> *lights)
{
    if (m_map != nullptr && !m_updating && m_vertexBufferTiles != nullptr) {
        BaseHeightMapRenderObjClass::Update_Center(camera, lights);
        m_updating = true;

        if (m_needFullUpdate) {
            m_needFullUpdate = false;
            Update_Block(0, 0, m_x - 1, m_y - 1, m_map, lights);
            m_updating = false;
            return;
        }

        if (m_x >= m_map->Get_X_Extent() && m_y >= m_map->Get_Y_Extent()) {
            m_updating = false;
            return;
        }

        int i1 = 1;
        Matrix3D camera_transform = camera->Get_Transform();
        Vector3 camera_pos = camera->Get_Position();
        Vector3 v1;
        Vector3 v2;
        const ViewportClass &viewport = camera->Get_Viewport();
        int height = WorldHeightMap::Get_Max_Height_Value();

        for (int x = 0; x < m_x; x++) {
            for (int y = 0; y < m_y; y++) {
                unsigned char display_height = m_map->Get_Display_Height(x, y);

                if (display_height < height) {
                    height = display_height;
                }
            }
        }

        float z = height;
        Vector2 min;
        Vector2 max;
        camera->Get_View_Plane(min, max);
        float x_dist = max.X - min.X;
        float y_dist = max.Y - min.Y;
        float f1 = -1.0f;
        float f2 = 200000.0f;
        float f3 = -200000.0f;
        float f4 = 200000.0f;
        float f5 = f3;

        for (int x = 0; x < 2; x++) {
            for (int y = 0; y < 2; y++) {
                float f6 = (-x + 0.5f + viewport.m_min.X) * f1 * x_dist;
                float f7 = (y - 0.5f - viewport.m_min.Y) * f1 * y_dist;
                float f8 = f1 * camera_transform[0][2];
                float f9 = f6 * camera_transform[0][0] + f8;
                float f10 = f7 * camera_transform[0][1] + f9;
                float f11 = f1 * camera_transform[1][2];
                float f12 = f6 * camera_transform[1][0] + f11;
                float f13 = f7 * camera_transform[1][1] + f12;
                float f14 = f1 * camera_transform[2][2];
                float f15 = f6 * camera_transform[2][0] + f14;
                float f16 = f7 * camera_transform[2][1] + f15;
                v1.Set(f10, f13, f16);
                v1.Normalize();
                v2 = camera_pos + v1;
                f10 = Vector3::Find_X_At_Z(z, camera_pos, v2);
                f13 = Vector3::Find_Y_At_Z(z, camera_pos, v2);

                if (f10 < f2) {
                    f2 = f10;
                }

                if (f10 > f3) {
                    f3 = f10;
                }

                if (f13 < f4) {
                    f4 = f13;
                }

                if (f13 > f5) {
                    f5 = f13;
                }
            }
        }

        f2 = f2 / 10.0f;
        f3 = f3 / 10.0f;
        f4 = f4 / 10.0f;
        f5 = f5 / 10.0f;
        f2 = m_map->Border_Size() + f2;
        f3 = m_map->Border_Size() + f3;
        f4 = m_map->Border_Size() + f4;
        f5 = m_map->Border_Size() + f5;

        s_visMinX = m_map->Get_X_Extent();
        s_visMinY = m_map->Get_Y_Extent();
        s_visMaxX = 0;
        s_visMaxY = 0;

        if (f2 < 0.0f) {
            f2 = 0.0f;
        }

        if (f4 < 0.0f) {
            f4 = 0.0f;
        }

        if (s_visMinX < f3) {
            f3 = s_visMinX;
        }

        if (s_visMinY < f5) {
            f5 = s_visMinY;
        }

        const FrustumClass &frustum = camera->Get_Frustum();
        int limit = ((f3 - f2) / 2.0f);

        if (limit > 16) {
            limit = 16;
        }

        Calc_Vis(frustum, m_map, (f2 - 16.0f), (f4 - 16.0f), (f3 + 16.0f), (f5 + 16.0f), limit);

        if (m_map != nullptr) {
            int x_org;
            int y_org;

            if (s_visMaxX - s_visMinX <= m_x) {
                x_org = (((s_visMinX + s_visMaxX) / 2) - m_x / 2.0f);
            } else {
                x_org = ((f3 + f2) / 2.0f - m_x / 2.0f);
            }

            if (s_visMaxY - s_visMinY <= m_y) {
                y_org = (((s_visMinY + s_visMaxY) / 2) - m_y / 2.0f);
            } else {
                y_org = s_visMinY + 1;
            }

            if (g_theTacticalView->Get_Field_Of_View() != 0.0f) {
                x_org = (((s_visMinX + s_visMaxX) / 2) - m_x / 2.0f);
                y_org = (((s_visMinY + s_visMaxY) / 2) - m_y / 2.0f);
            }

            int i2 = x_org - m_map->Get_Draw_Origin_X();
            int i3 = y_org - m_map->Get_Draw_Origin_Y();

            if (Invert_Value(i2) > m_x / 2 || Invert_Value(i3) > m_x / 2) {
                m_map->Set_Draw_Origin(x_org, y_org);
                m_originY = 0;
                m_originX = 0;
                Update_Block(0, 0, m_x - 1, m_y - 1, m_map, lights);
                m_updating = false;
                return;
            }

            if (abs(i2) > 2 || abs(i3) > 2) {
                if (abs(i3) >= 2) {
                    if (m_map->Set_Draw_Origin(m_map->Get_Draw_Origin_X(), y_org)) {
                        int i4 = 0;
                        int i5 = 0;
                        i3 -= y_org - m_map->Get_Draw_Origin_Y();
                        m_originY += i3;

                        if (m_originY >= m_y - 1) {
                            m_originY -= m_y - 1;
                        }

                        if (i3 >= 0) {
                            i4 = m_originY - i3;
                            i5 = m_originY;
                        } else {
                            i4 = m_originY;
                            i5 = m_originY - i3;
                        }

                        i4 -= i1;

                        if (m_originY < 0) {
                            m_originY = m_originY + m_y - 1;
                        }

                        if (i4 >= 0) {
                            Update_Block(0, i4, m_x - 1, i5, m_map, lights);
                        } else {
                            i4 = i4 + m_y - 1;

                            if (i4 < 0) {
                                i4 = 0;
                            }

                            Update_Block(0, i4, m_x - 1, m_y - 1, m_map, lights);
                            Update_Block(0, 0, m_x - 1, i5, m_map, lights);
                        }
                    }

                    if (abs(i2) < 16 && !m_doXNextTime) {
                        m_updating = false;
                        m_doXNextTime = true;
                        return;
                    }
                }

                if (abs(i2) > 2) {
                    m_doXNextTime = false;
                    x_org = i2 + m_map->Get_Draw_Origin_X();

                    if (m_map->Set_Draw_Origin(x_org, m_map->Get_Draw_Origin_Y())) {
                        int i6 = 0;
                        int i7 = 0;
                        i2 -= x_org - m_map->Get_Draw_Origin_X();
                        m_originX += i2;

                        if (m_originX >= m_x - 1) {
                            m_originX -= m_x - 1;
                        }

                        if (i2 >= 0) {
                            i6 = m_originX - i2;
                            i7 = m_originX;
                        } else {
                            i6 = m_originX;
                            i7 = m_originX - i2;
                        }

                        i6 -= i1;
                        i7 += i1;

                        if (m_originX < 0) {
                            m_originX = m_originX + m_x - 1;
                        }

                        if (i6 >= 0) {
                            Update_Block(i6, 0, i7, m_y - 1, m_map, lights);
                        } else {
                            i6 = i6 + m_x - 1;

                            if (i6 < 0) {
                                i6 = 0;
                            }

                            Update_Block(i6, 0, m_x - 1, m_y - 1, m_map, lights);
                            Update_Block(0, 0, i7, m_y - 1, m_map, lights);
                        }
                    }
                }
            }
        }

        m_updating = false;
    }
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

    for (int y_tile = 0; y_tile < m_numVBTilesY; y_tile++) {
        for (int x_tile = 0; x_tile < m_numVBTilesX; x_tile++) {
            count++;
            DX8Wrapper::Set_Vertex_Buffer(m_vertexBufferTiles[x_tile + m_numVBTilesX * y_tile], 0);

            if (!Is_Hidden()) {
                DX8Wrapper::Draw_Triangles(0, 2048, 0, 4096);
            }
        }
    }
#endif
}

void HeightMapRenderObjClass::Render_Extra_Blend_Tiles()
{
    int vertex_count = 0;
    int poly_count = 0;
    static int max_blend_tiles = 256;
    m_extraBlendTilesRendered = 0;

    if (m_numExtraBlendTiles != 0) {
        if (max_blend_tiles > 10000) {
            max_blend_tiles = 10000;
        }

        DynamicVBAccessClass vb(VertexBufferClass::BUFFER_TYPE_DYNAMIC_DX8, DX8_FVF_XYZNDUV2, 4 * max_blend_tiles);
        DynamicIBAccessClass ib(IndexBufferClass::BUFFER_TYPE_DYNAMIC_DX8, 6 * max_blend_tiles);

        {
            DynamicVBAccessClass::WriteLockClass vb_lock(&vb);
            VertexFormatXYZNDUV2 *vertices = vb_lock.Get_Formatted_Vertex_Array();
            DynamicIBAccessClass::WriteLockClass ib_lock(&ib);
            unsigned short *indices = ib_lock.Get_Index_Array();

            if (vertices == nullptr || indices == nullptr) {
                return;
            }

            unsigned char *data = m_map->Get_Data_Ptr();
            int y = m_map->Get_Draw_Origin_Y() + m_map->Get_Draw_Height() - 1;
            int x = m_map->Get_Draw_Origin_X() + m_map->Get_Draw_Width() - 1;

            if (x > m_map->Get_X_Extent() - 1) {
                x = m_map->Get_X_Extent() - 1;
            }

            if (y > m_map->Get_Y_Extent() - 1) {
                y = m_map->Get_Y_Extent() - 1;
            }

            for (int i = 0; i < m_numExtraBlendTiles && vertex_count < 4 * max_blend_tiles; i++) {
                int x_pos = m_extraBlendTilePositions[i] & 0xFFFF;
                int y_pos = m_extraBlendTilePositions[i] >> 16;
                float u[4];
                float v[4];
                unsigned char alpha[4];
                bool need_flip;
                bool is_cliff;

                if (x_pos >= m_map->Get_Draw_Origin_X() && x_pos < x && y_pos >= m_map->Get_Draw_Origin_Y() && y_pos < y
                    && m_map->Get_Extra_Alpha_UV_Data(x_pos, y_pos, u, v, alpha, &need_flip, &is_cliff)) {
                    int index = m_map->Get_X_Extent() * y_pos + x_pos;
                    float z1 = data[index] * HEIGHTMAP_SCALE;
                    float z2 = data[index + 1] * HEIGHTMAP_SCALE;
                    float z3 = data[index + 1 + m_map->Get_X_Extent()] * HEIGHTMAP_SCALE;
                    float z4 = data[index + m_map->Get_X_Extent()] * HEIGHTMAP_SCALE;

                    if (is_cliff) {
                        if (abs((int)(z1 - z3)) > abs((int)(z2 - z4))) {
                            need_flip = true;
                        }
                    }

                    vertices->x = (x_pos - m_map->Border_Size()) * 10.0f;
                    vertices->y = (y_pos - m_map->Border_Size()) * 10.0f;
                    vertices->z = z1;
                    vertices->nx = 0.0f;
                    vertices->ny = 0.0f;
                    vertices->nz = 0.0f;
                    vertices->diffuse = Get_Static_Diffuse(x_pos, y_pos) & 0xFFFFFF | (alpha[0] << 24);
                    vertices->u1 = u[0];
                    vertices->v1 = v[0];
                    vertices->u2 = 0.0f;
                    vertices->v2 = 0.0f;
                    vertices++;

                    vertices->x = (x_pos + 1 - m_map->Border_Size()) * 10.0f;
                    vertices->y = (y_pos - m_map->Border_Size()) * 10.0f;
                    vertices->z = z2;
                    vertices->nx = 0.0f;
                    vertices->ny = 0.0f;
                    vertices->nz = 0.0f;
                    vertices->diffuse = Get_Static_Diffuse(x_pos + 1, y_pos) & 0xFFFFFF | (alpha[1] << 24);
                    vertices->u1 = u[1];
                    vertices->v1 = v[1];
                    vertices->u2 = 0.0f;
                    vertices->v2 = 0.0f;
                    vertices++;

                    vertices->x = (x_pos + 1 - m_map->Border_Size()) * 10.0f;
                    vertices->y = (y_pos + 1 - m_map->Border_Size()) * 10.0f;
                    vertices->z = z3;
                    vertices->nx = 0.0f;
                    vertices->ny = 0.0f;
                    vertices->nz = 0.0f;
                    vertices->diffuse = Get_Static_Diffuse(x_pos + 1, y_pos + 1) & 0xFFFFFF | (alpha[2] << 24);
                    vertices->u1 = u[2];
                    vertices->v1 = v[2];
                    vertices->u2 = 0.0f;
                    vertices->v2 = 0.0f;
                    vertices++;

                    vertices->x = (x_pos - m_map->Border_Size()) * 10.0f;
                    vertices->y = (y_pos + 1 - m_map->Border_Size()) * 10.0f;
                    vertices->z = z4;
                    vertices->nx = 0.0f;
                    vertices->ny = 0.0f;
                    vertices->nz = 0.0f;
                    vertices->diffuse = Get_Static_Diffuse(x_pos, y_pos + 1) & 0xFFFFFF | (alpha[3] << 24);
                    vertices->u1 = u[3];
                    vertices->v1 = v[3];
                    vertices->u2 = 0.0f;
                    vertices->v2 = 0.0f;
                    vertices++;

                    if (need_flip) {
                        indices[0] = vertex_count + 1;
                        indices[1] = vertex_count + 3;
                        indices[2] = vertex_count;
                        indices[3] = vertex_count + 1;
                        indices[4] = vertex_count + 2;
                        indices[5] = vertex_count + 3;
                    } else {
                        indices[0] = vertex_count;
                        indices[1] = vertex_count + 2;
                        indices[2] = vertex_count + 3;
                        indices[3] = vertex_count;
                        indices[4] = vertex_count + 1;
                        indices[5] = vertex_count + 2;
                    }

                    indices += 6;
                    vertex_count += 4;
                    poly_count += 6;
                }
            }
        }

        if (vertex_count != 0) {
            if (vertex_count == 4 * max_blend_tiles) {
                max_blend_tiles += 16;
            }

            ShaderClass::Invalidate();
            DX8Wrapper::Set_Index_Buffer(ib, 0);
            DX8Wrapper::Set_Vertex_Buffer(vb);
            VertexMaterialClass *material = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
            DX8Wrapper::Set_Material(material);
            Ref_Ptr_Release(material);
            ShaderClass shader(ShaderClass::s_presetOpaqueShader);
            shader.Set_Depth_Mask(ShaderClass::DEPTH_WRITE_DISABLE);
            DX8Wrapper::Set_Shader(shader);

            if (g_theWriteableGlobalData->m_use3WayTerrainBlends == 2) {
                shader.Set_Primary_Gradient(ShaderClass::GRADIENT_DISABLE);
                shader.Set_Texturing(ShaderClass::TEXTURING_DISABLE);
                DX8Wrapper::Set_Shader(shader);
                DX8Wrapper::Set_Texture(0, nullptr);

                if (!Is_Hidden()) {
                    DX8Wrapper::Draw_Triangles(0, poly_count / 3, 0, vertex_count);
                    m_extraBlendTilesRendered += poly_count / 6;
                }
            } else {
                W3DShaderManager::Set_Texture(0, m_stageOneTexture);
                W3DShaderManager::Set_Texture(1, m_stageTwoTexture);
                W3DShaderManager::Set_Texture(2, m_stageThreeTexture);
                W3DShaderManager::ShaderTypes type = W3DShaderManager::ST_ROAD;
                bool use_cloud_map = g_theWriteableGlobalData->m_useCloudMap;

                if (g_theWriteableGlobalData->m_timeOfDay == TIME_OF_DAY_NIGHT) {
                    use_cloud_map = 0;
                }

                if (g_theWriteableGlobalData->m_useLightMap && use_cloud_map) {
                    type = W3DShaderManager::ST_ROAD_NOISE12;
                } else if (g_theWriteableGlobalData->m_useLightMap) {
                    type = W3DShaderManager::ST_ROAD_NOISE2;
                } else if (use_cloud_map) {
                    type = W3DShaderManager::ST_ROAD_NOISE1;
                }

                int passes = W3DShaderManager::Get_Shader_Passes(type);
                for (int pass = 0; pass < passes; pass++) {
                    W3DShaderManager::Set_Shader(type, pass);

                    if (!Is_Hidden()) {
                        DX8Wrapper::Draw_Triangles(0, poly_count / 3, 0, vertex_count);
                        m_extraBlendTilesRendered += poly_count / 6;
                    }
                }

                W3DShaderManager::Reset_Shader(type);
            }
        }
    }
}
