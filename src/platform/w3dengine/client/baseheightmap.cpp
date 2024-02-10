/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Base Height Map Render Object
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "baseheightmap.h"
#include "castres.h"
#include "colmath.h"
#include "colorspace.h"
#include "coltest.h"
#include "flatheightmap.h"
#include "frustum.h"
#include "heightmap.h"
#include "light.h"
#include "rinfo.h"
#include "scene.h"
#include "shadermanager.h"
#include "terraintex.h"
#include "terrainvisual.h"
#include "tri.h"
#include "w3d_file.h"
#include "w3dbibbuffer.h"
#include "w3dbridgebuffer.h"
#include "w3dpropbuffer.h"
#include "w3droadbuffer.h"
#include "w3dscene.h"
#include "w3dshadow.h"
#include "w3dshroud.h"
#include "w3dsmudge.h"
#include "w3dsnow.h"
#include "w3dterraintracks.h"
#include "w3dtreebuffer.h"
#include "w3dwater.h"
#include "w3dwaypointbuffer.h"
#include "water.h"

#ifndef GAME_DLL
BaseHeightMapRenderObjClass *g_theTerrainRenderObject;
#endif

static ShaderClass s_detailOpaqueShader(0x29441B);

BaseHeightMapRenderObjClass::BaseHeightMapRenderObjClass() :
    m_x(0),
    m_y(0),
    m_vertexScorch(nullptr),
    m_indexScorch(nullptr),
    m_scorchTexture(nullptr),
    m_cliffAngle(45.0f),
    m_map(nullptr),
    m_useDepthFade(false),
    m_updating(false),
    m_depthFade(0.0f, 0.0f, 0.0f),
    m_disableTextures(false),
    m_needFullUpdate(false),
    m_minHeight(0.0f),
    m_maxHeight((GameMath::Pow(256.0f, 1.0f) - 1.0f) * HEIGHTMAP_SCALE),
    m_showImpassableAreas(false),
    m_vertexMaterialClass(nullptr),
    m_stageZeroTexture(nullptr),
    m_stageOneTexture(nullptr),
    m_stageTwoTexture(nullptr),
    m_stageThreeTexture(nullptr),
    m_destAlphaLUT(nullptr),
    m_treeBuffer(nullptr),
    m_propBuffer(nullptr),
    m_bibBuffer(nullptr),
    m_waypointBuffer(nullptr),
    m_roadBuffer(nullptr),
    m_bridgeBuffer(nullptr),
    m_shorelineTiles(nullptr),
    m_numShorelineBlendTiles(0),
    m_shorelineBlendTilesRendered(0),
    m_shorelineBlendTileSize(0),
    m_transparentWaterMinOpacity(-1.0f),
    m_shorelineSortInfos(nullptr),
    m_shorelineSortInfoCount(0),
    m_sortAxis(1),
    m_endCell(0),
    m_startCell(0)
{
    g_theTerrainRenderObject = this;
    m_treeBuffer = new W3DTreeBuffer();
    m_propBuffer = new W3DPropBuffer();
    m_bibBuffer = new W3DBibBuffer();
    m_bridgeBuffer = new W3DBridgeBuffer();
    m_waypointBuffer = new W3DWaypointBuffer();
    m_roadBuffer = new W3DRoadBuffer();
    Clear_All_Scorches();

#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData->m_shroudOn) {
        m_shroud = new W3DShroud();
    } else {
        m_shroud = nullptr;
    }
#else
    m_shroud = new W3DShroud();
#endif

    DX8Wrapper::Set_Cleanup_Hook(this);
}

BaseHeightMapRenderObjClass::~BaseHeightMapRenderObjClass()
{
    Free_Map_Resources();
    delete m_treeBuffer;
    delete m_propBuffer;
    delete m_bibBuffer;
    delete m_roadBuffer;
    delete m_bridgeBuffer;
    delete m_waypointBuffer;
    delete m_shroud;
    delete[] m_shorelineTiles;
    delete[] m_shorelineSortInfos;
}

int BaseHeightMapRenderObjClass::Free_Map_Resources()
{
    Free_Scorch_Buffers();
    Ref_Ptr_Release(m_vertexMaterialClass);
    Ref_Ptr_Release(m_stageZeroTexture);
    Ref_Ptr_Release(m_stageOneTexture);
    Ref_Ptr_Release(m_stageTwoTexture);
    Ref_Ptr_Release(m_stageThreeTexture);
    Ref_Ptr_Release(m_destAlphaLUT);
    Ref_Ptr_Release(m_map);
    return 0;
}

void BaseHeightMapRenderObjClass::Release_Resources()
{
    if (m_treeBuffer != nullptr) {
        m_treeBuffer->Free_Tree_Buffers();
    }

    if (m_bibBuffer != nullptr) {
        m_bibBuffer->Free_Bib_Buffers();
    }

    if (m_bridgeBuffer != nullptr) {
        m_bridgeBuffer->Free_Bridge_Buffers();
    }

    if (m_waypointBuffer != nullptr) {
        m_waypointBuffer->Free_Waypoint_Buffers();
    }

    if (m_map != nullptr) {
        m_map->Add_Ref();
    }

    WorldHeightMap *map = m_map;
    Free_Map_Resources();
    m_map = map;

    if (g_theWaterRenderObj != nullptr) {
        g_theWaterRenderObj->Release_Resources();
    }

    if (g_theTerrainTracksRenderObjClassSystem != nullptr) {
        g_theTerrainTracksRenderObjClassSystem->Release_Resources();
    }

    if (g_theW3DShadowManager != nullptr) {
        g_theW3DShadowManager->Release_Resources();
    }

    if (m_shroud != nullptr) {
        m_shroud->Reset();
        m_shroud->Release_Resources();
    }

    if (g_theSmudgeManager != nullptr) {
        g_theSmudgeManager->Release_Resources();
    }

    if (g_theSnowManager != nullptr) {
        static_cast<W3DSnowManager *>(g_theSnowManager)->Release_Resources();
    }

    W3DShaderManager::Shutdown();

    if (m_roadBuffer != nullptr) {
        m_roadBuffer->Free_Road_Buffers();
    }
}

void BaseHeightMapRenderObjClass::Re_Acquire_Resources()
{
    W3DShaderManager::Init();

    if (g_theWaterRenderObj != nullptr) {
        g_theWaterRenderObj->Re_Acquire_Resources();
    }

    if (g_theTerrainTracksRenderObjClassSystem != nullptr) {
        g_theTerrainTracksRenderObjClassSystem->Re_Acquire_Resources();
    }

    if (g_theW3DShadowManager != nullptr) {
        g_theW3DShadowManager->Re_Acquire_Resources();
    }

    if (m_shroud != nullptr) {
        m_shroud->Re_Acquire_Resources();
    }

    if (m_map != nullptr) {
        Init_Height_Data(m_x, m_y, m_map, nullptr, true);
        m_needFullUpdate = 1;
    }

    if (m_treeBuffer != nullptr) {
        m_treeBuffer->Allocate_Tree_Buffers();
    }

    if (m_bibBuffer != nullptr) {
        m_bibBuffer->Allocate_Bib_Buffers();
    }

    if (m_bridgeBuffer != nullptr) {
        m_bridgeBuffer->Allocate_Bridge_Buffers();
    }

    if (g_theSmudgeManager != nullptr) {
        g_theSmudgeManager->Re_Acquire_Resources();
    }

    if (g_theSnowManager != nullptr) {
        static_cast<W3DSnowManager *>(g_theSnowManager)->Re_Acquire_Resources();
    }

    if (m_roadBuffer != nullptr) {
        m_roadBuffer->Allocate_Road_Buffers();
        m_roadBuffer->Load_Roads();
    }
    if (g_theTacticalView != nullptr) {
        g_theTacticalView->Force_Redraw();
        g_theTacticalView->Force_Redraw(); // TODO possible bug
    }
}

void BaseHeightMapRenderObjClass::Reset()
{
    if (m_treeBuffer != nullptr) {
        m_treeBuffer->Clear_All_Trees();
    }

    if (m_propBuffer != nullptr) {
        m_propBuffer->Clear_All_Props();
    }

    Clear_All_Scorches();

    if (m_roadBuffer != nullptr) {
        m_roadBuffer->Clear_All_Roads();
    }

    if (m_bridgeBuffer != nullptr) {
        m_bridgeBuffer->Clear_All_Bridges();
    }

    if (m_bibBuffer != nullptr) {
        m_bibBuffer->Clear_All_Bibs();
    }

    m_impassableAreas.clear();

    if (m_shroud != nullptr) {
        m_shroud->Reset();
        m_shroud->Set_Border_Shroud_Level(g_theWriteableGlobalData->m_shroudAlpha);
    }
}

void BaseHeightMapRenderObjClass::Notify_Added(SceneClass *scene)
{
    RenderObjClass::Notify_Added(scene);
    scene->Register(this, SceneClass::ON_FRAME_UPDATE);
}

void BaseHeightMapRenderObjClass::Update_Center(CameraClass *camera, RefMultiListIterator<RenderObjClass> *lights)
{
    if (m_map != nullptr && !m_updating) {
        if (m_treeBuffer != nullptr) {
            m_treeBuffer->Do_Full_Update();
        }

        if (m_propBuffer != nullptr) {
            m_propBuffer->Update_Center();
        }

        m_updating = true;

        if (m_roadBuffer != nullptr) {
            m_roadBuffer->Update_Center();
        }

        if (m_needFullUpdate) {
            m_bridgeBuffer->Do_Full_Update();
        }

        m_bridgeBuffer->Update_Center(camera, lights);
        m_updating = false;
    }
}

void BaseHeightMapRenderObjClass::Adjust_Terrain_LOD(int adj)
{
    if (adj > 0 && g_theWriteableGlobalData->m_terrainLOD < TERRAIN_LOD_MAX) {
        g_theWriteableGlobalData->m_terrainLOD++;
    }

    if (adj < 0 && g_theWriteableGlobalData->m_terrainLOD > TERRAIN_LOD_MIN) {
        g_theWriteableGlobalData->m_terrainLOD--;
    }

    if (g_theWriteableGlobalData->m_terrainLOD == TERRAIN_LOD_AUTOMATIC) {
        g_theWriteableGlobalData->m_terrainLOD = TERRAIN_LOD_MAX;
    }

    if (m_map != nullptr) {
        if (m_shroud != nullptr) {
            m_shroud->Reset();
        }

        BaseHeightMapRenderObjClass *heightmap;

        if (g_theWriteableGlobalData->m_terrainLOD == TERRAIN_LOD_MAX) {
            heightmap = g_theHeightMap;

            if (g_theHeightMap == nullptr) {
                heightmap = new HeightMapRenderObjClass();
            }
        } else {
            heightmap = g_theFlatHeightMap;

            if (g_theFlatHeightMap == nullptr) {
                heightmap = new FlatHeightMapRenderObjClass();
            }
        }

        if (g_theWriteableGlobalData->m_terrainLOD == TERRAIN_LOD_STRETCH_CLOUDS) {
            heightmap = nullptr;
        }

        if (m_scene != nullptr) {
            m_scene->Remove_Render_Object(this);
            m_scene->Unregister(this, SceneClass::ON_FRAME_UPDATE);

            if (heightmap != nullptr) {
                m_scene->Add_Render_Object(this);
                m_scene->Register(this, SceneClass::ON_FRAME_UPDATE);
            }
        }

        if (heightmap != nullptr) {
            heightmap->Init_Height_Data(m_map->Get_Draw_Width(), m_map->Get_Draw_Height(), m_map, nullptr, true);
            g_theTerrainRenderObject = heightmap;
            heightmap->Static_Lighting_Changed();
            heightmap->m_roadBuffer->Load_Roads();
        }

        if (g_theTacticalView != nullptr) {
            g_theTacticalView->Set_Angle(g_theTacticalView->Get_Angle() + 1.0f);
            g_theTacticalView->Set_Angle(g_theTacticalView->Get_Angle() - 1.0f);
        }
    }
}

void BaseHeightMapRenderObjClass::Free_Scorch_Buffers()
{
    Ref_Ptr_Release(m_vertexScorch);
    Ref_Ptr_Release(m_indexScorch);
    Ref_Ptr_Release(m_scorchTexture);
}

void BaseHeightMapRenderObjClass::Clear_All_Scorches()
{
    m_numScorches = 0;
    m_scorchesInBuffer = 0;
}

bool BaseHeightMapRenderObjClass::Cast_Ray(RayCollisionTestClass &raytest)
{
    bool hit = false;
    Vector3 normal;
    Vector3 p0;
    Vector3 p1;
    Vector3 p2;
    Vector3 p3;

    if (m_map == nullptr) {
        return false;
    }

    AABoxClass hbox;
    LineSegClass lineseg;
    LineSegClass lineseg2;
    CastResultStruct result;
    int start_cell_x = 0;
    int end_cell_x = 0;
    int start_cell_y = 0;
    int end_cell_y = 0;

    int size = m_map->Border_Size() + 64;
    Vector3 min_pt(-size * 10.0f, -size * 10.0f, -10.0f);
    Vector3 max_pt((size + m_map->Get_X_Extent()) * 10.0f,
        (size + m_map->Get_Y_Extent()) * 10.0f,
        m_map->Get_Max_Height_Value() * HEIGHTMAP_SCALE + 10.0f);

    MinMaxAABoxClass mmbox(min_pt, max_pt);
    hbox.Init(mmbox);
    lineseg = raytest.m_ray;
    p0 = raytest.m_ray.Get_P0();
    p1 = raytest.m_ray.Get_P1();
    result.compute_contact_point = true;

    for (int i = 0; i < 3; i++) {
        result.Reset();
        result.compute_contact_point = true;

        if (!CollisionMath::Collide(lineseg, hbox, &result)) {
            if (i == 0) {
                return false;
            }

            break;
        }

        if (!result.start_bad) {
            p0 = result.contact_point;
        }

        result.fraction = 1.0f;
        result.start_bad = false;
        lineseg2.Set(lineseg.Get_P1(), lineseg.Get_P0());

        if (CollisionMath::Collide(lineseg2, hbox, &result) && !result.start_bad) {
            p1 = result.contact_point;
        }

        if (p0.X > p1.X) {
            start_cell_x = GameMath::Fast_To_Int_Floor(p1.X / 10.0f);
            end_cell_x = GameMath::Fast_To_Int_Ceil(p0.X / 10.0f);
        } else {
            start_cell_x = GameMath::Fast_To_Int_Floor(p0.X / 10.0f);
            end_cell_x = GameMath::Fast_To_Int_Ceil(p1.X / 10.0f);
        }

        if (p0.Y > p1.Y) {
            start_cell_y = GameMath::Fast_To_Int_Floor(p1.Y / 10.0f);
            end_cell_y = GameMath::Fast_To_Int_Ceil(p0.Y / 10.0f);
        } else {
            start_cell_y = GameMath::Fast_To_Int_Floor(p0.Y / 10.0f);
            end_cell_y = GameMath::Fast_To_Int_Ceil(p1.Y / 10.0f);
        }

        int max_ht = m_map->Get_Max_Height_Value();
        int min_ht = 0;

        for (int cell_y = start_cell_y; cell_y <= end_cell_y; cell_y++) {
            for (int cell_x = start_cell_x; cell_x <= end_cell_x; cell_x++) {
                unsigned char clip = Get_Clip_Height(m_map->Border_Size() + cell_x, m_map->Border_Size() + cell_y);

                if (clip < max_ht) {
                    max_ht = clip;
                }

                if (min_ht < clip) {
                    min_ht = clip;
                }
            }
        }

        Vector3 min_pt2((start_cell_x - 1) * 10.0f, (start_cell_y - 1) * 10.0f, (max_ht - 1) * HEIGHTMAP_SCALE);
        Vector3 max_pt2((end_cell_x + 1) * 10.0f, (end_cell_y + 1) * 10.0f, (min_ht + 1) * HEIGHTMAP_SCALE);
        MinMaxAABoxClass mmbox2(min_pt2, max_pt2);
        hbox.Init(mmbox2);
    }

    raytest.m_result->compute_contact_point = true;
    start_cell_x += m_map->Border_Size();
    start_cell_y += m_map->Border_Size();
    end_cell_x += m_map->Border_Size();
    end_cell_y += m_map->Border_Size();

    for (int m = 1; m < 5; m *= 3) {
        for (int y = start_cell_y - m; y <= m + end_cell_y; y++) {
            for (int x = start_cell_x - m; x <= m + end_cell_x; x++) {
                p0.X = (x - m_map->Border_Size()) * 10.0f;
                p0.Y = (y - m_map->Border_Size()) * 10.0f;
                p0.Z = Get_Clip_Height(x, y) * HEIGHTMAP_SCALE;

                p1.X = (x + 1 - m_map->Border_Size()) * 10.0f;
                p1.Y = (y - m_map->Border_Size()) * 10.0f;
                p1.Z = Get_Clip_Height(x + 1, y) * HEIGHTMAP_SCALE;

                p2.X = (x + 1 - m_map->Border_Size()) * 10.0f;
                p2.Y = (y + 1 - m_map->Border_Size()) * 10.0f;
                p2.Z = Get_Clip_Height(x + 1, y + 1) * HEIGHTMAP_SCALE;

                p3.X = (x - m_map->Border_Size()) * 10.0f;
                p3.Y = (y + 1 - m_map->Border_Size()) * 10.0f;
                p3.Z = Get_Clip_Height(x, y + 1) * HEIGHTMAP_SCALE;

                TriClass tri;
                tri.V[0] = &p0;
                tri.V[1] = &p1;
                tri.V[2] = &p2;
                tri.N = &normal;
                tri.Compute_Normal();
                hit |= CollisionMath::Collide(raytest.m_ray, tri, raytest.m_result);

                if (raytest.m_result->start_bad) {
                    return true;
                }

                tri.V[0] = &p2;
                tri.V[1] = &p3;
                tri.V[2] = &p0;
                tri.N = &normal;
                tri.Compute_Normal();
                hit |= CollisionMath::Collide(raytest.m_ray, tri, raytest.m_result);

                if (hit) {
                    raytest.m_result->surface_type = SURFACE_TYPE_DEFAULT;
                }
            }
        }
    }

    return hit;
}

void BaseHeightMapRenderObjClass::Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const
{
    int x = 0;
    int y = 0;

    if (m_map != nullptr) {
        x = m_map->Get_X_Extent();
        y = m_map->Get_Y_Extent();
    }

    Vector3 center(x * 0.5f * 10.0f, y * 0.5f * 10.0f, (m_maxHeight - m_minHeight) * 0.5f + m_minHeight);
    float radius = center.Length();

    if (m_map != nullptr) {
        center.X += m_map->Get_Draw_Origin_X() * 10.0f;
        center.Y += m_map->Get_Draw_Origin_Y() * 10.0f;
    }

    sphere.Init(center, radius);
}

void BaseHeightMapRenderObjClass::Get_Obj_Space_Bounding_Box(AABoxClass &box) const
{
    int x = 0;
    int y = 0;

    if (m_map != nullptr) {
        x = m_map->Get_X_Extent();
        y = m_map->Get_Y_Extent();
    }

    Vector3 min(0.0f, 0.0f, m_minHeight);
    Vector3 max(x * 10.0f, y * 10.0f, m_maxHeight);
    MinMaxAABoxClass mmbox(min, max);
    box.Init(mmbox);
}

int BaseHeightMapRenderObjClass::Init_Height_Data(
    int x, int y, WorldHeightMap *map, RefMultiListIterator<RenderObjClass> *lights, bool update_shoreline)
{
    Ref_Ptr_Set(m_map, map);

    if (m_shroud != nullptr) {
        m_shroud->Init(m_map, g_theWriteableGlobalData->m_partitionCellSize, g_theWriteableGlobalData->m_partitionCellSize);
    }

    m_roadBuffer->Set_Map(m_map);
    unsigned char *data = nullptr;

    if (map != nullptr) {
        data = map->Get_Data_Ptr();
    }

    if (m_treeBuffer != nullptr) {
        Region2D region;
        region.lo.x = 0.0f;
        region.lo.y = 0.0f;
        region.hi.x = (map->Get_X_Extent() - 2 * map->Get_Border_Size()) * 10.0f;
        region.hi.y = (map->Get_Y_Extent() - 2 * map->Get_Border_Size()) * 10.0f;
        m_treeBuffer->Set_Partition_Region(&region);
    }

    if (update_shoreline) {
        m_numShorelineBlendTiles = 0;

        if (map != nullptr) {
            int x_extent = map->Get_X_Extent();
            int y_extent = map->Get_Y_Extent();
            int min_height = WorldHeightMap::Get_Max_Height_Value();
            int max_height = 0;

            for (int y_pos = 0; y_pos < y_extent; y_pos++) {
                for (int x_pos = 0; x_pos < x_extent; x_pos++) {
                    unsigned char height = map->Get_Height(x_pos, y_pos);

                    if (height < min_height) {
                        min_height = height;
                    }

                    if (max_height < height) {
                        max_height = height;
                    }
                }
            }

            m_minHeight = min_height * HEIGHTMAP_SCALE;
            m_maxHeight = max_height * HEIGHTMAP_SCALE;
            Update_Shoreline_Tiles(0, 0, x_extent - 1, y_extent - 1, map);

            if (g_theWaterTransparency->m_transparentWaterMinOpacity != m_transparentWaterMinOpacity) {
                Init_Dest_Alpha_LUT();
            }
        }
    }

    Set_Force_Visible(true);
    m_needFullUpdate = true;
    m_scorchesInBuffer = 0;
    m_curNumScorchVertices = 0;
    m_curNumScorchIndices = 0;

    if (data != nullptr && m_stageTwoTexture == nullptr) {
        Free_Map_Resources();
        Ref_Ptr_Set(m_map, map);
        m_stageTwoTexture = new CloudMapTerrainTextureClass(MIP_LEVELS_ALL);
        m_stageThreeTexture = new LightMapTerrainTextureClass(m_macroTextureName, MIP_LEVELS_ALL);
        m_destAlphaLUT = new TextureClass(256, 1, WW3D_FORMAT_A8R8G8B8, MIP_LEVELS_1, POOL_MANAGED, false, true);
        Init_Dest_Alpha_LUT();
        Allocate_Scorch_Buffers();
        m_vertexMaterialClass = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
        m_shaderClass = s_detailOpaqueShader;
    }

    return 0;
}

void BaseHeightMapRenderObjClass::Static_Lighting_Changed()
{
    m_needFullUpdate = true;
    m_scorchesInBuffer = 0;
    m_curNumScorchVertices = 0;
    m_curNumScorchIndices = 0;
    m_roadBuffer->Update_Lighting();
}

void BaseHeightMapRenderObjClass::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    xfer->xferSnapshot(m_treeBuffer);
    xfer->xferSnapshot(m_propBuffer);
}

int BaseHeightMapRenderObjClass::Get_Static_Diffuse(int x, int y)
{
    if (x < 0) {
        x = 0;
    }

    if (y < 0) {
        y = 0;
    }

    if (x >= m_map->Get_X_Extent()) {
        x = m_map->Get_X_Extent() - 1;
    }

    if (y >= m_map->Get_Y_Extent()) {
        y = m_map->Get_Y_Extent() - 1;
    }

    if (m_map == nullptr) {
        return 0;
    }

    Vector3 v1;
    Vector3 v2;
    Vector3 normal;

    int y1 = y - 1;
    int y2 = y + 1;

    if (y2 >= m_map->Get_Y_Extent()) {
        y2 = m_map->Get_Y_Extent() - 1;
    }

    if (y1 < 0) {
        y1 = 0;
    }

    int x1 = x - 1;
    int x2 = x + 1;

    if (x2 >= m_map->Get_X_Extent()) {
        x2 = m_map->Get_X_Extent() - 1;
    }

    if (x1 < 0) {
        x1 = 0;
    }

    Vector3 light[LIGHT_COUNT];

    for (int i = 0; i < g_theWriteableGlobalData->m_numberGlobalLights; i++) {
        light[i].Set(-g_theWriteableGlobalData->m_terrainLightPos[i].x,
            -g_theWriteableGlobalData->m_terrainLightPos[i].y,
            -g_theWriteableGlobalData->m_terrainLightPos[i].z);
    }

    v1.Set(20.0f, 0.0f, (m_map->Get_Height(x2, y) - m_map->Get_Height(x1, y)) * HEIGHTMAP_SCALE);
    v2.Set(0.0f, 20.0f, (m_map->Get_Height(x, y2) - m_map->Get_Height(x, y1)) * HEIGHTMAP_SCALE);
    Vector3::Normalized_Cross_Product(v1, v2, &normal);

    VertexFormatXYZDUV2 vb;
    vb.x = (x - m_map->Border_Size()) * 10.0f;
    vb.y = (y - m_map->Border_Size()) * 10.0f;
    vb.z = m_map->Get_Height(x, y) * HEIGHTMAP_SCALE;
    vb.u1 = 0.0f;
    vb.v1 = 0.0f;
    vb.u2 = 1.0f;
    vb.v2 = 1.0f;

    RTS3DScene *scene = static_cast<RTS3DScene *>(m_scene);

    if (scene != nullptr) {
        RefMultiListIterator<RenderObjClass> *lights = scene->Create_Lights_Iterator();
        Do_The_Light(&vb, light, &normal, lights, 1);

        if (lights != nullptr) {
            scene->Destroy_Lights_Iterator(lights);
        }
    } else {
        Do_The_Light(&vb, light, &normal, nullptr, 1);
    }

    return vb.diffuse;
}

float BaseHeightMapRenderObjClass::Get_Max_Cell_Height(float x, float y) const
{
    if (m_map == nullptr) {
        return 0.0f;
    }

    WorldHeightMap *map;

    if (g_theTerrainVisual != nullptr) {
        map = g_theTerrainVisual->Get_Logic_Height_Map();
    } else {
        map = m_map;
    }

    int x2 = map->Border_Size() + (x / 10.0f);
    int y2 = map->Border_Size() + (y / 10.0f);

    if (x2 < 0) {
        x2 = 0;
    }

    if (y2 < 0) {
        y2 = 0;
    }

    if (x2 >= map->Get_X_Extent() - 1) {
        x2 = map->Get_X_Extent() - 2;
    }

    if (y2 >= map->Get_Y_Extent() - 1) {
        y2 = map->Get_Y_Extent() - 2;
    }

    unsigned char *data = map->Get_Data_Ptr();
    float p1 = data[m_map->Get_X_Extent() * y2 + x2] * HEIGHTMAP_SCALE;
    float p2 = data[m_map->Get_X_Extent() * y2 + (x2 + 1)] * HEIGHTMAP_SCALE;
    float p3 = data[m_map->Get_X_Extent() * (y2 + 1) + (x2 + 1)] * HEIGHTMAP_SCALE;
    float p4 = data[m_map->Get_X_Extent() * (y2 + 1) + x2] * HEIGHTMAP_SCALE;
    float r1;
    float r2;

    if (p1 <= p2) {
        r1 = p2;
    } else {
        r1 = p1;
    }

    if (r1 <= p3) {
        r2 = p3;
    } else {
        r2 = p1;
    }

    if (r2 <= p4) {
        return p4;
    } else {
        return r2;
    }
}

void BaseHeightMapRenderObjClass::Set_Time_Of_Day(TimeOfDayType time)
{
    Static_Lighting_Changed();
}

void BaseHeightMapRenderObjClass::Do_The_Light(VertexFormatXYZDUV2 *vb,
    Vector3 *light,
    Vector3 *normal,
    RefMultiListIterator<RenderObjClass> *lights,
    unsigned char alpha)
{
    float shade_r = g_theWriteableGlobalData->m_terrainAmbient[0].red;
    float shade_g = g_theWriteableGlobalData->m_terrainAmbient[0].green;
    float shade_b = g_theWriteableGlobalData->m_terrainAmbient[0].blue;

    if (lights != nullptr) {
        for (lights->First(); !lights->Is_Done(); lights->Next()) {
            LightClass *obj = static_cast<LightClass *>(lights->Peek_Obj());
            Vector3 light_dir(vb->x, vb->y, vb->z);
            float ambient_factor = 1.0f;

            switch (obj->Get_Type()) {
                case LightClass::POINT:
                case LightClass::SPOT: {
                    Vector3 light_loc = obj->Get_Position();
                    light_dir -= light_loc;
                    double far_start;
                    double far_end;
                    obj->Get_Far_Attenuation_Range(far_start, far_end);

                    if (light_loc.X - far_end > vb->x) {
                        continue;
                    }

                    if (light_loc.X + far_end < vb->x) {
                        continue;
                    }

                    if (light_loc.Y - far_end > vb->y) {
                        continue;
                    }

                    if (light_loc.Y + far_end < vb->y) {
                        continue;
                    }

                    float dist = light_dir.Length();

                    if (dist >= far_end || far_start < 0.1f) {
                        continue;
                    }

                    ambient_factor = 1.0f - (dist - far_start) / (far_end - far_start);
                    ambient_factor = std::clamp(ambient_factor, 0.0f, 1.0f);
                    break;
                }
                case LightClass::DIRECTIONAL: {
                    light_dir = obj->Get_Transform().Get_Z_Vector();
                    ambient_factor = 1.0f;
                    break;
                }
            }

            light_dir.Normalize();
            Vector3 light_dir_neg(-light_dir.X, -light_dir.Y, -light_dir.Z);
            float diffuse_factor = (light_dir_neg * *normal) * ambient_factor;

            Vector3 diffuse;
            obj->Get_Diffuse(&diffuse);

            Vector3 ambient;
            obj->Get_Ambient(&ambient);

            if (diffuse_factor > 1.0f) {
                diffuse_factor = 1.0f;
            }

            if (diffuse_factor < 0.0f) {
                diffuse_factor = 0.0f;
            }

            shade_r += diffuse_factor * diffuse.X;
            shade_g += diffuse_factor * diffuse.Y;
            shade_b += diffuse_factor * diffuse.Z;
            shade_r += ambient_factor * ambient.X;
            shade_g += ambient_factor * ambient.Y;
            shade_b += ambient_factor * ambient.Z;
        }
    }

    for (int i = 0; i < g_theWriteableGlobalData->m_numberGlobalLights; i++) {
        float diffuse_factor = light[i] * *normal;

        if (diffuse_factor > 1.0f) {
            diffuse_factor = 1.0f;
        }

        if (diffuse_factor < 0.0f) {
            diffuse_factor = 0.0f;
        }

        shade_r += diffuse_factor * g_theWriteableGlobalData->m_terrainDiffuse[i].red;
        shade_g += diffuse_factor * g_theWriteableGlobalData->m_terrainDiffuse[i].green;
        shade_b += diffuse_factor * g_theWriteableGlobalData->m_terrainDiffuse[i].blue;
    }

    if (shade_r > 1.0f) {
        shade_r = 1.0f;
    }

    if (shade_r < 0.0f) {
        shade_r = 0.0f;
    }

    if (shade_g > 1.0f) {
        shade_g = 1.0f;
    }

    if (shade_g < 0.0f) {
        shade_g = 0.0f;
    }

    if (shade_b > 1.0f) {
        shade_b = 1.0f;
    }

    if (shade_b < 0.0f) {
        shade_b = 0.0f;
    }

    if (m_useDepthFade && vb->z <= g_theWriteableGlobalData->m_waterPositionZ) {
        float fade = (1.4f - vb->z) / g_theWriteableGlobalData->m_waterPositionZ;
        shade_r = (1.0f - (1.0f - m_depthFade.X) * fade) * shade_r;
        shade_g = (1.0f - (1.0f - m_depthFade.Y) * fade) * shade_g;
        shade_b = (1.0f - (1.0f - m_depthFade.Z) * fade) * shade_b;
    }

    shade_r = shade_r * 255.0f;
    shade_g = shade_g * 255.0f;
    shade_b = shade_b * 255.0f;

    vb->diffuse = Make_Color(GameMath::Fast_To_Int_Truncate(shade_r),
        GameMath::Fast_To_Int_Truncate(shade_g),
        GameMath::Fast_To_Int_Truncate(shade_b),
        alpha);
}

float BaseHeightMapRenderObjClass::Get_Height_Map_Height(float x, float y, Coord3D *pos) const
{
    WorldHeightMap *height_map;

    // Check if terrain visual is available, otherwise use default map
    if (g_theTerrainVisual != nullptr) {
        height_map = g_theTerrainVisual->Get_Logic_Height_Map();
    } else {
        height_map = m_map;
    }

    if (height_map != nullptr) {
        float scaled_x = x * 0.1f;
        float scaled_y = y * 0.1f;
        float floor_x = GameMath::Fast_Float_Floor(scaled_x);
        float floor_y = GameMath::Fast_Float_Floor(scaled_y);
        float fract_x = scaled_x - floor_x;
        float fract_y = scaled_y - floor_y;
        int index_x = height_map->Border_Size() + GameMath::Lrintf(floor_x);
        int index_y = height_map->Border_Size() + GameMath::Lrintf(floor_y);
        int map_width = height_map->Get_X_Extent();

        // Check if the indices are within valid bounds
        if (index_x <= map_width - 3 && index_y <= height_map->Get_Y_Extent() - 3 && index_y >= 1 && index_x >= 1) {
            unsigned char *height_data = height_map->Get_Data_Ptr();
            int current_point_index = map_width * index_y + index_x;
            float current_point_height = height_data[current_point_index];
            float top_right_neighbor_height = height_data[map_width + 1 + current_point_index];
            float interpolated_height;

            // Calculate the height based on interpolation
            // If fractional y is less than or equal to fractional x, our point is in the top right triangle of the cell
            if (fract_y <= fract_x) {
                float right_neighbor_height = height_data[current_point_index + 1];
                interpolated_height = ((top_right_neighbor_height - right_neighbor_height) * fract_y + right_neighbor_height
                                          + (1.0f - fract_x) * (current_point_height - right_neighbor_height))
                    * HEIGHTMAP_SCALE;
            } else {
                float top_neighbor_height = height_data[map_width + current_point_index];
                interpolated_height = ((1.0f - fract_y) * (current_point_height - top_neighbor_height) + top_neighbor_height
                                          + (top_right_neighbor_height - top_neighbor_height) * fract_x)
                    * HEIGHTMAP_SCALE;
            }

            // Calculate the normal vector if pos is provided
            if (pos != nullptr) {
                int prev_row_cell_index = map_width * (index_y - 1) + index_x;
                int current_cell_index = map_width * index_y + index_x;
                int next_row_cell_index = map_width + current_cell_index;
                int next_next_row_cell_index = map_width * (index_y + 2) + index_x;
                unsigned char current_cell_height_data = height_data[current_cell_index];
                unsigned char right = height_data[current_cell_index + 1];
                unsigned char up_right = height_data[map_width + 1 + current_cell_index];
                unsigned char up = height_data[map_width + current_cell_index];
                unsigned char down = height_data[prev_row_cell_index];
                unsigned char down_right = height_data[prev_row_cell_index + 1];
                unsigned char up_up_right = height_data[next_next_row_cell_index + 1];
                unsigned char up_up = height_data[next_next_row_cell_index];
                float slope_1 = right - height_data[current_cell_index - 1];
                float slope_2 = height_data[current_cell_index + 2] - current_cell_height_data;
                float slope_3 = height_data[map_width + 2 + current_cell_index] - up;
                float slope_4 = up - down;
                float slope_5 = up_right - down_right;
                float slope_6 = up_up_right - right;
                float slope_7 = up_up - current_cell_height_data;
                float interpolated_slope_x_difference = (1.0f - fract_x) * slope_1 + fract_x * slope_2;
                float interpolated_slope_y_difference = (1.0f - fract_x) * slope_2 + fract_x * slope_3;
                float interpolated_normal_x =
                    interpolated_slope_x_difference * (1.0f - fract_y) + fract_y * interpolated_slope_y_difference;
                float interpolated_height_x_difference = (1.0f - fract_x) * slope_4 + fract_x * slope_7;
                float interpolated_height_y_difference = (1.0f - fract_x) * slope_5 + fract_x * slope_6;
                float interpolated_normal_y =
                    interpolated_height_x_difference * (1.0f - fract_y) + fract_y * interpolated_height_y_difference;
                Vector3 v_x_axis;
                Vector3 v_y_axis;
                Vector3 normal_vector;
                v_x_axis.Set(32.0f, 0.0f, interpolated_normal_x);
                v_y_axis.Set(0.0f, 32.0f, interpolated_normal_y);
                Vector3::Normalized_Cross_Product(v_x_axis, v_y_axis, &normal_vector);
                pos->x = normal_vector.X;
                pos->y = normal_vector.Y;
                pos->z = normal_vector.Z;
            }

            return interpolated_height;
        } else {
            if (pos != nullptr) {
                pos->x = 0.0f;
                pos->y = 0.0f;
                pos->z = 1.0f;
            }

            return Get_Clip_Height(index_x, index_y) * HEIGHTMAP_SCALE;
        }
    } else {
        if (pos != nullptr) {
            pos->x = 0.0f;
            pos->y = 0.0f;
            pos->z = 1.0f;
        }

        return 0.0f;
    }
}

void Do_Trees(RenderInfoClass &rinfo)
{
    if (g_theTerrainRenderObject != nullptr) {
        g_theTerrainRenderObject->Render_Trees(&rinfo.m_camera);
    }
}

void BaseHeightMapRenderObjClass::Notify_Shroud_Changed()
{
    if (m_propBuffer != nullptr) {
        m_propBuffer->Notify_Shroud_Changed();
    }
}

void BaseHeightMapRenderObjClass::Add_Tree(
    DrawableID drawable, Coord3D location, float scale, float angle, float random, W3DTreeDrawModuleData const *module)
{
    if (m_treeBuffer != nullptr) {
        m_treeBuffer->Add_Tree(drawable, location, scale, angle, random, module);
    }
}

void BaseHeightMapRenderObjClass::Add_Prop(int id, Coord3D location, float orientation, float scale, const Utf8String &name)
{
    if (m_propBuffer != nullptr) {
        m_propBuffer->Add_Prop(id, location, orientation, scale, name);
    }
}

void BaseHeightMapRenderObjClass::Add_Scorch(Vector3 location, float radius, Scorches type)
{
    if (m_numScorches >= 500) {
        for (int i = 0; i < 499; i++) {
            m_scorches[i] = m_scorches[i + 1];
        }

        m_numScorches--;
    }

    for (int i = 0; i < m_numScorches; i++) {
        if (GameMath::Fabs(location.X - m_scorches[i].m_location.X) < radius / 4.0f
            && GameMath::Fabs(location.Y - m_scorches[i].m_location.Y) < radius / 4.0f
            && GameMath::Fabs(radius - m_scorches[i].m_radius) < radius / 4.0f && m_scorches[i].m_scorchType == type) {
            return;
        }
    }

    m_scorches[m_numScorches].m_location = location;
    m_scorches[m_numScorches].m_radius = radius;
    m_scorches[m_numScorches].m_scorchType = type;
    m_numScorches++;
    m_scorchesInBuffer = 0;
}

void BaseHeightMapRenderObjClass::Add_Terrain_Bib(Vector3 *points, ObjectID id, bool highlighted)
{
    m_bibBuffer->Add_Bib_To_Object(points, id, highlighted);
}

void BaseHeightMapRenderObjClass::Add_Terrain_Bib_Drawable(Vector3 *points, DrawableID id, bool highlighted)
{
    m_bibBuffer->Add_Bib_To_Drawable(points, id, highlighted);
}

void BaseHeightMapRenderObjClass::Allocate_Scorch_Buffers()
{
    m_vertexScorch = new DX8VertexBufferClass(DX8_FVF_XYZDUV1, 8194, DX8VertexBufferClass::USAGE_DEFAULT, 0);
    m_indexScorch = new DX8IndexBufferClass(49164, DX8IndexBufferClass::USAGE_DEFAULT);
    m_scorchTexture = new ScorchTextureClass(MIP_LEVELS_3);
    m_scorchesInBuffer = 0;
    m_curNumScorchVertices = 0;
    m_curNumScorchIndices = 0;
}

void BaseHeightMapRenderObjClass::Draw_Scorches()
{
    Update_Scorches();

    if (m_curNumScorchIndices != 0) {
        DX8Wrapper::Set_Index_Buffer(m_indexScorch, 0);
        DX8Wrapper::Set_Vertex_Buffer(m_vertexScorch, 0);
        DX8Wrapper::Set_Shader(ShaderClass::s_presetAlphaShader);
        DX8Wrapper::Set_Texture(0, m_scorchTexture);

        if (!Is_Hidden()) {
            DX8Wrapper::Draw_Triangles(0, m_curNumScorchIndices / 3, 0, m_curNumScorchVertices);
        }
    }
}

bool BaseHeightMapRenderObjClass::Evaluate_As_Visible_Cliff(int x, int y, float height)
{
    static float distance[4] = { 0.0f, 10.0f, GameMath::Sqrt(2.0f) * 10.0f, 10.0f };
    unsigned char heights[4];
    float fheights[4];

    heights[0] = m_map->Get_Height(x, y);
    heights[1] = m_map->Get_Height(x + 1, y);
    heights[2] = m_map->Get_Height(x + 1, y + 1);
    heights[3] = m_map->Get_Height(x, y + 1);

    fheights[0] = heights[0] * HEIGHTMAP_SCALE;
    fheights[1] = heights[1] * HEIGHTMAP_SCALE;
    fheights[2] = heights[2] * HEIGHTMAP_SCALE;
    fheights[3] = heights[3] * HEIGHTMAP_SCALE;

    bool match = 0;

    for (int i = 1; i < 4 && !match; i++) {
        match = height < GameMath::Fabs((fheights[i] - fheights[0]) / distance[i]);
    }

    return match;
}

bool BaseHeightMapRenderObjClass::Get_Maximum_Visible_Box(
    const FrustumClass &frustum, AABoxClass *box, bool ignore_max_height)
{
    PlaneClass plane(Vector3(0.0f, 0.0f, 1.0f), m_minHeight);
    Vector3 corners[8];
    corners[0] = frustum.m_corners[0];

    for (int i = 0; i < 4; i++) {
        corners[i] = frustum.m_corners[i];
        float k;

        if (plane.Compute_Intersection(frustum.m_corners[i], frustum.m_corners[i + 4], &k)) {
            corners[i + 4] = frustum.m_corners[i] + (frustum.m_corners[i + 4] - frustum.m_corners[i]) * k;
        } else {
            corners[i + 4] = frustum.m_corners[i + 4];
        }
    }

    if (box != nullptr) {
        box->Init(corners, 8);
    }

    return true;
}

void BaseHeightMapRenderObjClass::Init_Dest_Alpha_LUT()
{
    if (m_destAlphaLUT != nullptr) {
        SurfaceClass *surface = m_destAlphaLUT->Get_Surface_Level(0);

        if (surface != nullptr) {
            int pitch;
            unsigned int *buf = static_cast<unsigned int *>(surface->Lock(&pitch));
            int opacity = g_theWaterTransparency->m_transparentWaterMinOpacity * 255.0f;

            if (buf != nullptr) {
                for (int i = 0; i < 256; i++) {
                    int alpha = i;

                    if (i > opacity) {
                        alpha = opacity;
                    }

                    *buf = (alpha << 24) | 0xFFFFFF;
                    buf++;
                }

                surface->Unlock();
            }

            m_destAlphaLUT->Get_Texture_Filter()->Set_U_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
            m_destAlphaLUT->Get_Texture_Filter()->Set_V_Address_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
            Ref_Ptr_Release(surface);
            m_transparentWaterMinOpacity = g_theWaterTransparency->m_transparentWaterMinOpacity;
        }
    }
}

/**
 * Checks if there is a clear line of sight between two positions on the height map.
 *
 * @param pos1 The starting position.
 * @param pos2 The ending position.
 * @return True if there is a clear line of sight, false otherwise.
 */
bool BaseHeightMapRenderObjClass::Is_Clear_Line_Of_Sight(const Coord3D &pos1, const Coord3D &pos2) const
{
    // Check if the height map is valid
    if (m_map == nullptr) {
        return false;
    }

    WorldHeightMap *height_map;

    // Get the appropriate height map based on the terrain visual
    if (g_theTerrainVisual != nullptr) {
        height_map = g_theTerrainVisual->Get_Logic_Height_Map();
    } else {
        height_map = m_map;
    }

    int border = height_map->Border_Size();
    int x1 = border + GameMath::Fast_To_Int_Floor(0.1f * pos1.x);
    int y1 = border + GameMath::Fast_To_Int_Floor(0.1f * pos1.y);
    int x2 = border + GameMath::Fast_To_Int_Floor(0.1f * pos2.x);
    int y2 = border + GameMath::Fast_To_Int_Floor(0.1f * pos2.y);
    int x_dist = abs(x2 - x1);
    int y_dist = abs(y2 - y1);
    int current_x_pos = x1;
    int current_y_pos = y1;
    int x_increment_1;
    int x_increment_2;

    // Determine the increments based on the direction of the line
    if (x2 < x1) {
        x_increment_1 = -1;
        x_increment_2 = -1;
    } else {
        x_increment_1 = 1;
        x_increment_2 = 1;
    }

    int y_increment_1;
    int y_increment_2;

    if (y2 < y1) {
        y_increment_1 = -1;
        y_increment_2 = -1;
    } else {
        y_increment_1 = 1;
        y_increment_2 = 1;
    }

    int major_distance;
    int minor_distance;
    int minor_extent;
    int major_extent;

    // Adjust the variables for steeper lines
    if (x_dist < y_dist) {
        x_increment_2 = 0;
        y_increment_1 = 0;
        major_distance = y_dist;
        minor_distance = y_dist / 2;
        minor_extent = x_dist;
        major_extent = y_dist;
    } else {
        x_increment_1 = 0;
        y_increment_2 = 0;
        major_distance = x_dist;
        minor_distance = x_dist / 2;
        minor_extent = y_dist;
        major_extent = x_dist;
    }

    float current_height = pos1.z;
    float height_increment = (pos2.z - current_height) * (1.0f / major_extent);
    unsigned char *height_data = height_map->Get_Data_Ptr();
    int x_extent = height_map->Get_X_Extent();
    int y_extent = height_map->Get_Y_Extent();

    // Iterate over the height map to check for obstructions
    for (int i = 0; i < major_extent && current_x_pos >= 0 && current_y_pos >= 0 && current_x_pos < x_extent - 1
         && current_y_pos < y_extent - 1;
         i++) {
        int map_index = x_extent * current_y_pos + current_x_pos;
        float max_height_1;

        // Find the maximum value among neighboring height map data (current, right, down, and diagonal)
        if (height_data[map_index + 1] >= height_data[map_index]) {
            max_height_1 = height_data[map_index + 1];
        } else {
            max_height_1 = height_data[map_index];
        }

        float max_height_2;

        if (height_data[x_extent + map_index] >= max_height_1) {
            max_height_2 = height_data[x_extent + map_index];
        } else {
            max_height_2 = max_height_1;
        }

        float max_height_3;

        if (height_data[x_extent + 1 + map_index] >= max_height_2) {
            max_height_3 = height_data[x_extent + 1 + map_index];
        } else {
            max_height_3 = max_height_2;
        }

        // Check if there is an obstruction in the line of sight
        if (current_height + 0.5f < max_height_3 * HEIGHTMAP_SCALE) {
            return false;
        }

        // Check if the viewer is looking upwards and the maximum height is reached
        if (Get_Max_Height() <= current_height && height_increment > 0.0f) {
            return true;
        }

        // Update the height and increment variables
        current_height += height_increment;
        minor_distance += minor_extent;

        // Adjust positions based on the increments
        if (minor_distance >= major_distance) {
            minor_distance -= major_distance;
            current_x_pos += x_increment_1;
            current_y_pos += y_increment_1;
        }
        current_x_pos += x_increment_2;
        current_y_pos += y_increment_2;
    }

    return true;
}

bool BaseHeightMapRenderObjClass::Is_Cliff_Cell(float x, float y)
{
    if (m_map == nullptr) {
        return false;
    }

    WorldHeightMap *map;

    if (g_theTerrainVisual != nullptr) {
        map = g_theTerrainVisual->Get_Logic_Height_Map();
    } else {
        map = m_map;
    }

    int x_index = map->Border_Size() + (x / 10.0f);
    int y_index = map->Border_Size() + (y / 10.0f);

    if (x_index < 0) {
        x_index = 0;
    }

    if (y_index < 0) {
        y_index = 0;
    }

    if (x_index >= map->Get_X_Extent() - 1) {
        x_index = map->Get_X_Extent() - 2;
    }

    if (y_index >= map->Get_Y_Extent() - 1) {
        y_index = map->Get_Y_Extent() - 2;
    }
    return map->Get_Cliff_State(x_index, y_index);
}

void BaseHeightMapRenderObjClass::Load_Roads_And_Bridges(W3DTerrainLogic *pTerrainLogic, bool unk)
{
#ifdef BUILD_WITH_D3D8
    if (DX8Wrapper::Get_D3D_Device8() == nullptr || DX8Wrapper::Get_D3D_Device8()->TestCooperativeLevel() == 0) {
        if (m_roadBuffer != nullptr) {
            m_roadBuffer->Load_Roads();
        }

        if (m_bridgeBuffer != nullptr) {
            m_bridgeBuffer->Load_Bridges(pTerrainLogic, unk);
        }
    }
#endif
}

void BaseHeightMapRenderObjClass::Record_Shore_Line_Sort_Infos()
{
    if (!g_theWriteableGlobalData->m_unsortedShoreLines && m_shorelineTiles != nullptr && m_map != nullptr) {
        int count = m_map->Get_X_Extent() - 1;
        int axis = 1;

        if (count <= m_map->Get_Y_Extent() - 1) {
            count = m_map->Get_Y_Extent() - 1;
            axis = 0;
        }

        m_sortAxis = axis;

        if (m_shorelineSortInfos == nullptr || count > m_shorelineSortInfoCount) {
            if (m_shorelineSortInfos != nullptr) {
                delete[] m_shorelineSortInfos;
            }

            m_shorelineSortInfoCount = count;
            m_shorelineSortInfos = new ShorelineSortInfo[count];
        }

        memset(m_shorelineSortInfos, 0, sizeof(ShorelineSortInfo) * m_shorelineSortInfoCount);

        if (m_sortAxis != 0) {
            m_startCell = m_shorelineTiles[0].index & 0xFFFF;
            m_endCell = m_startCell;
            ShorelineSortInfo *info;

            for (int i = 0; i < m_numShorelineBlendTiles; i += info->unk2) {
                int index = m_shorelineTiles[i].index & 0xFFFF;
                info = &m_shorelineSortInfos[index];

                if (index > m_endCell) {
                    m_endCell = index;
                }

                int i1 = i + 1;
                int i2 = m_shorelineTiles[i].index >> 16;
                int i3 = i2;

                while ((m_shorelineTiles[i1].index & 0xFFFF) == index && i1 < m_numShorelineBlendTiles) {
                    if (m_shorelineTiles[i1].index >> 16 > i2) {
                        i2 = m_shorelineTiles[i1].index >> 16;
                    }

                    i1++;
                }

                info->unk1 = i;
                info->unk2 = i1 - i;
                info->unk3 = i3;
                info->unk4 = i2;
            }
        } else {
            m_startCell = m_shorelineTiles[0].index >> 16;
            m_endCell = m_startCell;
            ShorelineSortInfo *info;

            for (int i = 0; i < m_numShorelineBlendTiles; i += info->unk2) {
                int index = m_shorelineTiles[i].index >> 16;
                info = &m_shorelineSortInfos[index];

                if (index > m_endCell) {
                    m_endCell = index;
                }

                int i1 = i + 1;
                int i2 = m_shorelineTiles[i].index & 0xFFFF;
                int i3 = i2;

                while ((m_shorelineTiles[i1].index >> 16) == index && i1 < m_numShorelineBlendTiles) {
                    if ((m_shorelineTiles[i1].index & 0xFFFF) > i2) {
                        i2 = m_shorelineTiles[i1].index & 0xFFFF;
                    }

                    i1++;
                }

                info->unk1 = i;
                info->unk2 = i1 - i;
                info->unk3 = i3;
                info->unk4 = i2;
            }
        }
    }
}

void BaseHeightMapRenderObjClass::Remove_All_Props()
{
    if (m_propBuffer != nullptr) {
        m_propBuffer->Clear_All_Props();
    }
}

void BaseHeightMapRenderObjClass::Remove_All_Terrain_Bibs()
{
    m_bibBuffer->Clear_All_Bibs();
}

void BaseHeightMapRenderObjClass::Remove_All_Trees()
{
    if (m_treeBuffer != nullptr) {
        m_treeBuffer->Clear_All_Trees();
    }
}

void BaseHeightMapRenderObjClass::Remove_Prop(int id)
{
    if (m_propBuffer != nullptr) {
        m_propBuffer->Remove_Prop(id);
    }
}

void BaseHeightMapRenderObjClass::Remove_Terrain_Bib(ObjectID id)
{
    m_bibBuffer->Remove_Bib_From_Object(id);
}

void BaseHeightMapRenderObjClass::Remove_Terrain_Bib_Drawable(DrawableID id)
{
    m_bibBuffer->Remove_Bib_From_Drawable(id);
}

void BaseHeightMapRenderObjClass::Remove_Terrain_Bib_Highlighting()
{
    m_bibBuffer->Remove_Highlighting();
}

void BaseHeightMapRenderObjClass::Remove_Tree(DrawableID id)
{
    if (m_treeBuffer != nullptr) {
        m_treeBuffer->Remove_Tree(id);
    }
}

void BaseHeightMapRenderObjClass::Remove_Trees_And_Props_For_Construction(
    const Coord3D *pos, const GeometryInfo &geom, float angle)
{
    if (m_treeBuffer != nullptr) {
        m_treeBuffer->Remove_Trees_For_Construction(pos, geom, angle);
    }

    if (m_propBuffer != nullptr) {
        m_propBuffer->Remove_Props_For_Construction(pos, geom, angle);
    }
}

void BaseHeightMapRenderObjClass::Render_Shore_Lines(CameraClass *camera)
{
#ifdef BUILD_WITH_D3D8
    if (g_theWriteableGlobalData->m_unsortedShoreLines) {
        m_shorelineBlendTilesRendered = 0;

        if (g_theWriteableGlobalData->m_showSoftWaterEdge && g_theWaterTransparency->m_transparentWaterDepth != 0.0f
            && m_numShorelineBlendTiles != 0 && DX8Wrapper::Get_Back_Buffer_Format() == WW3D_FORMAT_A8R8G8B8) {
            int vertex_count = 0;
            int poly_count = 0;
            int y = m_map->Get_Draw_Origin_Y() + m_map->Get_Draw_Height() - 1;
            int x = m_map->Get_Draw_Origin_X() + m_map->Get_Draw_Width() - 1;

            if (x > m_map->Get_X_Extent() - 1) {
                x = m_map->Get_X_Extent() - 1;
            }

            if (y > m_map->Get_Y_Extent() - 1) {
                y = m_map->Get_Y_Extent() - 1;
            }

            int origin_x = m_map->Get_Draw_Origin_X();
            int origin_y = m_map->Get_Draw_Origin_Y();
            int index = 0;
            ShaderClass shader(ShaderClass::s_presetOpaque2DShader);
            shader.Set_Depth_Compare(ShaderClass::PASS_LEQUAL);
            DX8Wrapper::Set_Shader(shader);
            VertexMaterialClass *material = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
            DX8Wrapper::Set_Material(material);
            Ref_Ptr_Release(material);
            DX8Wrapper::Set_Texture(0, m_destAlphaLUT);
            Matrix3D tm(true);
            DX8Wrapper::Set_Transform(D3DTS_WORLD, tm);
            DX8Wrapper::Set_DX8_Render_State(D3DRS_COLORWRITEENABLE, 8);
            DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXCOORDINDEX, 0);

            while (index != m_numShorelineBlendTiles) {
                DynamicVBAccessClass vb(VertexBufferClass::BUFFER_TYPE_DYNAMIC_DX8, DX8_FVF_XYZNDUV2, 2048);
                DynamicIBAccessClass ib(IndexBufferClass::BUFFER_TYPE_DYNAMIC_DX8, 3072);

                {
                    DynamicVBAccessClass::WriteLockClass vb_lock(&vb);
                    VertexFormatXYZNDUV2 *vertices = vb_lock.Get_Formatted_Vertex_Array();
                    DynamicIBAccessClass::WriteLockClass ib_lock(&ib);
                    unsigned short *indices = ib_lock.Get_Index_Array();

                    if (indices == nullptr || vertices == nullptr) {
                        DX8Wrapper::Set_DX8_Render_State(D3DRS_COLORWRITEENABLE, 7);
                        return;
                    }

                    while (index < m_numShorelineBlendTiles && vertex_count < 2048) {
                        ShorelineTile *tile = &m_shorelineTiles[index];
                        int index_x = tile->index & 0xFFFF;
                        int index_y = tile->index >> 16;

                        if (index_x >= origin_x && index_x < x && index_y >= origin_y && index_y < y) {
                            vertices->x = tile->x1;
                            vertices->y = tile->y1;
                            vertices->z = tile->z1;
                            vertices->nx = 0.0f;
                            vertices->ny = 0.0f;
                            vertices->nz = 0.0f;
                            vertices->diffuse = 0;
                            vertices->u1 = tile->u1;
                            vertices->v1 = 0.0f;
                            vertices->u2 = 0.0f;
                            vertices->v2 = 0.0f;
                            vertices++;

                            vertices->x = tile->x2;
                            vertices->y = tile->y2;
                            vertices->z = tile->z2;
                            vertices->nx = 0.0f;
                            vertices->ny = 0.0f;
                            vertices->nz = 0.0f;
                            vertices->diffuse = 0;
                            vertices->u1 = tile->u2;
                            vertices->v1 = 0.0f;
                            vertices->u2 = 0.0f;
                            vertices->v2 = 0.0f;
                            vertices++;

                            vertices->x = tile->x3;
                            vertices->y = tile->y3;
                            vertices->z = tile->z3;
                            vertices->nx = 0.0f;
                            vertices->ny = 0.0f;
                            vertices->nz = 0.0f;
                            vertices->diffuse = 0;
                            vertices->u1 = tile->u3;
                            vertices->v1 = 0.0f;
                            vertices->u2 = 0.0f;
                            vertices->v2 = 0.0f;
                            vertices++;

                            vertices->x = tile->x4;
                            vertices->y = tile->y4;
                            vertices->z = tile->z4;
                            vertices->nx = 0.0f;
                            vertices->ny = 0.0f;
                            vertices->nz = 0.0f;
                            vertices->diffuse = 0;
                            vertices->u1 = tile->u4;
                            vertices->v1 = 0.0f;
                            vertices->u2 = 0.0f;
                            vertices->v2 = 0.0f;
                            vertices++;

                            if (m_map->Is_Cell_Flipped(index_x, index_y)) {
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

                        index++;
                    }
                }

                if (poly_count > 0 && vertex_count > 0) {
                    DX8Wrapper::Set_Index_Buffer(ib, 0);
                    DX8Wrapper::Set_Vertex_Buffer(vb);
                    DX8Wrapper::Draw_Triangles(0, poly_count / 3, 0, vertex_count);
                    m_shorelineBlendTilesRendered += poly_count / 6;
                }

                vertex_count = 0;
                poly_count = 0;
            }

            DX8Wrapper::Set_DX8_Render_State(D3DRS_COLORWRITEENABLE, 7);
            ShaderClass::Invalidate();
        }
    } else {
        Render_Shore_Lines_Sorted(camera);
    }
#endif
}

void BaseHeightMapRenderObjClass::Render_Shore_Lines_Sorted(CameraClass *camera)
{
#ifdef BUILD_WITH_D3D8
    m_shorelineBlendTilesRendered = 0;

    if (!g_theWriteableGlobalData->m_showSoftWaterEdge || g_theWaterTransparency->m_transparentWaterDepth == 0.0
        || !m_numShorelineBlendTiles || DX8Wrapper::Get_Back_Buffer_Format() != WW3D_FORMAT_A8R8G8B8) {
        return;
    }

    int vertex_count = 0;
    int poly_count = 0;
    int y = m_map->Get_Draw_Origin_Y() + m_map->Get_Draw_Height() - 1;
    int x = m_map->Get_Draw_Origin_X() + m_map->Get_Draw_Width() - 1;

    if (x > m_map->Get_X_Extent() - 1) {
        x = m_map->Get_X_Extent() - 1;
    }

    if (y > m_map->Get_Y_Extent() - 1) {
        y = m_map->Get_Y_Extent() - 1;
    }

    int origin_x = m_map->Get_Draw_Origin_X();
    int origin_y = m_map->Get_Draw_Origin_Y();

    if (m_sortAxis != 0) {
        if (m_startCell > origin_x) {
            origin_x = m_startCell;
        }

        if (m_endCell + 1 < x) {
            x = m_endCell + 1;
        }

        if (x - origin_x <= 0) {
            return;
        }
    } else {
        if (m_startCell > origin_y) {
            origin_y = m_startCell;
        }

        if (m_endCell + 1 < y) {
            y = m_endCell + 1;
        }

        if (y - origin_y <= 0) {
            return;
        }
    }

    ShaderClass shader(ShaderClass::s_presetOpaque2DShader);
    shader.Set_Depth_Compare(ShaderClass::PASS_LEQUAL);
    DX8Wrapper::Set_Shader(shader);
    VertexMaterialClass *material = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
    DX8Wrapper::Set_Material(material);
    Ref_Ptr_Release(material);
    DX8Wrapper::Set_Texture(0, m_destAlphaLUT);
    Matrix3D tm(true);
    DX8Wrapper::Set_Transform(D3DTS_WORLD, tm);
    DX8Wrapper::Set_DX8_Render_State(D3DRS_COLORWRITEENABLE, 8);
    DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXCOORDINDEX, 0);
    bool b = false;
    int index = 0;

    while (!b) {
        DynamicVBAccessClass vb(VertexBufferClass::BUFFER_TYPE_DYNAMIC_DX8, DX8_FVF_XYZNDUV2, 2048);
        DynamicIBAccessClass ib(IndexBufferClass::BUFFER_TYPE_DYNAMIC_DX8, 3072);

        {
            DynamicVBAccessClass::WriteLockClass vb_lock(&vb);
            VertexFormatXYZNDUV2 *vertices = vb_lock.Get_Formatted_Vertex_Array();
            DynamicIBAccessClass::WriteLockClass ib_lock(&ib);
            unsigned short *indices = ib_lock.Get_Index_Array();

            if (indices == nullptr || vertices == nullptr) {
                DX8Wrapper::Set_DX8_Render_State(D3DRS_COLORWRITEENABLE, 7);
                return;
            }

            if (m_sortAxis != 0) {
                int i;

                for (i = origin_x; i < x; i++) {
                    ShorelineSortInfo *info = &m_shorelineSortInfos[i];

                    if (info->unk2 != 0) {
                        int i1 = origin_y;

                        if (info->unk3 > origin_y) {
                            i1 = info->unk3;
                        }

                        int i2 = y;

                        if (info->unk4 + 1 < y) {
                            i2 = info->unk4 + 1;
                        }

                        if (i2 - i1 > 0) {
                            ShorelineTile *tile = &m_shorelineTiles[info->unk1 + index];

                            for (int j = index; j < info->unk2; j++) {
                                int i3 = tile->index >> 16;

                                if (i3 >= i1) {
                                    if (i3 >= i2) {
                                        break;
                                    }
                                    if (vertex_count >= 2048) {
                                        index = j;
                                        goto l1;
                                    }

                                    vertices->x = tile->x1;
                                    vertices->y = tile->y1;
                                    vertices->z = tile->z1;
                                    vertices->nx = 0.0f;
                                    vertices->ny = 0.0f;
                                    vertices->nz = 0.0f;
                                    vertices->diffuse = 0;
                                    vertices->u1 = tile->u1;
                                    vertices->v1 = 0.0f;
                                    vertices->u2 = 0.0f;
                                    vertices->v2 = 0.0f;
                                    vertices++;

                                    vertices->x = tile->x2;
                                    vertices->y = tile->y2;
                                    vertices->z = tile->z2;
                                    vertices->nx = 0.0f;
                                    vertices->ny = 0.0f;
                                    vertices->nz = 0.0f;
                                    vertices->diffuse = 0;
                                    vertices->u1 = tile->u2;
                                    vertices->v1 = 0.0f;
                                    vertices->u2 = 0.0f;
                                    vertices->v2 = 0.0f;
                                    vertices++;

                                    vertices->x = tile->x3;
                                    vertices->y = tile->y3;
                                    vertices->z = tile->z3;
                                    vertices->nx = 0.0f;
                                    vertices->ny = 0.0f;
                                    vertices->nz = 0.0f;
                                    vertices->diffuse = 0;
                                    vertices->u1 = tile->u3;
                                    vertices->v1 = 0.0f;
                                    vertices->u2 = 0.0f;
                                    vertices->v2 = 0.0f;
                                    vertices++;

                                    vertices->x = tile->x4;
                                    vertices->y = tile->y4;
                                    vertices->z = tile->z4;
                                    vertices->nx = 0.0f;
                                    vertices->ny = 0.0f;
                                    vertices->nz = 0.0f;
                                    vertices->diffuse = 0;
                                    vertices->u1 = tile->u4;
                                    vertices->v1 = 0.0f;
                                    vertices->u2 = 0.0f;
                                    vertices->v2 = 0.0f;
                                    vertices++;

                                    if (m_map->Is_Cell_Flipped(i, i3)) {
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
                                    tile++;
                                } else {
                                    tile++;
                                }
                            }

                            index = 0;
                        }
                    }
                }

            l1:
                origin_x = i;
                b = i >= x;
            } else {
                int i;

                for (i = origin_y; i < y; i++) {
                    ShorelineSortInfo *info = &m_shorelineSortInfos[i];

                    if (info->unk2 != 0) {
                        int i1 = origin_x;

                        if (info->unk3 > origin_x) {
                            i1 = info->unk3;
                        }

                        int i2 = x;

                        if (info->unk4 + 1 < x) {
                            i2 = info->unk4 + 1;
                        }

                        if (i2 - i1 > 0) {
                            ShorelineTile *tile = &m_shorelineTiles[info->unk1 + index];

                            for (int j = index; j < info->unk2; j++) {
                                int i3 = tile->index & 0xFFFF;

                                if (i3 >= i1) {
                                    if (i3 >= i2) {
                                        break;
                                    }
                                    if (vertex_count >= 2048) {
                                        index = j;
                                        goto l2;
                                    }

                                    vertices->x = tile->x1;
                                    vertices->y = tile->y1;
                                    vertices->z = tile->z1;
                                    vertices->nx = 0.0f;
                                    vertices->ny = 0.0f;
                                    vertices->nz = 0.0f;
                                    vertices->diffuse = 0;
                                    vertices->u1 = tile->u1;
                                    vertices->v1 = 0.0f;
                                    vertices->u2 = 0.0f;
                                    vertices->v2 = 0.0f;
                                    vertices++;

                                    vertices->x = tile->x2;
                                    vertices->y = tile->y2;
                                    vertices->z = tile->z2;
                                    vertices->nx = 0.0f;
                                    vertices->ny = 0.0f;
                                    vertices->nz = 0.0f;
                                    vertices->diffuse = 0;
                                    vertices->u1 = tile->u2;
                                    vertices->v1 = 0.0f;
                                    vertices->u2 = 0.0f;
                                    vertices->v2 = 0.0f;
                                    vertices++;

                                    vertices->x = tile->x3;
                                    vertices->y = tile->y3;
                                    vertices->z = tile->z3;
                                    vertices->nx = 0.0f;
                                    vertices->ny = 0.0f;
                                    vertices->nz = 0.0f;
                                    vertices->diffuse = 0;
                                    vertices->u1 = tile->u3;
                                    vertices->v1 = 0.0f;
                                    vertices->u2 = 0.0f;
                                    vertices->v2 = 0.0f;
                                    vertices++;

                                    vertices->x = tile->x4;
                                    vertices->y = tile->y4;
                                    vertices->z = tile->z4;
                                    vertices->nx = 0.0f;
                                    vertices->ny = 0.0f;
                                    vertices->nz = 0.0f;
                                    vertices->diffuse = 0;
                                    vertices->u1 = tile->u4;
                                    vertices->v1 = 0.0f;
                                    vertices->u2 = 0.0f;
                                    vertices->v2 = 0.0f;
                                    vertices++;

                                    if (m_map->Is_Cell_Flipped(i3, i)) {
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
                                    tile++;
                                } else {
                                    tile++;
                                }
                            }

                            index = 0;
                        }
                    }
                }

            l2:
                origin_y = i;
                b = i >= y;
            }
        }

        if (poly_count > 0 && vertex_count > 0) {
            DX8Wrapper::Set_Index_Buffer(ib, 0);
            DX8Wrapper::Set_Vertex_Buffer(vb);
            DX8Wrapper::Draw_Triangles(0, poly_count / 3, 0, vertex_count);
            m_shorelineBlendTilesRendered += poly_count / 6;
        }

        vertex_count = 0;
        poly_count = 0;
    }

    DX8Wrapper::Set_DX8_Render_State(D3DRS_COLORWRITEENABLE, 7);
    ShaderClass::Invalidate();
#endif
}

void BaseHeightMapRenderObjClass::Render_Trees(CameraClass *camera)
{
#ifdef BUILD_WITH_D3D8
    if (m_map != nullptr && m_scene != nullptr) {
        if (m_treeBuffer != nullptr) {
            Matrix3D tm(m_transform);
            DX8Wrapper::Set_Transform(D3DTS_WORLD, tm);
            RefMultiListIterator<RenderObjClass> iter(static_cast<RTS3DScene *>(m_scene)->Get_Dynamic_Lights());
            m_treeBuffer->Draw_Trees(camera, &iter);
        }
    }
#endif
}

void BaseHeightMapRenderObjClass::Set_Shore_Line_Detail()
{
    if (m_map != nullptr) {
        Update_Shoreline_Tiles(0, 0, m_map->Get_X_Extent() - 1, m_map->Get_Y_Extent() - 1, m_map);
    }
}

void BaseHeightMapRenderObjClass::Set_Texture_LOD(int LOD)
{
    if (m_treeBuffer != nullptr) {
        m_treeBuffer->Set_Texture_LOD(LOD);
    }

    if (m_map != nullptr) {
        m_map->Set_Texture_LOD(LOD);
    }
}

bool BaseHeightMapRenderObjClass::Show_As_Visible_Cliff(int x, int y) const
{
    if (m_map == nullptr) {
        return false;
    }

    return m_impassableAreas[m_map->Get_X_Extent() * y + x];
}

void BaseHeightMapRenderObjClass::Unit_Moved(Object *object)
{
    if (m_treeBuffer != nullptr) {
        m_treeBuffer->Unit_Moved(object);
    }
}

void BaseHeightMapRenderObjClass::Update_Macro_Texture(Utf8String texture_name)
{
    m_macroTextureName = texture_name;
    Ref_Ptr_Release(m_stageThreeTexture);
    m_stageThreeTexture = new LightMapTerrainTextureClass(m_macroTextureName, MIP_LEVELS_ALL);
}

void BaseHeightMapRenderObjClass::Update_Scorches()
{
    if (m_scorchesInBuffer <= 1 && m_numScorches != 0 && m_indexScorch != nullptr && m_vertexScorch != nullptr) {
        m_curNumScorchVertices = 0;
        m_curNumScorchIndices = 0;
        IndexBufferClass::WriteLockClass ib_lock(m_indexScorch, 0);
        unsigned short *indices = ib_lock.Get_Index_Array();
        VertexBufferClass::WriteLockClass vb_lock(m_vertexScorch, 0);
        VertexFormatXYZDUV1 *vertices = static_cast<VertexFormatXYZDUV1 *>(vb_lock.Get_Vertex_Array());
        float red = g_theWriteableGlobalData->m_terrainAmbient[0].red;
        float green = g_theWriteableGlobalData->m_terrainAmbient[0].green;
        float blue = g_theWriteableGlobalData->m_terrainAmbient[0].blue;
        red += g_theWriteableGlobalData->m_terrainDiffuse[0].red / 2.0f;
        green += g_theWriteableGlobalData->m_terrainDiffuse[0].green / 2.0f;
        blue += g_theWriteableGlobalData->m_terrainDiffuse[0].blue / 2.0f;
        red *= 255.0f;
        green *= 255.0f;
        blue *= 255.0f;
        unsigned int diffuse = Make_Color(GameMath::Fast_To_Int_Truncate(red),
            GameMath::Fast_To_Int_Truncate(green),
            GameMath::Fast_To_Int_Truncate(blue),
            255);
        m_scorchesInBuffer = 0;

        for (int i = m_numScorches - 1; i >= 0; i--) {
            m_scorchesInBuffer++;
            float radius = m_scorches[i].m_radius;
            Vector3 location(m_scorches[i].m_location);
            int scorch_type = m_scorches[i].m_scorchType;

            if (scorch_type < 0) {
                scorch_type = 0;
            }

            if (scorch_type >= 9) {
                scorch_type = 0;
            }

            int x_min = GameMath::Fast_To_Int_Floor((location.X - radius) / 10.0f);
            int y_min = GameMath::Fast_To_Int_Floor((location.Y - radius) / 10.0f);

            if (x_min < -m_map->Border_Size()) {
                x_min = -m_map->Border_Size();
            }

            if (y_min < -m_map->Border_Size()) {
                y_min = -m_map->Border_Size();
            }

            int x_max = GameMath::Fast_To_Int_Ceil((location.X + radius) / 10.0f) + 1;
            int y_max = GameMath::Fast_To_Int_Ceil((location.Y + radius) / 10.0f) + 1;

            if (x_max > m_map->Get_X_Extent() - m_map->Border_Size()) {
                x_max = m_map->Get_X_Extent() - m_map->Border_Size();
            }

            if (y_max > m_map->Get_Y_Extent() - m_map->Border_Size()) {
                y_max = m_map->Get_Y_Extent() - m_map->Border_Size();
            }

            int vert_count = m_curNumScorchVertices;

            for (int y = y_min; y < y_max; y++) {
                for (int x = x_min; x < x_max; x++) {
                    if (m_curNumScorchVertices >= 8194) {
                        return;
                    }

                    vertices->diffuse = diffuse;
                    float vz =
                        Get_Clip_Height(m_map->Border_Size() + x, m_map->Border_Size() + y) * HEIGHTMAP_SCALE + 0.0625f;
                    float u = (scorch_type % 3) * 1.5f;
                    float v = (scorch_type / 3) * 1.5f;
                    float vx = x * 10.0f;
                    float vy = y * 10.0f;
                    vertices->u1 = (u + 0.5f + (vx - location.X) / (2.0f * radius)) / 4.0f;
                    vertices->v1 = (v + 0.5f + (vy - location.Y) / (2.0f * radius)) / 4.0f;
                    vertices->x = vx;
                    vertices->y = vy;
                    vertices->z = vz;
                    vertices++;
                    m_curNumScorchVertices++;
                }
            }

            int x_dist = x_max - x_min;

            for (int y = 0; y < y_max - y_min - 1; y++) {
                for (int x = 0; x < x_max - x_min - 1; x++) {
                    if (m_curNumScorchIndices + 6 > 49164) {
                        return;
                    }

                    int x_index = m_map->Border_Size() + x_min + x;
                    int y_index = m_map->Border_Size() + y_min + y;

                    if (m_map->Get_Flip_State(x_index, y_index)) {
                        *indices++ = x_dist * y + vert_count + x + 1;
                        *indices++ = x_dist + x + x_dist * y + vert_count;
                        *indices++ = x + x_dist * y + vert_count;
                        *indices++ = x_dist * y + vert_count + x + 1;
                        *indices++ = x + x_dist * y + vert_count + x_dist + 1;
                        *indices++ = x_dist + x + x_dist * y + vert_count;
                    } else {
                        *indices++ = x + x_dist * y + vert_count;
                        *indices++ = x + x_dist * y + vert_count + x_dist + 1;
                        *indices++ = x_dist + x + x_dist * y + vert_count;
                        *indices++ = x + x_dist * y + vert_count;
                        *indices++ = x_dist * y + vert_count + x + 1;
                        *indices++ = x + x_dist * y + vert_count + x_dist + 1;
                    }

                    m_curNumScorchIndices += 6;
                }
            }
        }
    }
}

void BaseHeightMapRenderObjClass::Update_Shoreline_Tile(int x, int y, int border_size, WorldHeightMap *map)
{
    float opacity = g_theWaterTransparency->m_transparentWaterDepth * g_theWaterTransparency->m_transparentWaterMinOpacity;
    float opacity_div = 1.0f / opacity;
    float x1 = (x - border_size) * 10.0f;
    float y1 = (y - border_size) * 10.0f;
    float height1 = g_theWaterRenderObj->Get_Water_Height(x1, y1);
    float z1 = map->Get_Height(x, y) * HEIGHTMAP_SCALE;
    int flags = z1 < height1;

    float x2 = (x - border_size + 1) * 10.0f;
    float y2 = (y - border_size + 1) * 10.0f;
    float height2 = g_theWaterRenderObj->Get_Water_Height(x2, y1);
    float z2 = map->Get_Height(x + 1, y) * HEIGHTMAP_SCALE;
    flags |= (z2 < height2) << 1;

    float height3 = g_theWaterRenderObj->Get_Water_Height(x2, y2);
    float z3 = map->Get_Height(x + 1, y + 1) * HEIGHTMAP_SCALE;
    flags |= (z3 < height3) << 2;

    float height4 = g_theWaterRenderObj->Get_Water_Height(x1, y2);
    float z4 = map->Get_Height(x, y + 1) * HEIGHTMAP_SCALE;
    flags |= (z4 < height4) << 3;

    if (flags != 0 && height1 * height2 * height3 * height4 > 0.0
        && (flags < 15 || height1 - z1 < opacity || height2 - z2 < opacity || height3 - z3 < opacity
            || height4 - z4 < opacity)) {
        if (m_numShorelineBlendTiles >= m_shorelineBlendTileSize) {
            ShorelineTile *tiles = new ShorelineTile[m_shorelineBlendTileSize + 512];
            memcpy(tiles, m_shorelineTiles, m_shorelineBlendTileSize * sizeof(ShorelineTile));
            delete[] m_shorelineTiles;
            m_shorelineTiles = tiles;
            m_shorelineBlendTileSize += 512;
        }

        ShorelineTile *tile = &m_shorelineTiles[m_numShorelineBlendTiles];
        tile->index = (y << 16) | x;
        tile->x1 = x1;
        tile->y1 = y1;
        tile->z1 = z1;
        tile->u1 = (height1 - z1) * opacity_div;
        tile->x2 = x2;
        tile->y2 = y1;
        tile->z2 = z2;
        tile->u2 = (height2 - z2) * opacity_div;
        tile->x3 = x2;
        tile->y3 = y2;
        tile->z3 = z3;
        tile->u3 = (height3 - z3) * opacity_div;
        tile->x4 = x1;
        tile->y4 = y2;
        tile->z4 = z4;
        tile->u4 = (height4 - z4) * opacity_div;
        m_numShorelineBlendTiles++;
    }
}

void BaseHeightMapRenderObjClass::Update_Shoreline_Tiles(int min_x, int min_y, int max_x, int max_y, WorldHeightMap *map)
{
    int border_size = map->Get_Border_Size();

    if (min_x < 0) {
        min_x = 0;
    }

    if (min_y < 0) {
        min_y = 0;
    }

    if (max_x > map->Get_X_Extent() - 1) {
        max_x = map->Get_X_Extent() - 1;
    }

    if (max_y > map->Get_Y_Extent() - 1) {
        max_y = map->Get_Y_Extent() - 1;
    }

    if (m_shorelineTiles == nullptr) {
        m_shorelineTiles = new ShorelineTile[4096];
        m_shorelineBlendTileSize = 4096;
    }

    for (int i = 0; i < m_numShorelineBlendTiles; i++) {
        int x_index = m_shorelineTiles[i].index & 0xFFFF;
        int y_index = m_shorelineTiles[i].index >> 16;

        if (x_index >= min_x && x_index < max_x && y_index >= min_y && y_index < max_y) {
            memcpy(
                &m_shorelineTiles[i], &m_shorelineTiles[i + 1], (m_numShorelineBlendTiles - 1 - i) * sizeof(ShorelineTile));
            m_numShorelineBlendTiles--;
            i--;
        }
    }

    if (g_theWaterTransparency->m_transparentWaterDepth != 0.0f && g_theWriteableGlobalData->m_showSoftWaterEdge) {
        bool is_vertical_map = false;

        if (!g_theWriteableGlobalData->m_unsortedShoreLines) {
            if (m_map->Get_Y_Extent() - 1 > m_map->Get_X_Extent()) {
                is_vertical_map = true;
            }
        }

        if (is_vertical_map) {
            for (int x = min_x; x < max_x; x++) {
                for (int y = min_y; y < max_y; y++) {
                    Update_Shoreline_Tile(x, y, border_size, map);
                }
            }
        } else {
            for (int y = min_y; y < max_y; y++) {
                for (int x = min_x; x < max_x; x++) {
                    Update_Shoreline_Tile(x, y, border_size, map);
                }
            }
        }

        Record_Shore_Line_Sort_Infos();
    }
}

void BaseHeightMapRenderObjClass::Update_Tree_Position(DrawableID drawable, Coord3D pos, float angle)
{
    if (m_treeBuffer != nullptr) {
        m_treeBuffer->Update_Tree_Position(drawable, pos, angle);
    }
}

void BaseHeightMapRenderObjClass::Update_View_Impassable_Areas(bool unk, int min_x, int min_y, int max_x, int max_y)
{
    int x_extent = m_map->Get_X_Extent();
    int y_extent = m_map->Get_Y_Extent();

    if (m_impassableAreas.size() != x_extent * y_extent) {
        m_impassableAreas.resize(x_extent * y_extent);
    }

    if (!unk) {
        min_x = 0;
        max_x = 0;
        min_y = x_extent;
        max_y = y_extent;
    }

    float height = GameMath::Tan((m_cliffAngle / 360.0f + m_cliffAngle / 360.0f) * GAMEMATH_PI);

    for (int y = max_x; y < max_y; y++) {
        for (int x = min_x; x < min_y; x++) {
            m_impassableAreas[x_extent * y + x] = Evaluate_As_Visible_Cliff(x, y, height);
        }
    }
}

void BaseHeightMapRenderObjClass::World_Builder_Update_Bridge_Towers(GameAssetManager *assetmgr, SimpleSceneClass *scene)
{
    if (m_bridgeBuffer != nullptr) {
        m_bridgeBuffer->World_Builder_Update_Bridge_Towers(assetmgr, scene);
    }
}
