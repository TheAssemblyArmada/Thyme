/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Terrain Tracks
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dterraintracks.h"
#include "assetmgr.h"
#include "drawable.h"
#include "dx8indexbuffer.h"
#include "dx8vertexbuffer.h"
#include "dx8wrapper.h"
#include "globaldata.h"
#include "object.h"
#include "scene.h"
#include "terrainlogic.h"
#include "w3d.h"

#ifndef GAME_DLL
TerrainTracksRenderObjClassSystem *g_theTerrainTracksRenderObjClassSystem;
#endif

TerrainTracksRenderObjClass::TerrainTracksRenderObjClass() :
    // BUGFIX initialize all members
    m_stageZeroTexture(nullptr),
    m_activeEdgeCount(0),
    m_totalEdgesAdded(0),
    m_boundDrawable(nullptr),
    m_edges(),
    m_lastAnchor(0.0f, 1.0f, 2.25f),
    m_bottomIndex(0),
    m_topIndex(0),
    m_haveAnchor(false),
    m_bound(false),
    m_width(0),
    m_length(0),
    m_aboveTerrain(false),
    m_unk(true),
    m_nextSystem(nullptr),
    m_prevSystem(nullptr)
{
}

void TerrainTracksRenderObjClass::Render(RenderInfoClass &rinfo)
{
    if (g_theWriteableGlobalData->m_makeTrackMarks && m_activeEdgeCount >= 2) {
        g_theTerrainTracksRenderObjClassSystem->m_edgesToFlush += m_activeEdgeCount;
    }
}

void TerrainTracksRenderObjClass::Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const
{
    sphere = m_boundingSphere;
}

void TerrainTracksRenderObjClass::Get_Obj_Space_Bounding_Box(AABoxClass &box) const
{
    box = m_boundingBox;
}

int TerrainTracksRenderObjClass::Free_Terrain_Tracks_Resources()
{
    Ref_Ptr_Release(m_stageZeroTexture);
    m_haveAnchor = false;
    m_unk = true;
    m_topIndex = 0;
    m_bottomIndex = 0;
    m_activeEdgeCount = 0;
    m_totalEdgesAdded = 0;
    m_boundDrawable = nullptr;
    return 0;
}

void TerrainTracksRenderObjClass::Add_Cap_Edge_To_Track(float x, float y)
{
    if (!m_unk) {
        if (m_activeEdgeCount == 1) {
            m_unk = true;
            m_haveAnchor = false;
        } else {
            Vector3 anchor;
            Vector3 v1;
            Coord3D n;
            float z;
            PathfindLayerEnum layer;

            if (m_boundDrawable && (layer = m_boundDrawable->Get_Object()->Get_Layer()) != LAYER_GROUND) {
                z = g_theTerrainLogic->Get_Layer_Height(x, y, layer, &n, true) + 0.25f;
            } else {
                z = g_theTerrainLogic->Get_Ground_Height(x, y, &n);
            }

            v1.X = n.x;
            v1.Y = n.y;
            v1.Z = n.z;
            anchor.X = x;
            anchor.Y = y;
            anchor.Z = z;
            Vector3 dist = Vector3(x, y, z) - m_lastAnchor;
            int maxedges = g_theTerrainTracksRenderObjClassSystem->m_maxTankTrackEdges;

            if (GameMath::Square(m_length) > dist.Length2()) {
                int e = m_topIndex - 1;

                if (e < 0) {
                    e = maxedges - 1;
                }

                m_edges[e].alpha = 0.0f;
            } else {
                if (m_activeEdgeCount >= maxedges) {
                    m_bottomIndex++;
                    m_activeEdgeCount--;

                    if (m_bottomIndex >= maxedges) {
                        m_bottomIndex = 0;
                    }
                }

                if (m_topIndex >= maxedges) {
                    m_topIndex = 0;
                }

                dist.Z = 0.0f;
                dist.Normalize();
                Vector3 v2;
                Vector3::Cross_Product(dist, v1, &v2);
                EdgeInfo *edge = &m_edges[m_topIndex];
                edge->end_point_pos[0] = anchor - ((m_width * 0.5f) * v2);
                edge->end_point_pos[0].Z += 2.0f;

                if ((m_totalEdgesAdded & 1) != 0) {
                    edge->end_point_uv[0].X = 0.0f;
                    edge->end_point_uv[0].Y = 0.0f;
                } else {
                    edge->end_point_uv[0].X = 0.0f;
                    edge->end_point_uv[0].Y = 1.0f;
                }

                edge->end_point_pos[1] = anchor + ((m_width * 0.5f) * v2);
                edge->end_point_pos[1].Z += 2.0f;

                if ((m_totalEdgesAdded & 1) != 0) {
                    edge->end_point_uv[1].X = 1.0f;
                    edge->end_point_uv[1].Y = 0.0f;
                } else {
                    edge->end_point_uv[1].X = 1.0f;
                    edge->end_point_uv[1].Y = 1.0f;
                }

                edge->time_added = W3D::Get_Sync_Time();

                edge->alpha = 0.0f;

                m_lastAnchor = anchor;

                m_activeEdgeCount++;
                m_totalEdgesAdded++;
                m_topIndex++;
            }

            m_unk = true;
            m_haveAnchor = false;
        }
    }
}

void TerrainTracksRenderObjClass::Add_Edge_To_Track(float x, float y)
{
    if (!m_haveAnchor) {
        float z;
        PathfindLayerEnum layer;

        if (m_boundDrawable && (layer = m_boundDrawable->Get_Object()->Get_Layer()) != LAYER_GROUND) {
            z = g_theTerrainLogic->Get_Layer_Height(x, y, layer, nullptr, true) + 0.25f;
            m_lastAnchor = Vector3(x, y, z);
        } else {
            z = g_theTerrainLogic->Get_Ground_Height(x, y, nullptr);
            m_lastAnchor = Vector3(x, y, z);
        }

        m_haveAnchor = true;
        m_aboveTerrain = true;
        m_unk = true;
    } else {
        m_unk = false;
        Vector3 anchor;
        Vector3 v1;
        Coord3D n;
        float z;
        PathfindLayerEnum layer;

        if (m_boundDrawable && (layer = m_boundDrawable->Get_Object()->Get_Layer()) != LAYER_GROUND) {
            z = g_theTerrainLogic->Get_Layer_Height(x, y, layer, &n, true) + 0.25f;
        } else {
            z = g_theTerrainLogic->Get_Ground_Height(x, y, &n);
        }

        v1.X = n.x;
        v1.Y = n.y;
        v1.Z = n.z;
        anchor.X = x;
        anchor.Y = y;
        anchor.Z = z;
        Vector3 dist = Vector3(x, y, z) - m_lastAnchor;

        if (GameMath::Square(m_length) <= dist.Length2()) {
            int maxedges = g_theTerrainTracksRenderObjClassSystem->m_maxTankTrackEdges;

            if (m_activeEdgeCount >= maxedges) {
                m_bottomIndex++;
                m_activeEdgeCount--;

                if (m_bottomIndex >= maxedges) {
                    m_bottomIndex = 0;
                }
            }

            if (m_topIndex >= maxedges) {
                m_topIndex = 0;
            }

            dist.Z = 0.0f;
            dist.Normalize();
            Vector3 v2;
            Vector3::Cross_Product(dist, v1, &v2);
            EdgeInfo *edge = &m_edges[m_topIndex];
            edge->end_point_pos[0] = anchor - ((m_width * 0.5f) * v2);
            edge->end_point_pos[0].Z += 2.0f;

            if ((m_totalEdgesAdded & 1) != 0) {
                edge->end_point_uv[0].X = 0.0f;
                edge->end_point_uv[0].Y = 0.0f;
            } else {
                edge->end_point_uv[0].X = 0.0f;
                edge->end_point_uv[0].Y = 1.0f;
            }

            edge->end_point_pos[1] = anchor + ((m_width * 0.5f) * v2);
            edge->end_point_pos[1].Z += 2.0f;

            if ((m_totalEdgesAdded & 1) != 0) {
                edge->end_point_uv[1].X = 1.0f;
                edge->end_point_uv[1].Y = 0.0f;
            } else {
                edge->end_point_uv[1].X = 1.0f;
                edge->end_point_uv[1].Y = 1.0f;
            }

            edge->time_added = W3D::Get_Sync_Time();

            edge->alpha = 1.0f;
            if (m_aboveTerrain || m_activeEdgeCount <= 1) {
                edge->alpha = 0.0f;
            }
            m_aboveTerrain = false;

            m_lastAnchor = anchor;

            m_activeEdgeCount++;
            m_totalEdgesAdded++;
            m_topIndex++;
        }
    }
}

void TerrainTracksRenderObjClass::Init(float width, float length, char const *texture_name)
{
    Free_Terrain_Tracks_Resources();
    m_boundingSphere.Init(Vector3(0.0f, 0.0f, 0.0f), 4000.0f);
    m_boundingBox.Init(Vector3(0.0f, 0.0f, 0.0f), Vector3(4000.0f, 4000.0f, 1.0f));
    m_width = width;
    m_length = length;
    Set_Force_Visible(true);
    m_stageZeroTexture = W3DAssetManager::Get_Instance()->Get_Texture(texture_name);
}

TerrainTracksRenderObjClassSystem::TerrainTracksRenderObjClassSystem() :
    m_vertexBuffer(nullptr),
    m_indexBuffer(nullptr),
    m_vertexMaterialClass(nullptr),
    m_usedModules(nullptr),
    m_freeModules(nullptr),
    m_TerrainTracksScene(nullptr),
    m_edgesToFlush(0),
    m_maxTankTrackEdges(g_theWriteableGlobalData->m_maxTankTrackEdges),
    m_maxTankTrackOpaqueEdges(g_theWriteableGlobalData->m_maxTankTrackOpaqueEdges),
    m_maxTankTrackFadeDelay(g_theWriteableGlobalData->m_maxTankTrackFadeDelay)
{
}

TerrainTracksRenderObjClassSystem::~TerrainTracksRenderObjClassSystem()
{
    Shutdown();
    m_vertexMaterialClass = nullptr;
    m_TerrainTracksScene = nullptr;
}

void TerrainTracksRenderObjClassSystem::Release_Resources()
{
    Ref_Ptr_Release(m_indexBuffer);
    Ref_Ptr_Release(m_vertexBuffer);
}

void TerrainTracksRenderObjClassSystem::Re_Acquire_Resources()
{
    int num_modules = g_theWriteableGlobalData->m_maxTerrainTracks;
    Ref_Ptr_Release(m_indexBuffer);
    Ref_Ptr_Release(m_vertexBuffer);
    m_indexBuffer = new DX8IndexBufferClass(6 * (m_maxTankTrackEdges - 1), DX8IndexBufferClass::USAGE_DEFAULT);

    {
        IndexBufferClass::WriteLockClass lock(m_indexBuffer, 0);
        unsigned short *indices = lock.Get_Index_Array();
        for (int i = 0; i < m_maxTankTrackEdges - 1; i++) {
            indices[0] = 2 * i;
            indices[3] = 2 * i;
            indices[1] = 2 * i + 1;

            indices[2] = 2 * i + 3;
            indices[4] = 2 * i + 3;
            indices[5] = 2 * i + 2;

            indices += 6;
        }
    }

    captainslog_dbgassert(num_modules * m_maxTankTrackEdges * 2 < 65535, "Too many terrain track edges!");
    captainslog_dbgassert(num_modules * m_maxTankTrackEdges * 2 > 0, "Um, zero terrain track edges?");

    m_vertexBuffer = new DX8VertexBufferClass(
        DX8_FVF_XYZDUV1, num_modules * m_maxTankTrackEdges * 2, DX8VertexBufferClass::USAGE_DYNAMIC, 0);
}

void TerrainTracksRenderObjClassSystem::Flush()
{
#ifdef BUILD_WITH_D3D8
    TerrainTracksRenderObjClass *mod = m_usedModules;

    if (mod != nullptr) {
        if (!ShaderClass::Is_Backface_Culling_Inverted()) {
            float r = g_theWriteableGlobalData->m_terrainAmbient[0].red;
            float g = g_theWriteableGlobalData->m_terrainAmbient[0].green;
            float b = g_theWriteableGlobalData->m_terrainAmbient[0].blue;

            r = g_theWriteableGlobalData->m_terrainDiffuse[0].red / 2.0f + r;
            g = g_theWriteableGlobalData->m_terrainDiffuse[0].green / 2.0f + g;
            b = g_theWriteableGlobalData->m_terrainDiffuse[0].blue / 2.0f + b;

            r = r * 255.0f;
            g = g * 255.0f;
            b = b * 255.0f;

            int color = (GameMath::Fast_To_Int_Truncate(r) << 16) | (GameMath::Fast_To_Int_Truncate(g) << 8)
                | GameMath::Fast_To_Int_Truncate(b);

            float edges = (float)(m_maxTankTrackEdges - m_maxTankTrackOpaqueEdges);

            if (m_edgesToFlush >= 2) {
                VertexBufferClass::WriteLockClass lock(m_vertexBuffer, 0);
                VertexFormatXYZDUV1 *vertices = (VertexFormatXYZDUV1 *)lock.Get_Vertex_Array();

                for (mod = m_usedModules; mod != nullptr; mod = mod->m_nextSystem) {
                    if (mod->m_activeEdgeCount >= 2 && mod->Is_Really_Visible()) {

                        for (int i = 0, j = mod->m_bottomIndex; i < mod->m_activeEdgeCount; i++, j++) {

                            if (j >= m_maxTankTrackEdges) {
                                j = 0;
                            }

                            Vector3 *vert = &mod->m_edges[j].end_point_pos[0];
                            Vector2 *uv = &mod->m_edges[j].end_point_uv[0];

                            float alpha = 1.0f;

                            if (mod->m_activeEdgeCount - 1 - i >= m_maxTankTrackOpaqueEdges) {
                                alpha = 1.0f - (float)(mod->m_activeEdgeCount - i - m_maxTankTrackOpaqueEdges) / edges;
                            }

                            alpha *= mod->m_edges[j].alpha;

                            vertices->x = vert->X;
                            vertices->y = vert->Y;
                            vertices->z = vert->Z;
                            vertices->u1 = uv->X;
                            vertices->v1 = uv->Y;
                            vertices->diffuse = (GameMath::Fast_To_Int_Truncate(alpha * 255.0f) << 24) | color;
                            vertices++;

                            vert = &mod->m_edges[j].end_point_pos[1];
                            uv = &mod->m_edges[j].end_point_uv[1];

                            vertices->x = vert->X;
                            vertices->y = vert->Y;
                            vertices->z = vert->Z;
                            vertices->u1 = uv->X;
                            vertices->v1 = uv->Y;
                            vertices->diffuse = (GameMath::Fast_To_Int_Truncate(alpha * 255.0f) << 24) | color;
                            vertices++;
                        }
                    }
                }
            }

            if (m_edgesToFlush >= 2) {
                ShaderClass::Invalidate();
                DX8Wrapper::Set_Material(m_vertexMaterialClass);
                DX8Wrapper::Set_Shader(m_shaderClass);
                DX8Wrapper::Set_Index_Buffer(m_indexBuffer, 0);
                DX8Wrapper::Set_Vertex_Buffer(m_vertexBuffer, 0);
                int index_offset = 0;
                mod = m_usedModules;
                Matrix3D m = mod->Get_Transform_No_Validity_Check();
                DX8Wrapper::Set_Transform(D3DTS_WORLD, m);

                while (mod != nullptr) {
                    if (mod->m_activeEdgeCount >= 2 && mod->Is_Really_Visible()) {
                        DX8Wrapper::Set_Texture(0, mod->m_stageZeroTexture);
                        DX8Wrapper::Set_Index_Buffer_Index_Offset(index_offset);
                        DX8Wrapper::Draw_Triangles(0, 2 * mod->m_activeEdgeCount - 2, 0, 2 * mod->m_activeEdgeCount);
                        index_offset += 2 * mod->m_activeEdgeCount;
                    }

                    mod = mod->m_nextSystem;
                }
            }

            m_edgesToFlush = 0;
        }
    }
#endif
}

void TerrainTracksRenderObjClassSystem::Update()
{
    int time = W3D::Get_Sync_Time();
    TerrainTracksRenderObjClass *next;

    for (TerrainTracksRenderObjClass *mod = m_usedModules; mod != nullptr; mod = next) {
        next = mod->m_nextSystem;

        if (!g_theWriteableGlobalData->m_makeTrackMarks) {
            mod->m_haveAnchor = false;
        }

        for (int i = 0, j = mod->m_bottomIndex; i < mod->m_activeEdgeCount; i++, j++) {
            if (j >= m_maxTankTrackEdges) {
                j = 0;
            }

            float alpha = 1.0f - (float)(time - mod->m_edges[j].time_added) / (float)m_maxTankTrackFadeDelay;

            if (alpha < 0.0f) {
                alpha = 0.0f;
            }

            if (mod->m_edges[j].alpha > 0.0f) {
                mod->m_edges[j].alpha = alpha;
            }

            if (alpha == 0.0f) {
                mod->m_bottomIndex++;
                mod->m_activeEdgeCount--;

                if (mod->m_bottomIndex >= m_maxTankTrackEdges) {
                    mod->m_bottomIndex = 0;
                }
            }

            if (!mod->m_activeEdgeCount && !mod->m_bound) {
                Release_Track(mod);
            }
        }
    }
}

void TerrainTracksRenderObjClassSystem::Init(SceneClass *terrain_tracks_scene)
{
    int max = g_theWriteableGlobalData->m_maxTerrainTracks;
    m_TerrainTracksScene = terrain_tracks_scene;
    Re_Acquire_Resources();
    m_vertexMaterialClass = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
    m_shaderClass = ShaderClass::s_presetAlphaShader;

    if (!m_freeModules && !m_usedModules) {
        for (int i = 0; i < max; i++) {
            TerrainTracksRenderObjClass *mod = new TerrainTracksRenderObjClass();

            if (mod == nullptr) {
                captainslog_assert(0);
                break;
            }

            mod->m_prevSystem = nullptr;
            mod->m_nextSystem = m_freeModules;

            if (m_freeModules) {
                m_freeModules->m_prevSystem = mod;
            }

            m_freeModules = mod;
        }
    } else {
        captainslog_assert(0);
    }
}

void TerrainTracksRenderObjClassSystem::Shutdown()
{
    TerrainTracksRenderObjClass *next;

    for (TerrainTracksRenderObjClass *mod = m_usedModules; mod != nullptr; mod = next) {
        next = mod->m_nextSystem;

        if (!mod->m_bound) {
            Release_Track(mod);
        }
    }

    captainslog_assert(m_usedModules == nullptr);

    while (m_freeModules) {
        next = m_freeModules->m_nextSystem;
        Ref_Ptr_Release(m_freeModules);
        m_freeModules = next;
    }

    Ref_Ptr_Release(m_indexBuffer);
    Ref_Ptr_Release(m_vertexMaterialClass);
    Ref_Ptr_Release(m_vertexBuffer);
}

void TerrainTracksRenderObjClassSystem::Reset()
{
    TerrainTracksRenderObjClass *next;

    for (TerrainTracksRenderObjClass *mod = m_usedModules; mod != nullptr; mod = next) {
        next = mod->m_nextSystem;
        Release_Track(mod);
    }

    captainslog_assert(m_usedModules == nullptr);

    m_edgesToFlush = 0;
}

float Compute_Track_Spacing(RenderObjClass *robj)
{
    float spacing = 14.0f;
    int bone1 = robj->Get_Bone_Index("TREADFX01");

    if (bone1) {
        int bone2 = robj->Get_Bone_Index("TREADFX02");

        if (bone2) {
            Vector3 bonet1 = robj->Get_Bone_Transform(bone1).Get_Translation();
            Vector3 bonet2 = robj->Get_Bone_Transform(bone2).Get_Translation();

            bonet2 -= bonet1;

            spacing = (bonet2).Length() + 4.0f;
        }
    }

    return spacing;
}

TerrainTracksRenderObjClass *TerrainTracksRenderObjClassSystem::Bind_Track(
    RenderObjClass *render_object, float length, char const *texture_name)
{
    TerrainTracksRenderObjClass *mod = m_freeModules;

    if (mod != nullptr) {
        if (mod->m_nextSystem != nullptr) {
            mod->m_nextSystem->m_prevSystem = mod->m_prevSystem;
        }

        if (mod->m_prevSystem != nullptr) {
            mod->m_prevSystem->m_nextSystem = mod->m_nextSystem;
        } else {
            m_freeModules = mod->m_nextSystem;
        }

        mod->m_prevSystem = nullptr;
        mod->m_nextSystem = m_usedModules;

        if (m_usedModules != nullptr) {
            m_usedModules->m_prevSystem = mod;
        }

        m_usedModules = mod;

        mod->Init(Compute_Track_Spacing(render_object), length, texture_name);

        mod->m_bound = true;

        m_TerrainTracksScene->Add_Render_Object(mod);
    }

    return mod;
}

void TerrainTracksRenderObjClassSystem::Clear_Tracks()
{
    for (TerrainTracksRenderObjClass *mod = m_usedModules; mod != nullptr; mod = mod->m_nextSystem) {
        mod->m_haveAnchor = false;
        mod->m_unk = true;
        mod->m_topIndex = 0;
        mod->m_bottomIndex = 0;
        mod->m_activeEdgeCount = 0;
        mod->m_totalEdgesAdded = 0;
    }

    m_edgesToFlush = 0;
}

void TerrainTracksRenderObjClassSystem::Release_Track(TerrainTracksRenderObjClass *mod)
{
    if (mod != nullptr) {
        captainslog_dbgassert(!mod->m_bound, "mod is bound.");

        if (mod->m_nextSystem != nullptr) {
            mod->m_nextSystem->m_prevSystem = mod->m_prevSystem;
        }
        if (mod->m_prevSystem != nullptr) {
            mod->m_prevSystem->m_nextSystem = mod->m_nextSystem;
        } else {
            m_usedModules = mod->m_nextSystem;
        }

        mod->m_prevSystem = nullptr;
        mod->m_nextSystem = m_freeModules;

        if (m_freeModules != nullptr) {
            m_freeModules->m_prevSystem = mod;
        }

        m_freeModules = mod;

        mod->Free_Terrain_Tracks_Resources();

        m_TerrainTracksScene->Remove_Render_Object(mod);
    }
}

void TerrainTracksRenderObjClassSystem::Set_Detail()
{
    Clear_Tracks();
    Release_Resources();
    m_maxTankTrackEdges = g_theWriteableGlobalData->m_maxTankTrackEdges;
    m_maxTankTrackOpaqueEdges = g_theWriteableGlobalData->m_maxTankTrackOpaqueEdges;
    m_maxTankTrackFadeDelay = g_theWriteableGlobalData->m_maxTankTrackFadeDelay;
    Re_Acquire_Resources();
}

void TerrainTracksRenderObjClassSystem::Unbind_Track(TerrainTracksRenderObjClass *mod)
{
    mod->m_bound = false;
    mod->m_boundDrawable = nullptr;
}
