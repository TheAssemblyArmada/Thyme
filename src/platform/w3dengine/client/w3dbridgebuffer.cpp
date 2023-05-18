/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Bridge buffer
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dbridgebuffer.h"
#include "assetmgr.h"
#include "baseheightmap.h"
#include "dx8indexbuffer.h"
#include "dx8vertexbuffer.h"
#include "dx8wrapper.h"
#include "globaldata.h"
#include "mapobject.h"
#include "mesh.h"
#include "meshmdl.h"
#include "shadermanager.h"
#include "terrainroads.h"
#include "texture.h"
#include "w3dshroud.h"
#include "w3dterrainlogic.h"
#include <cstring>
using std::strcat;
using std::strcpy;

ShaderClass g_detailAlphaShader(361659);

// BUGFIX added extra variable initializations.
W3DBridge::W3DBridge() :
    m_scale(1.0f),
    m_length(1.0f),
    m_bridgeType(FIXED_BRIDGE),
    m_bridgeTexture(nullptr),
    m_leftMesh(nullptr),
    m_minY(0.0f),
    m_maxY(0.0f),
    m_leftMinX(0.0f),
    m_leftMaxX(0.0f),
    m_sectionMesh(nullptr),
    m_sectionMinX(0.0f),
    m_sectionMaxX(0.0f),
    m_rightMesh(nullptr),
    m_rightMinX(0.0f),
    m_rightMaxX(0.0f),
    m_firstIndex(0),
    m_numVertex(0),
    m_firstVertex(0),
    m_numPolygons(0),
    m_visible(false),
    m_curDamageState(BODY_PRISTINE),
    m_enabled(false)
{
}

W3DBridge::~W3DBridge()
{
    Clear_Bridge();
}

void W3DBridge::Clear_Bridge()
{
    m_visible = false;
    Ref_Ptr_Release(m_bridgeTexture);
    Ref_Ptr_Release(m_leftMesh);
    Ref_Ptr_Release(m_sectionMesh);
    Ref_Ptr_Release(m_rightMesh);
}

bool W3DBridge::Cull_Bridge(CameraClass *camera)
{
    bool visible = m_visible;
    m_visible = true;
    return visible != true;
}

void W3DBridge::Get_Bridge_Info(BridgeInfo *info)
{
    info->from.x = m_start.X;
    info->from.y = m_start.Y;
    info->from.z = m_start.Z;
    info->to.x = m_end.X;
    info->to.y = m_end.Y;
    info->to.z = m_end.Z;
    info->bridge_width = (m_maxY - m_minY) * m_scale;

    Vector3 normal(-(m_end - m_start).Y, (m_end - m_start).X, 0.0f);
    normal.Normalize();

    info->from_left.x = normal.X * m_maxY * m_scale + m_start.X;
    info->from_left.y = normal.Y * m_maxY * m_scale + m_start.Y;
    info->from_left.z = normal.Z * m_maxY * m_scale + m_start.Z;
    info->from_right.x = normal.X * m_minY * m_scale + m_start.X;
    info->from_right.y = normal.Y * m_minY * m_scale + m_start.Y;
    info->from_right.z = normal.Z * m_minY * m_scale + m_start.Z;
    info->to_left.x = normal.X * m_maxY * m_scale + m_end.X;
    info->to_left.y = normal.Y * m_maxY * m_scale + m_end.Y;
    info->to_left.z = normal.Z * m_maxY * m_scale + m_end.Z;
    info->to_right.x = normal.X * m_minY * m_scale + m_end.X;
    info->to_right.y = normal.Y * m_minY * m_scale + m_end.Y;
    info->to_right.z = normal.Z * m_minY * m_scale + m_end.Z;
}

void W3DBridge::Init(Vector3 from_loc, Vector3 to_loc, Utf8String bridge_template_name)
{
    m_start = from_loc;
    m_end = to_loc;
    m_template = bridge_template_name;
    m_enabled = true;
}

bool W3DBridge::Load(BodyDamageType cur_damage_state)
{
    Ref_Ptr_Release(m_bridgeTexture);
    Ref_Ptr_Release(m_leftMesh);
    Ref_Ptr_Release(m_sectionMesh);
    Ref_Ptr_Release(m_rightMesh);
    char texture[260] = {};
    char model[260] = {};
    char left[260] = {};
    char section[260] = {};
    char right[260] = {};
    strcpy(texture, "No Texture");
    strcpy(model, "BRIDGESECTIONAL");
    float scale = 0.7f;
    float length = 170.0f;
    TerrainRoadType *bridge = g_theTerrainRoads->Find_Bridge(m_template);

    if (bridge == nullptr) {
        return false;
    }

    scale = bridge->Get_Bridge_Scale();

    switch (cur_damage_state) {
        case BODY_PRISTINE:
            strcpy(texture, bridge->Get_Texture().Str());
            strcpy(model, bridge->Get_Bridge_Model().Str());
            break;
        case BODY_DAMAGED:
            strcpy(texture, bridge->Get_Texture_Damaged().Str());
            strcpy(model, bridge->Get_Bridge_Model_Name_Damaged().Str());
            break;
        case BODY_REALLYDAMAGED:
            strcpy(texture, bridge->Get_Texture_Really_Damaged().Str());
            strcpy(model, bridge->Get_Bridge_Model_Name_Really_Damaged().Str());
            break;
        case BODY_RUBBLE:
            strcpy(texture, bridge->Get_Texture_Broken().Str());
            strcpy(model, bridge->Get_Bridge_Model_Name_Broken().Str());
            break;
        default:
            return false;
    }

    strcpy(left, model);
    strcat(left, ".BRIDGE_LEFT");
    strcpy(section, model);
    strcat(section, ".BRIDGE_SPAN");
    strcpy(right, model);
    strcat(right, ".BRIDGE_RIGHT");
    W3DAssetManager *mgr = W3DAssetManager::Get_Instance();
    m_bridgeTexture = mgr->Get_Texture(texture, MIP_LEVELS_3);
    m_leftMtx.Make_Identity();
    m_rightMtx.Make_Identity();
    m_sectionMtx.Make_Identity();
    RenderObjClass *obj = mgr->Create_Render_Obj(model);

    if (obj == nullptr) {
        return false;
    } else {
        for (int i = 0; i < obj->Get_Num_Sub_Objects(); i++) {
            RenderObjClass *subobj = obj->Get_Sub_Object(i);
            Matrix3D m = subobj->Get_Transform();

            if (!strncasecmp(left, subobj->Get_Name(), strlen(left))) {
                m_leftMtx = m;
                strcpy(left, subobj->Get_Name());
            }

            if (!strncasecmp(section, subobj->Get_Name(), strlen(section))) {
                m_sectionMtx = m;
                strcpy(section, subobj->Get_Name());
            }

            if (!strncasecmp(right, subobj->Get_Name(), strlen(right))) {
                m_rightMtx = m;
                strcpy(right, subobj->Get_Name());
            }

            Ref_Ptr_Release(subobj);
        }

        Ref_Ptr_Release(obj);
        m_leftMesh = static_cast<MeshClass *>(W3DAssetManager::Get_Instance()->Create_Render_Obj(left));
        m_sectionMesh = static_cast<MeshClass *>(W3DAssetManager::Get_Instance()->Create_Render_Obj(section));
        m_rightMesh = static_cast<MeshClass *>(W3DAssetManager::Get_Instance()->Create_Render_Obj(right));
        m_scale = scale;

        if (m_leftMesh == nullptr) {
            Clear_Bridge();
            return false;
        } else {
            m_bridgeType = SECTIONAL_BRIDGE;

            if (m_rightMesh == nullptr || m_sectionMesh == nullptr) {
                m_bridgeType = FIXED_BRIDGE;
            }

            int count = m_leftMesh->Peek_Model()->Get_Vertex_Count();
            Vector3 *verts = m_leftMesh->Peek_Model()->Get_Vertex_Array();

            m_leftMinX = GAMEMATH_FLOAT_MAX;
            m_leftMaxX = -GAMEMATH_FLOAT_MAX;
            m_minY = GAMEMATH_FLOAT_MAX;
            m_maxY = -GAMEMATH_FLOAT_MAX;

            for (int i = 0; i < count; ++i) {
                Vector3 out;
                Matrix3D::Transform_Vector(m_leftMtx, verts[i], &out);

                if (m_leftMinX > (double)out.X) {
                    m_leftMinX = out.X;
                }

                if (m_minY > (double)out.Y) {
                    m_minY = out.Y;
                }

                if (out.X > (double)m_leftMaxX) {
                    m_leftMaxX = out.X;
                }

                if (out.Y > (double)m_maxY) {
                    m_maxY = out.Y;
                }
            }

            if (m_bridgeType == SECTIONAL_BRIDGE) {
                count = m_sectionMesh->Peek_Model()->Get_Vertex_Count();
                verts = m_sectionMesh->Peek_Model()->Get_Vertex_Array();
                m_sectionMinX = GAMEMATH_FLOAT_MAX;
                m_sectionMaxX = -GAMEMATH_FLOAT_MAX;

                for (int i = 0; i < count; ++i) {
                    Vector3 out;
                    Matrix3D::Transform_Vector(m_sectionMtx, verts[i], &out);
                    if (m_sectionMinX > (double)out.X) {
                        m_sectionMinX = out.X;
                    }
                    if (out.X > (double)m_sectionMaxX) {
                        m_sectionMaxX = out.X;
                    }
                }

                count = m_rightMesh->Peek_Model()->Get_Vertex_Count();
                verts = m_rightMesh->Peek_Model()->Get_Vertex_Array();
                m_rightMinX = GAMEMATH_FLOAT_MAX;
                m_rightMaxX = -GAMEMATH_FLOAT_MAX;

                for (int i = 0; i < count; ++i) {
                    Vector3 out;
                    Matrix3D::Transform_Vector(m_rightMtx, verts[i], &out);
                    if (m_rightMinX > (double)out.X) {
                        m_rightMinX = out.X;
                    }
                    if (out.X > (double)m_rightMaxX) {
                        m_rightMaxX = out.X;
                    }
                }
            } else {
                m_sectionMinX = m_leftMaxX;
                m_sectionMaxX = m_leftMaxX;
                m_rightMinX = m_leftMaxX;
                m_rightMaxX = m_leftMaxX;
            }

            length = m_rightMaxX - m_leftMinX;

            if (length < 1.0f) {
                length = 1.0f;
            }

            m_length = length;

            if (m_bridgeType == SECTIONAL_BRIDGE) {
                if (0.05 * length + m_sectionMinX < m_leftMaxX) {
                    m_bridgeType = FIXED_BRIDGE;
                }

                if (m_sectionMaxX - 0.05 * length > m_rightMinX) {
                    m_bridgeType = FIXED_BRIDGE;
                }
            }

            return true;
        }
    }
}

int W3DBridge::Get_Model_Vertices_Fixed(VertexFormatXYZNDUV1 *destination_vb,
    int cur_vertex,
    Matrix3D const &mtx,
    MeshClass *mesh,
    RefMultiListIterator<RenderObjClass> *lights)
{
    if (mesh == nullptr) {
        return 0;
    }

    Vector3 vec = m_end - m_start;

    if (vec.Length2() < 1.0f) {
        vec.Normalize();
    }

    Vector3 normal(-vec.Y, vec.X, 0.0f);
    normal.Normalize();
    float f = (m_end.Z - m_start.Z) / vec.Length();
    Vector3 vec_z(-f, 0.0f, GameMath::Sqrt(1.0f - f * f));
    vec /= m_length;
    normal *= m_scale;
    vec_z *= m_scale;
    return Get_Model_Vertices(destination_vb, cur_vertex, -m_leftMinX, vec, normal, vec_z, m_start, mtx, mesh, lights);
}

void W3DBridge::Render_Bridge(bool wireframe)
{
    if (m_visible) {
        if (m_numPolygons) {
            if (m_numVertex) {
                if (!wireframe) {
                    DX8Wrapper::Set_Texture(0, m_bridgeTexture);
                }

                DX8Wrapper::Draw_Triangles(m_firstIndex, m_numPolygons, m_firstVertex, m_numVertex);
            }
        }
    }
}

int W3DBridge::Get_Model_Indices(unsigned short *destination_ib, int cur_index, int vertex_offset, MeshClass *mesh)
{
    if (mesh == nullptr) {
        return 0;
    }

    int count = mesh->Peek_Model()->Get_Polygon_Count();
    const TriIndex *polys = mesh->Peek_Model()->Get_Polygon_Array();

    if (cur_index + 3 * count + 6 >= W3DBridgeBuffer::MAX_BRIDGE_INDEX) {
        return 0;
    }

    unsigned short *index = &destination_ib[cur_index];

    for (int i = 0; i < count; i++) {
        *index = polys[i].I + vertex_offset;
        index++;
        *index = polys[i].J + vertex_offset;
        index++;
        *index = polys[i].K + vertex_offset;
        index++;
    }

    return 3 * count;
}

int W3DBridge::Get_Model_Vertices(VertexFormatXYZNDUV1 *destination_vb,
    int cur_vertex,
    float x_offset,
    Vector3 &vec,
    Vector3 &vec_normal,
    Vector3 &vec_z,
    Vector3 &offset,
    Matrix3D const &mtx,
    MeshClass *mesh,
    RefMultiListIterator<RenderObjClass> *lights)
{
    if (mesh == nullptr) {
        return 0;
    }

    int count = mesh->Peek_Model()->Get_Vertex_Count();
    Vector3 *verts = mesh->Peek_Model()->Get_Vertex_Array();
    const Vector3 *normals = mesh->Peek_Model()->Get_Vertex_Normal_Array();

    if (cur_vertex + count + 2 >= W3DBridgeBuffer::MAX_BRIDGE_VERTEX) {
        return 0;
    }

    Vector3 light[LIGHT_COUNT];

    for (int i = 0; i < g_theWriteableGlobalData->m_numberGlobalLights; i++) {
        light[i].Set(-g_theWriteableGlobalData->m_terrainLightPos[i].x,
            -g_theWriteableGlobalData->m_terrainLightPos[i].y,
            -g_theWriteableGlobalData->m_terrainLightPos[i].z);
        light[i].Normalize();
    }

    const Vector2 *uvs = mesh->Peek_Model()->Get_UV_Array_By_Index(0);
    VertexFormatXYZNDUV1 *vertex = &destination_vb[cur_vertex];

    for (int i = 0; i < count; i++) {
        Vector3 pos;
        Vector3 out;
        Matrix3D::Transform_Vector(mtx, verts[i], &out);
        pos = ((out.X + x_offset) * vec) + (out.Y * vec_normal) + (out.Z * vec_z);
        pos.X += m_start.X;
        pos.Y += m_start.Y;
        pos.Z += m_start.Z;
        vertex->x = pos.X;
        vertex->y = pos.Y;
        vertex->z = pos.Z;

        VertexFormatXYZDUV2 vb;
        vb.x = pos.X;
        vb.y = pos.Y;
        vb.z = pos.Z;
        Vector3 normal;
        Matrix3D::Rotate_Vector(mtx, normals[i], &normal);
        normal = (normal.X * vec) + (normal.Y * vec_normal) + (normal.Z * vec_z);
        normal.Normalize();

        g_theTerrainRenderObject->Do_The_Light(&vb, light, &normal, nullptr, 1);
        vertex->nx = 0.0f;
        vertex->ny = 0.0f;
        vertex->nz = 1.0f;

        vertex->diffuse = vb.diffuse | 0xFF000000;
        vertex->u1 = uvs[i].X;
        vertex->v1 = uvs[i].Y;
        vertex++;
    }

    return count;
}

void W3DBridge::Get_Indices_And_Vertices(unsigned short *destination_ib,
    VertexFormatXYZNDUV1 *destination_vb,
    int *cur_index,
    int *cur_vertex,
    RefMultiListIterator<RenderObjClass> *lights)
{
    m_firstVertex = *cur_vertex;
    m_firstIndex = *cur_index;
    m_numVertex = 0;
    m_numPolygons = 0;

    if (m_sectionMesh == nullptr) {
        int vcount = Get_Model_Vertices_Fixed(destination_vb, *cur_vertex, m_leftMtx, m_leftMesh, lights);

        if (!vcount) {
            captainslog_dbgassert(vcount, "W3DBridge::GetIndicesNVertices(). Vertex overflow.");
            return;
        }

        int icount = Get_Model_Indices(destination_ib, *cur_index, *cur_vertex, m_leftMesh);

        if (!icount) {
            captainslog_dbgassert(icount, "W3DBridge::GetIndicesNVertices(). Index overflow.");
            return;
        }

        *cur_index += icount;
        *cur_vertex += vcount;
        m_numVertex += vcount;
        m_numPolygons += icount / 3;
    } else {
        Vector3 vec = m_end - m_start;

        if (vec.Length2() < 1.0f) {
            vec.Normalize();
        }

        Vector3 vec_normal(-vec.Y, vec.X, 0.0f);
        vec_normal.Normalize();
        vec_normal *= m_scale;

        float f = (m_end.Z - m_start.Z) / vec.Length();
        Vector3 vec_z(-f, 0.0f, GameMath::Sqrt(1.0f - f * f));
        vec_z *= m_scale;
        float section_span_length = m_rightMinX - m_leftMaxX;
        int section_span_count = 1;

        if (m_bridgeType == SECTIONAL_BRIDGE) {
            float f3 = vec.Length() - (m_length - section_span_length);
            section_span_count = GameMath::Fast_To_Int_Floor((section_span_length / 2.0f + f3) / section_span_length);
        }

        vec /= (float)(section_span_count - 1) * section_span_length + m_length;
        float x_offset = -m_leftMinX;
        int vcount = Get_Model_Vertices(
            destination_vb, *cur_vertex, x_offset, vec, vec_normal, vec_z, m_start, m_leftMtx, m_leftMesh, lights);

        if (!vcount) {
            captainslog_dbgassert(vcount, "W3DBridge::GetIndicesNVertices(). Vertex overflow.");
            return;
        }

        int icount = Get_Model_Indices(destination_ib, *cur_index, *cur_vertex, m_leftMesh);

        if (!icount) {
            captainslog_dbgassert(icount, "W3DBridge::GetIndicesNVertices(). Index overflow.");
            return;
        }

        *cur_index += icount;
        *cur_vertex += vcount;
        m_numVertex += vcount;
        m_numPolygons += icount / 3;

        for (int i = 0; i < section_span_count; i++) {
            vcount = Get_Model_Vertices(destination_vb,
                *cur_vertex,
                (float)i * section_span_length + x_offset,
                vec,
                vec_normal,
                vec_z,
                m_start,
                m_sectionMtx,
                m_sectionMesh,
                lights);

            if (!vcount) {
                captainslog_dbgassert(vcount, "W3DBridge::GetIndicesNVertices(). Vertex overflow.");
                return;
            }

            icount = Get_Model_Indices(destination_ib, *cur_index, *cur_vertex, m_sectionMesh);

            if (!icount) {
                captainslog_dbgassert(icount, "W3DBridge::GetIndicesNVertices(). Index overflow.");
                return;
            }

            *cur_index += icount;
            *cur_vertex += vcount;
            m_numVertex += vcount;
            m_numPolygons += icount / 3;
        }

        vcount = Get_Model_Vertices(destination_vb,
            *cur_vertex,
            (float)(section_span_count - 1) * section_span_length + x_offset,
            vec,
            vec_normal,
            vec_z,
            m_start,
            m_rightMtx,
            m_rightMesh,
            lights);

        if (!vcount) {
            captainslog_dbgassert(vcount, "W3DBridge::GetIndicesNVertices(). Vertex overflow.");
            return;
        }

        icount = Get_Model_Indices(destination_ib, *cur_index, *cur_vertex, m_rightMesh);

        if (!icount) {
            captainslog_dbgassert(icount, "W3DBridge::GetIndicesNVertices(). Index overflow.");
            return;
        }

        *cur_index += icount;
        *cur_vertex += vcount;
        m_numVertex += vcount;
        m_numPolygons += icount / 3;
    }
}

void W3DBridgeBuffer::Cull(CameraClass *camera)
{
    m_anythingChanged = m_updateVis;

    for (int i = 0; i < m_numBridges; i++) {
        if (m_bridges[i].Cull_Bridge(camera)) {
            m_anythingChanged = true;
        }
    }
}

void W3DBridgeBuffer::Load_Bridges_In_Vertex_And_Index_Buffers(RefMultiListIterator<class RenderObjClass> *lights)
{
#ifdef BUILD_WITH_D3D8
    if (m_indexBridge != nullptr && m_vertexBridge != nullptr) {
        if (m_initialized) {
            m_curNumBridgeVertices = 0;
            m_curNumBridgeIndices = 0;

            IndexBufferClass::WriteLockClass ib_lock(m_indexBridge, D3DLOCK_DISCARD);
            VertexBufferClass::WriteLockClass vb_lock(m_vertexBridge, D3DLOCK_DISCARD);
            VertexFormatXYZNDUV1 *destination_vb = static_cast<VertexFormatXYZNDUV1 *>(vb_lock.Get_Vertex_Array());
            unsigned short *destination_ib = ib_lock.Get_Index_Array();

            for (int i = 0; i < m_numBridges; i++) {
                m_bridges[i].Get_Indices_And_Vertices(
                    destination_ib, destination_vb, &m_curNumBridgeIndices, &m_curNumBridgeVertices, lights);
            }
        }
    }
#endif
}

W3DBridgeBuffer::~W3DBridgeBuffer()
{
    Free_Bridge_Buffers();
}

W3DBridgeBuffer::W3DBridgeBuffer() :
    m_vertexBridge(nullptr),
    m_indexBridge(nullptr),
    m_vertexMaterial(nullptr),
    m_bridgeTexture(nullptr),
    m_curNumBridgeVertices(0),
    m_curNumBridgeIndices(0),
    m_initialized(false)
{
    Clear_All_Bridges();
    Allocate_Bridge_Buffers();
    m_initialized = true;
}

void W3DBridgeBuffer::Free_Bridge_Buffers()
{
    Ref_Ptr_Release(m_vertexBridge);
    Ref_Ptr_Release(m_indexBridge);
    Ref_Ptr_Release(m_vertexMaterial);
}

void W3DBridgeBuffer::Allocate_Bridge_Buffers()
{
    m_vertexBridge =
        new DX8VertexBufferClass(DX8_FVF_XYZNDUV1, MAX_BRIDGE_VERTEX + 4, DX8VertexBufferClass::USAGE_DYNAMIC, 0);
    m_indexBridge = new DX8IndexBufferClass(MAX_BRIDGE_INDEX + 4, DX8IndexBufferClass::USAGE_DYNAMIC);
    m_vertexMaterial = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
    m_curNumBridgeVertices = 0;
    m_curNumBridgeIndices = 0;
}

void W3DBridgeBuffer::Clear_All_Bridges()
{
    for (int i = 0; i < m_numBridges; i++) {
        m_bridges[i].Clear_Bridge();
    }
    m_curNumBridgeIndices = 0;
    m_numBridges = 0;
}

void W3DBridgeBuffer::Load_Bridges(W3DTerrainLogic *terrain_logic, bool unk)
{
    Clear_All_Bridges();

    for (MapObject *obj = MapObject::Get_First_Map_Object(); obj; obj = obj->Get_Next()) {
        if (obj->Get_Flag(FLAG_BRIDGE_POINT1)) {
            MapObject *next = obj->Get_Next();

            if (next == nullptr || !next->Get_Flag(FLAG_BRIDGE_POINT2)) {
                captainslog_debug("Missing second bridge point.  Ignoring first.");
            }

            if (next == nullptr) {
                break;
            }

            if (next->Get_Flag(FLAG_BRIDGE_POINT2)) {
                Vector3 point1;
                Vector3 point2;
                point1.Set(obj->Get_Location()->x, obj->Get_Location()->y, 0.0f);
                point1.Z = g_theTerrainRenderObject->Get_Height_Map_Height(point1.X, point1.Y, nullptr) + 0.25f;
                point2.Set(next->Get_Location()->x, next->Get_Location()->y, 0.0f);
                point2.Z = g_theTerrainRenderObject->Get_Height_Map_Height(point2.X, point2.Y, nullptr) + 0.25f;

                Add_Bridge(point1, point2, obj->Get_Name(), terrain_logic, obj->Get_Properties());
            }
        }
    }

    if (terrain_logic != nullptr) {
        terrain_logic->Update_Bridge_Damage_States();
    }
}

void W3DBridgeBuffer::Add_Bridge(
    Vector3 from_loc, Vector3 to_loc, Utf8String name, W3DTerrainLogic *terrain_logic, Dict *props)
{
    if (m_numBridges < MAX_BRIDGES && m_initialized) {
        m_bridges[m_numBridges].Init(from_loc, to_loc, name);

        if (m_bridges[m_numBridges].Load(BODY_PRISTINE)) {
            if (terrain_logic != nullptr) {
                BridgeInfo info;
                m_bridges[m_numBridges].Get_Bridge_Info(&info);
                info.bridge_index = m_numBridges;
                terrain_logic->Add_Bridge_To_Logic(&info, props, name);
            }

            m_numBridges++;
        }
    }
}

void W3DBridgeBuffer::Update_Center(CameraClass *camera, RefMultiListIterator<RenderObjClass> *lights)
{
    Cull(camera);

    if (m_anythingChanged || !m_curNumBridgeIndices) {
        Load_Bridges_In_Vertex_And_Index_Buffers(lights);
    }

    m_updateVis = false;
}

void W3DBridgeBuffer::Draw_Bridges(CameraClass *camera, bool wireframe, TextureClass *cloud_texture)
{
    if (g_theTerrainLogic != nullptr) {
        for (int i = 0; i < m_numBridges; i++) {
            m_bridges[i].Set_Enabled(false);
        }

        bool draw = false;

        for (Bridge *bridge = g_theTerrainLogic->Get_First_Bridge(); bridge; bridge = bridge->Get_Next()) {
            BridgeInfo info;
            bridge->Get_Bridge_Info(&info);

            if (info.bridge_index >= 0 && info.bridge_index < m_numBridges) {
                m_bridges[info.bridge_index].Set_Enabled(true);

                if (m_bridges[info.bridge_index].Get_Damage_State() != info.cur_damage_state) {
                    BodyDamageType cur_damage_state = m_bridges[info.bridge_index].Get_Damage_State();
                    m_bridges[info.bridge_index].Set_Damage_State(info.cur_damage_state);
                    draw = true;

                    if (!m_bridges[info.bridge_index].Load(info.cur_damage_state)) {
                        m_bridges[info.bridge_index].Load(cur_damage_state);
                        m_bridges[info.bridge_index].Set_Damage_State(cur_damage_state);
                    }
                }
            }
        }

        if (draw) {
            Load_Bridges_In_Vertex_And_Index_Buffers(nullptr);
        }
    } else {
        for (int i = 0; i < m_numBridges; i++) {
            m_bridges[i].Set_Enabled(true);
        }
    }

    if (m_curNumBridgeIndices) {
        DX8Wrapper::Set_Material(m_vertexMaterial);
        DX8Wrapper::Set_Index_Buffer(m_indexBridge, 0);
        DX8Wrapper::Set_Vertex_Buffer(m_vertexBridge, 0);
        DX8Wrapper::Set_Shader(g_detailAlphaShader);
        DX8Wrapper::Apply_Render_State_Changes();

        if (!wireframe) {
            if (cloud_texture != nullptr) {
                W3DShaderManager::Set_Texture(1, cloud_texture);
                W3DShaderManager::Set_Shader(W3DShaderManager::ST_CLOUD_TEXTURE, 1);
            }
        }

        for (int i = 0; i < m_numBridges; i++) {
            if (m_bridges[i].Is_Enabled()) {
                if (m_bridges[i].Is_Visible()) {
                    m_bridges[i].Render_Bridge(wireframe);
                }
            }
        }

        if (!wireframe) {
            if (cloud_texture != nullptr) {
                W3DShaderManager::Reset_Shader(W3DShaderManager::ST_CLOUD_TEXTURE);
            }
        }

        if (!wireframe) {
            if (g_theTerrainRenderObject->Get_Shroud()) {
                DX8Wrapper::Invalidate_Cached_Render_States();
                DX8Wrapper::Set_Shader(ShaderClass::s_presetOpaqueShader);
                DX8Wrapper::Set_Material(m_vertexMaterial);
                DX8Wrapper::Set_Index_Buffer(m_indexBridge, 0);
                DX8Wrapper::Set_Vertex_Buffer(m_vertexBridge, 0);
                DX8Wrapper::Apply_Render_State_Changes();
                W3DShaderManager::Set_Texture(0, g_theTerrainRenderObject->Get_Shroud()->Get_Shroud_Texture());
                W3DShaderManager::Set_Shader(W3DShaderManager::ST_SHROUD_TEXTURE, 0);

                for (int i = 0; i < m_numBridges; i++) {
                    if (m_bridges[i].Is_Enabled()) {
                        if (m_bridges[i].Is_Visible()) {
                            m_bridges[i].Render_Bridge(true);
                        }
                    }
                }

                W3DShaderManager::Reset_Shader(W3DShaderManager::ST_SHROUD_TEXTURE);
            }
        }
    }
}

void W3DBridgeBuffer::World_Builder_Update_Bridge_Towers(GameAssetManager *assetmgr, SimpleSceneClass *scene)
{
    // heightmap TODO
#ifdef GAME_DLL
    Call_Method<void, W3DBridgeBuffer, GameAssetManager *, SimpleSceneClass *>(
        PICK_ADDRESS(0x0, 0x005FF8C8), this, assetmgr, scene); // does not exist in game exe
#endif
}
