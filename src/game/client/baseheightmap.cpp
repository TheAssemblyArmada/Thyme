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
#include "flatheightmap.h"
#include "frustum.h"
#include "heightmap.h"
#include "rinfo.h"
#include "scene.h"
#include "shadermanager.h"
#include "terraintex.h"
#include "terrainvisual.h"
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

BaseHeightMapRenderObjClass::BaseHeightMapRenderObjClass() :
    m_x(0),
    m_y(0),
    m_needFullUpdate(false),
    m_showImpassableAreas(false),
    m_updating(false),
    m_maxHeight((GameMath::Pow(256.0f, 1.0f) - 1.0f) * 0.625f),
    m_minHeight(0.0f),
    m_shorelineTiles(nullptr),
    m_numShorelineBlendTiles(0),
    m_shorlineSortInfos(nullptr),
    m_shorlineSortInfoCount(0),
    m_sortAxis(1),
    m_endCell(0),
    m_startCell(0),
    m_shorelineBlendTilesRendered(0),
    m_shorelineBlendTileSize(0),
    m_transparentWaterMinOpacity(-1.0f),
    m_vertexMaterialClass(nullptr),
    m_stageZeroTexture(nullptr),
    m_stageOneTexture(nullptr),
    m_stageTwoTexture(nullptr),
    m_stageThreeTexture(nullptr),
    m_destAlphaLUT(nullptr),
    m_map(nullptr),
    m_depthFade(0.0f, 0.0f, 0.0f),
    m_useDepthFade(false),
    m_disableTextures(false),
    m_treeBuffer(nullptr),
    m_propBuffer(nullptr),
    m_bibBuffer(nullptr),
    m_cliffAngle(45.0f),
    m_bridgeBuffer(nullptr),
    m_waypointBuffer(nullptr),
    m_roadBuffer(nullptr),
    m_vertexScorch(nullptr),
    m_indexScorch(nullptr),
    m_scorchTexture(nullptr)
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
    delete[] m_shorlineSortInfos;
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
        g_theSnowManager->Release_Resources();
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
        g_theSnowManager->Re_Acquire_Resources();
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
    // heightmap TODO
#ifdef GAME_DLL
    return Call_Method<bool, BaseHeightMapRenderObjClass, RayCollisionTestClass &>(
        PICK_ADDRESS(0x007518C0, 0x005FDA9E), this, raytest);
#else
    return false;
#endif
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
    // heightmap TODO
#ifdef GAME_DLL
    return Call_Method<int,
        BaseHeightMapRenderObjClass,
        int,
        int,
        WorldHeightMap *,
        RefMultiListIterator<RenderObjClass> *,
        bool>(PICK_ADDRESS(0x00754160, 0x00600681), this, x, y, map, lights, update_shoreline);
#else
    return 0;
#endif
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
    // heightmap TODO
#ifdef GAME_DLL
    return Call_Method<int, BaseHeightMapRenderObjClass, int, int>(PICK_ADDRESS(0x00754E70, 0x006017D2), this, x, y);
#else
    return 0;
#endif
}

float BaseHeightMapRenderObjClass::Get_Max_Cell_Height(float x, float y)
{
    // heightmap TODO
#ifdef GAME_DLL
    return Call_Method<float, BaseHeightMapRenderObjClass, float, float>(PICK_ADDRESS(0x00752D00, 0x005FF000), this, x, y);
#else
    return 0;
#endif
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
    // heightmap TODO
#ifdef GAME_DLL
    Call_Method<void,
        BaseHeightMapRenderObjClass,
        VertexFormatXYZDUV2 *,
        Vector3 *,
        Vector3 *,
        RefMultiListIterator<RenderObjClass> *,
        unsigned char>(PICK_ADDRESS(0x007512A0, 0x005FD3A8), this, vb, light, normal, lights, alpha);
#endif
}

float BaseHeightMapRenderObjClass::Get_Height_Map_Height(float x, float y, Coord3D *pos)
{
    // heightmap TODO
#ifdef GAME_DLL
    return Call_Method<float, BaseHeightMapRenderObjClass, float, float, Coord3D *>(
        PICK_ADDRESS(0x00752580, 0x005FE5B6), this, x, y, pos);
#else
    return 0.0f;
#endif
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

bool BaseHeightMapRenderObjClass::Is_Clear_Line_Of_Sight(const Coord3D &pos1, const Coord3D &pos2) const
{
    // heightmap TODO
#ifdef GAME_DLL
    return Call_Method<bool, const BaseHeightMapRenderObjClass, const Coord3D &, const Coord3D &>(
        PICK_ADDRESS(0x00752960, 0x005FEBBC), this, pos1, pos2);
#else
    return false;
#endif
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
    // heightmap TODO
#ifdef GAME_DLL
    Call_Method<void, BaseHeightMapRenderObjClass>(PICK_ADDRESS(0x00753580, 0x005FF958), this);
#endif
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
    // heightmap TODO
#ifdef GAME_DLL
    Call_Method<void, BaseHeightMapRenderObjClass, CameraClass *>(PICK_ADDRESS(0x00755460, 0x00602046), this, camera);
#endif
}

void BaseHeightMapRenderObjClass::Render_Shore_Lines_Sorted(CameraClass *camera)
{
    // heightmap TODO
#ifdef GAME_DLL
    Call_Method<void, BaseHeightMapRenderObjClass, CameraClass *>(PICK_ADDRESS(0x00755D40, 0x006027B2), this, camera);
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

void BaseHeightMapRenderObjClass::Uupdate_Macro_Texture(Utf8String texture_name)
{
    m_macroTextureName = texture_name;
    Ref_Ptr_Release(m_stageThreeTexture);
    m_stageThreeTexture = new LightMapTerrainTextureClass(m_macroTextureName, MIP_LEVELS_ALL);
}

void BaseHeightMapRenderObjClass::Update_Scorches()
{
    // heightmap TODO
#ifdef GAME_DLL
    Call_Method<void, BaseHeightMapRenderObjClass>(PICK_ADDRESS(0x007546C0, 0x00600CE2), this);
#endif
}

void BaseHeightMapRenderObjClass::Update_Shoreline_Tile(int x, int y, int border_size, WorldHeightMap *map)
{
    // heightmap TODO
#ifdef GAME_DLL
    Call_Method<void, BaseHeightMapRenderObjClass, int, int, int, WorldHeightMap *>(
        PICK_ADDRESS(0x00753800, 0x005FFD4B), this, x, y, border_size, map);
#endif
}

void BaseHeightMapRenderObjClass::Update_Shoreline_Tiles(int min_x, int min_y, int max_x, int max_y, WorldHeightMap *map)
{
    // heightmap TODO
#ifdef GAME_DLL
    Call_Method<void, BaseHeightMapRenderObjClass, int, int, int, int, WorldHeightMap *>(
        PICK_ADDRESS(0x00753C90, 0x00600193), this, min_x, min_y, max_x, max_y, map);
#endif
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
