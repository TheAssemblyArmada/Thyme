/**
 * @file
 *
 * @author tomsons26
 *
 * @brief
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dvolumetricshadow.h"
#include "baseheightmap.h"
#include "drawable.h"
#include "globaldata.h"
#include "hlod.h"
#include "mesh.h"
#include "meshmdl.h"
#include "terrainlogic.h"
#include "view.h"
#include "w3dbuffermanager.h"
#include <cmath>
#include <new>
#ifdef BUILD_WITH_D3D8
#include <d3dx8.h>
#endif

#ifndef GAME_DLL
W3DVolumetricShadowManager *g_theW3DVolumetricShadowManager;
#endif

#ifdef BUILD_WITH_D3D8
#ifndef GAME_DLL
IDirect3DVertexBuffer8 *g_shadowVertexBufferD3D;
IDirect3DIndexBuffer8 *g_shadowIndexBufferD3D;
#else
extern IDirect3DVertexBuffer8 *&g_shadowVertexBufferD3D;
extern IDirect3DIndexBuffer8 *&g_shadowIndexBufferD3D;
#endif
static IDirect3DVertexBuffer8 *s_lastActiveVertexBuffer;
#endif

static float s_bcX;
static float s_bcY;
static float s_bcZ;
static float s_beX;
static float s_beY;
static float s_beZ;
static float s_cosAngleToCare = GameMath::Cos(0.003490658601125081f);

int g_nShadowVertsInBuf;
int g_nShadowStartBatchVertex;
int g_nShadowIndicesInBuf;
int g_nShadowStartBatchIndex;

W3DShadowGeometryMesh::W3DShadowGeometryMesh() :
    // BUGFIX Init all members
    m_mesh(nullptr),
    m_modelIndex(0),
    m_verts(nullptr),
    m_polygonNormals(nullptr),
    m_numVerts(0),
    m_numPolygons(0),
    m_polygons(nullptr),
    m_parentVerts(nullptr),
    m_polyNeighbors(nullptr),
    m_numPolyNeighbors(0),
    m_parentGeometry(nullptr)
{
}

W3DShadowGeometryMesh::~W3DShadowGeometryMesh()
{
    Delete_Neighbors();

    if (m_parentVerts != nullptr) {
        delete[] m_parentVerts;
    }

    if (m_polygonNormals != nullptr) {
        delete[] m_polygonNormals;
    }
}

PolyNeighbor *W3DShadowGeometryMesh::Get_Poly_Neighbor(int index)
{
    if (m_polyNeighbors == nullptr) {
        Build_Polygon_Neighbors();
    }

    if (index < 0 || index >= m_numPolyNeighbors) {
        captainslog_assert(0);
        return nullptr;
    }

    return &m_polyNeighbors[index];
}

void W3DShadowGeometryMesh::Build_Polygon_Neighbors()
{
    Build_Polygon_Normals();

    const int num = Get_Num_Polygon();

    if (num == 0) {
        if (m_numPolyNeighbors != 0) {
            Delete_Neighbors();
        }
        return;
    }

    if (num != m_numPolyNeighbors) {
        Delete_Neighbors();

        if (!Allocate_Neighbors(num)) {
            return;
        }
    }

    for (int i = 0; i < m_numPolyNeighbors; ++i) {

        m_polyNeighbors[i].myIndex = i;

        for (int j = 0; j < 3; ++j) {
            m_polyNeighbors[i].neighbor[j].neighborIndex = -1;
        }
    }

    for (int i = 0; i < m_numPolyNeighbors; ++i) {

        short poly[3];
        Get_Polygon_Index(i, poly);
        Vector3 *normal = Get_Polygon_Normal(i);

        for (int j = 0; j < m_numPolyNeighbors; ++j) {

            if (i != j) {

                short other_poly[3];
                Get_Polygon_Index(j, other_poly);

                int index1_pos[2];

                int index1 = -1;
                int index2 = -1;

                for (int k = 0; k < 3; ++k) {

                    for (int l = 0; l < 3; ++l) {

                        if (poly[k] == other_poly[l]) {

                            if (index1 == -1) {
                                index1 = poly[k];
                                index1_pos[0] = k;
                                index1_pos[1] = l;

                            } else if (index2 == -1) {

                                int p0_dist = k - index1_pos[0];
                                int p1_dist = l - index1_pos[1];

                                unsigned int p0 = (p0_dist & 0x80000000) ^ (abs(p0_dist) & 2) << 30;
                                unsigned int p1 = (p1_dist & 0x80000000) ^ (abs(p1_dist) & 2) << 30;

                                if (p0 != p1) {

                                    Vector3 *other_normal = Get_Polygon_Normal(j);

                                    const float epsilon = 1.0f / 100.0f;

                                    if (GameMath::Fabs((*other_normal * *normal) + 1.0f) > epsilon) {
                                        index2 = poly[k];
                                    }
                                }

                            } else {
                                index2 = -1;
                                index1 = -1;
                            }
                        }
                    }
                }

                if (index1 != -1 && index2 != -1) {
                    // found a shared edge
                    for (int edge = 0; edge < 3; ++edge) {
                        if (m_polyNeighbors[i].neighbor[edge].neighborIndex == -1) {
                            m_polyNeighbors[i].neighbor[edge].neighborIndex = j;
                            m_polyNeighbors[i].neighbor[edge].neighborEdgeIndex[0] = index1;
                            m_polyNeighbors[i].neighbor[edge].neighborEdgeIndex[1] = index2;
                            break;
                        }
                    }
                }
            }
        }
    }
}

bool W3DShadowGeometryMesh::Allocate_Neighbors(int num_polys)
{
    captainslog_assert(m_numPolyNeighbors == 0);
    captainslog_assert(m_polyNeighbors == nullptr);

    m_polyNeighbors = new PolyNeighbor[num_polys];

    if (m_polyNeighbors == nullptr) {
        captainslog_assert(0);
        return false;
    }

    m_numPolyNeighbors = num_polys;
    return true;
}

void W3DShadowGeometryMesh::Delete_Neighbors()
{
    if (m_polyNeighbors != nullptr) {
        delete[] m_polyNeighbors;
        m_polyNeighbors = nullptr;
        m_numPolyNeighbors = 0;
    }
}

Vector3 *W3DShadowGeometryMesh::Get_Polygon_Normal(int index) const
{
    captainslog_assert(m_polygonNormals != nullptr);

    return &m_polygonNormals[index];
}

void W3DShadowGeometryMesh::Build_Polygon_Normals()
{
    if (m_polygonNormals == nullptr) {
        Vector3 *normals = new Vector3[m_numPolygons];

        for (int i = 0; i < m_numPolygons; ++i) {
            Get_Polygon_Normal(i, &normals[i]);
        }

        m_polygonNormals = normals;
    }
}

void W3DShadowGeometryMesh::Get_Polygon_Normal(int index, Vector3 *normal) const
{

    if (m_polygonNormals != nullptr) {
        *normal = m_polygonNormals[index];
        return;
    }

    short index_list[3];
    Get_Polygon_Index(index, index_list);

    Vector3 *v0 = Get_Vertex(index_list[0]);
    Vector3 *v1 = Get_Vertex(index_list[1]);
    Vector3 *v2 = Get_Vertex(index_list[2]);

    Vector3 b = *v1 - *v0;
    Vector3 a = *v1 - *v2;

    Vector3::Normalized_Cross_Product(a, b, normal);
}

void W3DShadowGeometryMesh::Get_Polygon_Index(int polygon_index, short *index_list) const
{
    const TriIndex *v = &m_polygons[polygon_index];
    index_list[0] = m_parentVerts[v->I];
    index_list[1] = m_parentVerts[v->J];
    index_list[2] = m_parentVerts[v->K];
}

W3DShadowGeometry::W3DShadowGeometry() : m_name{}, m_meshCount(0), m_numTotalsVerts(0) {}

void W3DShadowGeometry::Set_Name(const char *name)
{
    strlcpy(m_name, name, sizeof(m_name));
}

int W3DShadowGeometry::Init(RenderObjClass *robj)
{
    return 1;
}

int W3DShadowGeometry::Init_From_HLOD(RenderObjClass *robj)
{
    HLodClass *hlod = static_cast<HLodClass *>(robj);

    int lod_index = hlod->Get_LOD_Count() - 1;

    captainslog_dbgassert(m_meshCount < MAX_SHADOW_CASTER_MESHES, "Too many shadow sub-meshes");

    W3DShadowGeometryMesh *geo_mesh = &m_meshList[m_meshCount];

    m_numTotalsVerts = 0;
    for (int model_index = 0; model_index < hlod->Get_Lod_Model_Count(lod_index); ++model_index) {

        RenderObjClass *lod_model = hlod->Peek_Lod_Model(lod_index, model_index);

        if (lod_model != nullptr) {

            if (lod_model->Class_ID() == RenderObjClass::CLASSID_MESH) {

                captainslog_dbgassert(m_meshCount < MAX_SHADOW_CASTER_MESHES, "Too many shadow sub-meshes");

                MeshClass *mesh = static_cast<MeshClass *>(lod_model);

                geo_mesh->m_mesh = mesh;
                geo_mesh->m_modelIndex = model_index;

                MeshModelClass *model = geo_mesh->m_mesh->Peek_Model();

                if (geo_mesh->m_mesh->Is_Alpha() || geo_mesh->m_mesh->Is_Translucent()) {

                    if (!model->Get_Flag(MeshGeometryClass::CAST_SHADOW)) {
                        continue;
                    }
                }

                if (model->Get_Flag(MeshGeometryClass::SKIN)) {
                    continue;
                }

                geo_mesh->m_numVerts = model->Get_Vertex_Count();
                geo_mesh->m_verts = model->Get_Vertex_Array();
                geo_mesh->m_numPolygons = model->Get_Polygon_Count();
                geo_mesh->m_polygons = model->Get_Polygon_Array();

                if (geo_mesh->m_numVerts > 16384) {
                    return 0;
                }

                unsigned short verts[16384];
                memset(verts, -1, sizeof(verts));

                int num = geo_mesh->m_numVerts;

                for (int i = 0; i < geo_mesh->m_numVerts; ++i) {
                    if (verts[i] == 0xFFFF) {

                        for (int j = i + 1; j < geo_mesh->m_numVerts; ++j) {

                            Vector3 d = geo_mesh->m_verts[i] - geo_mesh->m_verts[j];

                            if (d.Length2() == 0.0f) {
                                verts[j] = i;
                                --num;
                            }
                        }
                        verts[i] = i;
                    }
                }

                geo_mesh->m_parentVerts = new unsigned short[geo_mesh->m_numVerts];
                memcpy(geo_mesh->m_parentVerts, verts, geo_mesh->m_numVerts * sizeof(unsigned short));

                geo_mesh->m_numVerts = num;
                m_numTotalsVerts += num;
                geo_mesh->m_parentGeometry = this;

                ++geo_mesh;

                ++m_meshCount;
            }
        }

        if (lod_model != nullptr) {
            if (lod_model->Class_ID() == RenderObjClass::CLASSID_SHDMESH) {
                captainslog_dbgassert(0, "Meshes using shaders are not supported! How did you end up here!?");
            }
        }
    }
    return m_meshCount != 0;
}

int W3DShadowGeometry::Init_From_Mesh(RenderObjClass *robj)
{
    captainslog_dbgassert(m_meshCount < MAX_SHADOW_CASTER_MESHES, "Too many shadow sub-meshes");

    W3DShadowGeometryMesh *geo_mesh = &m_meshList[m_meshCount];

    MeshClass *mesh = static_cast<MeshClass *>(robj);

    geo_mesh->m_mesh = mesh;

    geo_mesh->m_modelIndex = -1;

    MeshModelClass *model = geo_mesh->m_mesh->Peek_Model();

    if (geo_mesh->m_mesh->Is_Alpha() || geo_mesh->m_mesh->Is_Translucent()) {
        if (!model->Get_Flag(MeshGeometryClass::CAST_SHADOW)) {
            return 0;
        }
    }

    geo_mesh->m_numVerts = model->Get_Vertex_Count();
    geo_mesh->m_verts = model->Get_Vertex_Array();
    geo_mesh->m_numPolygons = model->Get_Polygon_Count();
    geo_mesh->m_polygons = model->Get_Polygon_Array();

    if (geo_mesh->m_numVerts > 16384) {
        return 0;
    }

    unsigned short verts[16384];
    memset(verts, -1, sizeof(verts));

    int num = geo_mesh->m_numVerts;

    for (int i = 0; i < geo_mesh->m_numVerts; ++i) {
        if (verts[i] == 0xFFFF) {

            for (int j = i + 1; j < geo_mesh->m_numVerts; ++j) {

                Vector3 d = geo_mesh->m_verts[i] - geo_mesh->m_verts[j];

                if (d.Length2() == 0.0f) {
                    verts[j] = i;
                    --num;
                }
            }
            verts[i] = i;
        }
    }

    geo_mesh->m_parentVerts = new unsigned short[geo_mesh->m_numVerts];
    memcpy(geo_mesh->m_parentVerts, verts, geo_mesh->m_numVerts * sizeof(unsigned short));

    geo_mesh->m_numVerts = num;
    geo_mesh->m_parentGeometry = this;

    m_meshCount = 1;
    m_numTotalsVerts = num;

    return 1;
}

W3DShadowGeometryManager::W3DShadowGeometryManager()
{
    m_geomPtrTable = new HashTableClass(2048);
    m_missingGeomTable = new HashTableClass(2048);
}

W3DShadowGeometryManager::~W3DShadowGeometryManager()
{
    Free_All_Geoms();

    if (m_geomPtrTable != nullptr) {
        delete m_geomPtrTable;
    }

    if (m_missingGeomTable != nullptr) {
        delete m_missingGeomTable;
    }
}

void W3DShadowGeometryManager::Free_All_Geoms()
{
    W3DShadowGeometryManagerIterator it(*this);

    for (it.First(); !it.Is_Done(); it.Next()) {
        W3DShadowGeometry *geo = it.Get_Current_Geom();
        geo->Release_Ref();
    }

    m_geomPtrTable->Reset();
}

W3DShadowGeometry *W3DShadowGeometryManager::Peek_Geom(const char *name)
{
    return static_cast<W3DShadowGeometry *>(m_geomPtrTable->Find(name));
}

W3DShadowGeometry *W3DShadowGeometryManager::Get_Geom(const char *name)
{
    W3DShadowGeometry *geo = Peek_Geom(name);

    if (geo != nullptr) {
        geo->Add_Ref();
    }

    return geo;
}

bool W3DShadowGeometryManager::Add_Geom(W3DShadowGeometry *new_geom)
{

    captainslog_assert(new_geom != nullptr);

    new_geom->Add_Ref();
    m_geomPtrTable->Add(new_geom);

    return true;
}

void W3DShadowGeometryManager::Register_Missing(const char *name)
{
    m_missingGeomTable->Add(new MissingGeomClass(name));
}

bool W3DShadowGeometryManager::Is_Missing(const char *name) const
{
    return m_missingGeomTable->Find(name) != nullptr;
}

int W3DShadowGeometryManager::Load_Geom(RenderObjClass *robj, const char *name)
{
    bool state = false;

    W3DShadowGeometry *geo = new W3DShadowGeometry;

    if (geo == nullptr) {
        return 1;
    }

    geo->Set_Name(name);

    switch (robj->Class_ID()) {
        case RenderObjClass::CLASSID_MESH:
            state = geo->Init_From_Mesh(robj) != 0;
            break;

        case RenderObjClass::CLASSID_HLOD:
            state = geo->Init_From_HLOD(robj) != 0;
            break;

        default:
            break;
    }

    if (!state) {
        geo->Release_Ref();
        return 1;
    }

    if (Peek_Geom(geo->Get_Name())) {
        geo->Release_Ref();
        return 1;
    }

    Add_Geom(geo);
    geo->Release_Ref();

    return 0;
}

W3DShadowGeometry *W3DShadowGeometryManagerIterator::Get_Current_Geom()
{
    return static_cast<W3DShadowGeometry *>(Get_Current());
}

W3DVolumetricShadowManager::W3DVolumetricShadowManager() : m_shadowList(nullptr), m_dynamicShadowVolumesToRender(nullptr)
{
    m_W3DShadowGeometryManager = new W3DShadowGeometryManager;
    g_theW3DBufferManager = new W3DBufferManager;
}

W3DVolumetricShadowManager::~W3DVolumetricShadowManager()
{
    Release_Resources();

    if (m_W3DShadowGeometryManager != nullptr) {
        delete m_W3DShadowGeometryManager;
        m_W3DShadowGeometryManager = nullptr;
    }

    if (g_theW3DBufferManager != nullptr) {
        delete g_theW3DBufferManager;
        g_theW3DBufferManager = nullptr;
    }
}

bool W3DVolumetricShadowManager::Init()
{
    return true;
}

void W3DVolumetricShadowManager::Reset()
{
    m_W3DShadowGeometryManager->Free_All_Geoms();
    g_theW3DBufferManager->Free_All_Buffers();
}

void W3DVolumetricShadowManager::Release_Resources()
{
#ifdef BUILD_WITH_D3D8
    if (g_shadowIndexBufferD3D != nullptr) {
        g_shadowIndexBufferD3D->Release();
        g_shadowIndexBufferD3D = nullptr;
    }

    if (g_shadowVertexBufferD3D != nullptr) {
        g_shadowVertexBufferD3D->Release();
        g_shadowVertexBufferD3D = nullptr;
    }
#endif

    if (g_theW3DBufferManager != nullptr) {
        g_theW3DBufferManager->Release_Resources();
        Invalidate_Cached_Light_Positions();
    }
}

bool W3DVolumetricShadowManager::Re_Acquire_Resources()
{
    Release_Resources();
#ifdef BUILD_WITH_D3D8
    IDirect3DDevice8 *device = DX8Wrapper::Get_D3D_Device8();
    captainslog_dbgassert(device, "Trying to Re_Acquire_Resources on W3DVolumetricShadowManager without device");

    if (FAILED(device->CreateIndexBuffer(sizeof(unsigned short) * SHADOW_INDEX_SIZE,
            D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
            D3DFMT_INDEX16,
            D3DPOOL_DEFAULT,
            &g_shadowIndexBufferD3D))) {
        return false;
    }

    if (!g_shadowVertexBufferD3D
        && FAILED(device->CreateVertexBuffer(sizeof(Vector3) * SHADOW_VERTEX_SIZE,
            D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
            0,
            D3DPOOL_DEFAULT,
            &g_shadowVertexBufferD3D))) {
        return false;
    }

    if (g_theW3DBufferManager == nullptr || g_theW3DBufferManager->ReAcquire_Resources()) {
        return true;
    }
#endif
    return false;
}

void W3DVolumetricShadowManager::Remove_Shadow(W3DVolumetricShadow *shadow)
{
    W3DVolumetricShadow *prev = nullptr;

    for (W3DVolumetricShadow *i = m_shadowList; i != nullptr; i = i->m_next) {
        if (i == shadow) {
            if (prev != nullptr) {
                prev->m_next = shadow->m_next;
            } else {
                m_shadowList = shadow->m_next;
            }

            delete shadow;
            return;
        }

        prev = i;
    }
}

void W3DVolumetricShadowManager::Remove_All_Shadows()
{
    W3DVolumetricShadow *next;

    for (W3DVolumetricShadow *i = m_shadowList; i != nullptr; i = next) {
        next = i->m_next;
        i->m_next = nullptr;
        delete i;
    }

    m_shadowList = nullptr;
}

void W3DVolumetricShadowManager::Add_Dynamic_Shadow_Task(W3DVolumetricShadowRenderTask *task)
{
    W3DVolumetricShadowRenderTask *cur = m_dynamicShadowVolumesToRender;
    m_dynamicShadowVolumesToRender = task;
    m_dynamicShadowVolumesToRender->m_nextTask = cur;
}

void W3DVolumetricShadowManager::Render_Stencil_Shadows()
{
#ifdef BUILD_WITH_D3D8
    struct _TRANS_LIT_VERTEX
    {
        D3DXVECTOR4 p;
        unsigned long color;
    };

    IDirect3DDevice8 *dev = DX8Wrapper::Get_D3D_Device8();
    if (dev != nullptr) {
        _TRANS_LIT_VERTEX vertex[4];

        int32_t x;
        int32_t y;

        g_theTacticalView->Get_Origin(&x, &y);

        int32_t width = g_theTacticalView->Get_Width();
        int32_t height = g_theTacticalView->Get_Height();

        float fy = float(height + y);
        float fx = float(width + x);

        vertex[0].p = D3DXVECTOR4(fx, fy, 0.0f, 1.0f);
        vertex[1].p = D3DXVECTOR4(fx, 0.0f, 0.0f, 1.0f);
        vertex[2].p = D3DXVECTOR4(float(x), fy, 0.0f, 1.0f);
        vertex[3].p = D3DXVECTOR4(float(x), 0.0f, 0.0f, 1.0f);

        vertex[0].color = g_theW3DShadowManager->Get_Shadow_Color();
        vertex[1].color = g_theW3DShadowManager->Get_Shadow_Color();
        vertex[2].color = g_theW3DShadowManager->Get_Shadow_Color();
        vertex[3].color = g_theW3DShadowManager->Get_Shadow_Color();

        dev->SetVertexShader(D3DFVF_DIFFUSE | D3DFVF_XYZRHW);
        dev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
        dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
        dev->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
        dev->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
        dev->SetRenderState(D3DRS_STENCILENABLE, TRUE);
        dev->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_LESSEQUAL);
        dev->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
        dev->SetRenderState(D3DRS_STENCILMASK, ~g_theW3DShadowManager->Get_Stencil_Mask());
        dev->SetRenderState(D3DRS_STENCILREF, 1);
        dev->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);

        if (DX8Wrapper::Is_Triangle_Draw_Enabled()) {
            dev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertex, sizeof(_TRANS_LIT_VERTEX));
        }

        dev->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
        dev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        dev->SetRenderState(D3DRS_STENCILENABLE, FALSE);
    }
#endif
}

void W3DVolumetricShadowManager::Render_Shadows(bool force_stencil_fill)
{
#ifdef BUILD_WITH_D3D8
    AABoxClass aabox;
    SphereClass sphere;

    g_theTerrainRenderObject->Get_Maximum_Visible_Box(*g_shadowCameraFrustum, &aabox, 1);

    s_bcX = aabox.m_center.X;
    s_bcY = aabox.m_center.Y;
    s_bcZ = aabox.m_center.Z;
    s_beX = aabox.m_extent.X;
    s_beY = aabox.m_extent.Y;
    s_beZ = aabox.m_extent.Z;

    if (m_shadowList != nullptr && g_theWriteableGlobalData->m_shadowVolumes) {
        IDirect3DDevice8 *dev = DX8Wrapper::Get_D3D_Device8();

        if (dev != nullptr) {
            g_nShadowIndicesInBuf = 0xFFFF;
            g_nShadowVertsInBuf = 0xFFFF;

            VertexMaterialClass *material = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
            DX8Wrapper::Set_Material(material);
            Ref_Ptr_Release(material);

            DX8Wrapper::Set_Shader(ShaderClass::s_presetOpaqueShader);
            DX8Wrapper::Set_Texture(0, nullptr);
            DX8Wrapper::Set_Texture(1, nullptr);
            DX8Wrapper::Apply_Render_State_Changes();

            dev->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
            dev->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
            dev->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
            dev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
            dev->SetRenderState(D3DRS_FOGENABLE, FALSE);
            dev->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);
            dev->SetRenderState(D3DRS_LIGHTING, FALSE);

            dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
            dev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
            dev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
            dev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
            dev->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
            dev->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
            dev->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
            dev->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);

            dev->SetTexture(0, nullptr);
            dev->SetTexture(1, nullptr);

            DWORD color_write = 0x12345678;

            if (DX8Wrapper::Get_Current_Caps()->Get_DX8_Caps().PrimitiveMiscCaps & D3DPMISCCAPS_COLORWRITEENABLE) {
                dev->GetRenderState(D3DRS_COLORWRITEENABLE, &color_write);
                DX8Wrapper::Set_DX8_Render_State(D3DRS_COLORWRITEENABLE, 0);
            } else {
                dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
                dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
                dev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
            }

            dev->SetRenderState(D3DRS_STENCILENABLE, TRUE);

            if (g_theW3DShadowManager->Get_Stencil_Mask() == 0x80808080) {
                dev->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_NOTEQUAL);
            } else {
                dev->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_GREATEREQUAL);
            }

            dev->SetRenderState(D3DRS_STENCILREF, 0x80808080);
            dev->SetRenderState(D3DRS_STENCILMASK, g_theW3DShadowManager->Get_Stencil_Mask());
            dev->SetRenderState(D3DRS_STENCILWRITEMASK, 0xFFFFFFFF);
            dev->SetRenderState(D3DRS_STENCILZFAIL, 1);
            dev->SetRenderState(D3DRS_STENCILFAIL, 1);
            dev->SetRenderState(D3DRS_STENCILPASS, 7);
            dev->SetVertexShader(2);
            dev->SetRenderState(D3DRS_CULLMODE, 2);

            s_lastActiveVertexBuffer = nullptr;
            m_dynamicShadowVolumesToRender = nullptr;

            for (W3DVolumetricShadow *k = m_shadowList; k != nullptr; k = k->m_next) {
                if (k->m_isEnabled && !k->m_isInvisibleEnabled) {
                    W3DVolumetricShadowRenderTask *task = m_dynamicShadowVolumesToRender;
                    k->Update();
                    W3DVolumetricShadowRenderTask *task2 = m_dynamicShadowVolumesToRender;

                    while (task2 != task) {
                        k->Render_Volume(task2->m_meshIndex, task2->m_lightIndex);
                        task2 = static_cast<W3DVolumetricShadowRenderTask *>(task2->m_nextTask);
                    }
                }
            }

            int format = W3DBufferManager::Get_DX8_Format(W3DBufferManager::VBM_FVF_XYZ);
            dev->SetVertexShader(format);

            for (W3DBufferManager::W3DVertexBuffer *vb =
                     g_theW3DBufferManager->Get_Next_Vertex_Buffer(nullptr, W3DBufferManager::VBM_FVF_XYZ);
                 vb != nullptr;
                 vb = g_theW3DBufferManager->Get_Next_Vertex_Buffer(vb, W3DBufferManager::VBM_FVF_XYZ)) {

                W3DVolumetricShadowRenderTask *task = static_cast<W3DVolumetricShadowRenderTask *>(vb->m_renderTaskList);

                while (task != nullptr) {
                    task->m_parentShadow->Render_Volume(task->m_meshIndex, task->m_lightIndex);
                    task = static_cast<W3DVolumetricShadowRenderTask *>(task->m_nextTask);
                }
            }

            dev->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_DECRSAT);
            dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

            for (W3DBufferManager::W3DVertexBuffer *vb =
                     g_theW3DBufferManager->Get_Next_Vertex_Buffer(nullptr, W3DBufferManager::VBM_FVF_XYZ);
                 vb != nullptr;
                 vb = g_theW3DBufferManager->Get_Next_Vertex_Buffer(vb, W3DBufferManager::VBM_FVF_XYZ)) {

                for (W3DVolumetricShadowRenderTask *task =
                         static_cast<W3DVolumetricShadowRenderTask *>(vb->m_renderTaskList);
                     task != nullptr;
                     task = static_cast<W3DVolumetricShadowRenderTask *>(task->m_nextTask)) {
                    task->m_parentShadow->Render_Volume(task->m_meshIndex, task->m_lightIndex);
                }
            }

            dev->SetVertexShader(D3DFVF_XYZ);

            for (W3DVolumetricShadowRenderTask *task = m_dynamicShadowVolumesToRender; task != nullptr;
                 task = static_cast<W3DVolumetricShadowRenderTask *>(task->m_nextTask)) {
                task->m_parentShadow->Render_Volume(task->m_meshIndex, task->m_lightIndex);
            }

            for (W3DBufferManager::W3DVertexBuffer *vb =
                     g_theW3DBufferManager->Get_Next_Vertex_Buffer(nullptr, W3DBufferManager::VBM_FVF_XYZ);
                 vb != nullptr;
                 vb = g_theW3DBufferManager->Get_Next_Vertex_Buffer(vb, W3DBufferManager::VBM_FVF_XYZ)) {
                vb->m_renderTaskList = nullptr;
            }

            dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);

            if (color_write != 0x12345678) {
                DX8Wrapper::Set_DX8_Render_State(D3DRS_COLORWRITEENABLE, color_write);
            }

            Render_Stencil_Shadows();

            dev->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
            dev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
            dev->SetRenderState(D3DRS_LIGHTING, FALSE);

            DX8Wrapper::Invalidate_Cached_Render_States();
        }
    } else if (force_stencil_fill) {

        VertexMaterialClass *vetmat = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
        DX8Wrapper::Set_Material(vetmat);
        Ref_Ptr_Release(vetmat);

        DX8Wrapper::Set_Shader(ShaderClass::s_presetOpaqueShader);
        DX8Wrapper::Set_Texture(0, nullptr);
        DX8Wrapper::Apply_Render_State_Changes();

        Render_Stencil_Shadows();
        DX8Wrapper::Invalidate_Cached_Render_States();
    }
#endif
}

void W3DVolumetricShadowManager::Invalidate_Cached_Light_Positions()
{
    if (m_shadowList != nullptr) {
        Vector3 light_pos(0.0f, 0.0f, 0.0f);

        for (W3DVolumetricShadow *i = m_shadowList; i != nullptr; i = i->m_next) {
            for (int j = 0; j < 1; j++) {
                for (int k = 0; k < MAX_SHADOW_CASTER_MESHES; k++) {
                    i->Set_Light_Pos_History(j, k, light_pos);
                }
            }
        }
    }
}

W3DVolumetricShadow *W3DVolumetricShadowManager::Add_Shadow(
    RenderObjClass *robj, Shadow::ShadowTypeInfo *shadow_info, Drawable *drawable)
{
    if (!DX8Wrapper::Has_Stencil() || robj == nullptr || !g_theWriteableGlobalData->m_shadowVolumes) {
        return nullptr;
    }

    const char *name = robj->Get_Name();

    if (name == nullptr) {
        return nullptr;
    }

    W3DShadowGeometry *geom = m_W3DShadowGeometryManager->Get_Geom(name);

    if (geom == nullptr) {
        m_W3DShadowGeometryManager->Load_Geom(robj, name);
        geom = m_W3DShadowGeometryManager->Get_Geom(name);

        if (geom == nullptr) {
            return nullptr;
        }
    }

    W3DVolumetricShadow *shadow = new W3DVolumetricShadow();
    shadow->Set_Render_Object(robj);
    shadow->Set_Geometry(geom);
    SphereClass sphere;
    robj->Get_Obj_Space_Bounding_Sphere(sphere);
    shadow->Set_Bounds_Radius(sphere.Radius * 1.0f);
    float scale = 0.0f;

    if (shadow_info->m_sizeX != 0.0f) {
        scale = GameMath::Tan(shadow_info->m_sizeX / 180.0f * GAMEMATH_PI);
    }

    shadow->Set_Shadow_Length_Scale(scale);

    if (drawable == nullptr || !drawable->Is_KindOf(KINDOF_IMMOBILE)) {
        shadow->Set_Optimal_Extrusion_Padding(0.1f);
    }

    shadow->m_next = m_shadowList;
    m_shadowList = shadow;
    return shadow;
}

Geometry::Geometry() :
    // BUGFIX init all members
    m_verts(nullptr),
    m_indices(nullptr),
    m_numPolygon(0),
    m_numVertex(0),
    m_numActivePolygon(0),
    m_numActiveVertex(0),
    m_flags(0),
    m_visibleState(STATE_UNKNOWN)
{
}

Geometry::~Geometry()
{
    Release();
}

bool Geometry::Create(int num_vert, int num_poly)
{
    if (num_vert) {
        // BUGFIX Geos can be reused so clean up to avoid memory leak
        if (m_verts != nullptr) {
            delete[] m_verts;
        }

        m_verts = new Vector3[num_vert];
        if (m_verts == nullptr) {
            return false;
        }
    }

    if (num_poly) {
        // BUGFIX Geos can be reused so clean up to avoid memory leak
        if (m_indices != nullptr) {
            delete[] m_indices;
        }

        m_indices = new unsigned short[3 * num_poly];

        if (m_indices == nullptr) {
            // BUGFIX clean up to avoid memory leak
            if (m_verts != nullptr) {
                delete[] m_verts;
            }

            return false;
        }
    }

    m_numPolygon = num_poly;
    m_numVertex = num_vert;
    m_numActivePolygon = 0;
    m_numActiveVertex = 0;

    return true;
}

void Geometry::Release()
{
    if (m_verts != nullptr) {
        delete[] m_verts;
        m_verts = nullptr;
    }
    if (m_indices != nullptr) {
        delete[] m_indices;
        m_indices = nullptr;
    }

    m_numPolygon = 0;
    m_numActivePolygon = 0;
    m_numVertex = 0;
    m_numActiveVertex = 0;
}

W3DVolumetricShadow::W3DVolumetricShadow() :
    m_next(nullptr),
    m_geometry(nullptr),
    m_robj(nullptr),
    m_shadowLengthScale(0.0f),
    m_boundsRadius(0.0f),
    m_optimalExtrusionPadding(0.0f)
{
    m_isEnabled = true;
    m_isInvisibleEnabled = false;

    for (int i = 0; i < MAX_SHADOW_CASTER_MESHES; i++) {
        m_numSilhouetteIndices[i] = 0;
        m_maxSilhouetteEntries[i] = 0;
        m_numIndicesPerMesh[i] = 0;

        m_silhouetteIndex[i] = nullptr;
        m_shadowVolumeCount[i] = 0;
    }

    for (int i = 0; i < 1; i++) {
        for (int j = 0; j < MAX_SHADOW_CASTER_MESHES; j++) {
            m_shadowVolume[i][j] = nullptr;
            m_shadowVolumeVB[i][j] = nullptr;
            m_shadowVolumeIB[i][j] = nullptr;

            m_shadowVolumeRenderTask[i][j].m_parentShadow = this;
            m_shadowVolumeRenderTask[i][j].m_meshIndex = j;
            m_shadowVolumeRenderTask[i][j].m_lightIndex = i;

            m_objectXformHistory[i][j].Make_Identity();

            m_lightPosHistory[i][j] = Vector3(0.0f, 0.0f, 0.0f);
        }
    }
}

W3DVolumetricShadow::~W3DVolumetricShadow()
{
    for (int i = 0; i < MAX_SHADOW_CASTER_MESHES; i++) {
        Delete_Silhouette(i);
    }

    for (int volume_index = 0; volume_index < 1; volume_index++) {
        for (int mesh_index = 0; mesh_index < MAX_SHADOW_CASTER_MESHES; mesh_index++) {
            if (m_shadowVolume[volume_index][mesh_index] != nullptr) {
                Geometry *geo = m_shadowVolume[volume_index][mesh_index];
                delete geo;
            }

            if (m_shadowVolumeVB[volume_index][mesh_index] != nullptr) {
                g_theW3DBufferManager->Release_Slot(m_shadowVolumeVB[volume_index][mesh_index]);
            }

            if (m_shadowVolumeIB[volume_index][mesh_index] != nullptr) {
                g_theW3DBufferManager->Release_Slot(m_shadowVolumeIB[volume_index][mesh_index]);
            }
        }
    }

    Ref_Ptr_Release(m_geometry);
    m_robj = nullptr;
}

void W3DVolumetricShadow::Set_Geometry(W3DShadowGeometry *geometry)
{
    unsigned short num_verticies = 0;
    unsigned short new_num_verticies = 0;

    for (int i = 0; i < MAX_SHADOW_CASTER_MESHES; i++) {
        if (m_geometry != nullptr) {
            W3DShadowGeometryMesh *mesh = m_geometry->Get_Mesh(i);
            num_verticies = mesh->Get_Num_Vertex();
        }

        if (geometry != nullptr) {
            W3DShadowGeometryMesh *mesh = geometry->Get_Mesh(i);
            new_num_verticies = mesh->Get_Num_Vertex();
        }

        if (new_num_verticies > num_verticies) {
            Delete_Silhouette(i);
            if (!Allocate_Silhouette(i, new_num_verticies)) {
                return;
            }
        }
    }

    m_geometry = geometry;
}

void W3DVolumetricShadow::Add_Silhouette_Edge(int mesh_index, PolyNeighbor *poly_neighbor, PolyNeighbor *hidden)
{
    int edge = 0;
    W3DShadowGeometryMesh *geo_mesh = m_geometry->Get_Mesh(mesh_index);

    for (int i = 0; i < 3; i++) {
        if (poly_neighbor->neighbor[i].neighborIndex == hidden->myIndex) {
            edge = i;
            break;
        }
    }

    short start = 0;
    short end = 0;
    short index_list[4];

    geo_mesh->Get_Polygon_Index(poly_neighbor->myIndex, index_list);

    if (index_list[0] != poly_neighbor->neighbor[edge].neighborEdgeIndex[0]
        && index_list[0] != poly_neighbor->neighbor[edge].neighborEdgeIndex[1]) {
        start = index_list[1];
        end = index_list[2];
    } else if (index_list[1] != poly_neighbor->neighbor[edge].neighborEdgeIndex[0]
        && index_list[1] != poly_neighbor->neighbor[edge].neighborEdgeIndex[1]) {
        start = index_list[2];
        end = index_list[0];
    } else {
        start = index_list[0];
        end = index_list[1];
    }

    Add_Silhouette_Indices(mesh_index, start, end);
}

void W3DVolumetricShadow::Add_Neighborless_Edges(int mesh_index, PolyNeighbor *poly_neighbor)
{
    W3DShadowGeometryMesh *geo_mesh = m_geometry->Get_Mesh(mesh_index);

    // BUGFIX Original didn't clear these
    short start = 0;
    short end = 0;
    short index_list[4];

    geo_mesh->Get_Polygon_Index(poly_neighbor->myIndex, index_list);

    for (int i = 0; i < 3; i++) {
        start = index_list[i];

        if (i == 2) {
            end = index_list[0];
        } else {
            end = index_list[i + 1];
        }

        bool add = true;

        for (int j = 0; j < 3; j++) {
            if (poly_neighbor->neighbor[j].neighborIndex != -1
                && ((poly_neighbor->neighbor[j].neighborEdgeIndex[0] == start
                        && poly_neighbor->neighbor[j].neighborEdgeIndex[1] == end)
                    || (poly_neighbor->neighbor[j].neighborEdgeIndex[1] == start
                        && poly_neighbor->neighbor[j].neighborEdgeIndex[0] == end))) {
                add = false;
                break;
            }
        }

        if (add) {
            Add_Silhouette_Indices(mesh_index, start, end);
        }
    }
}

void W3DVolumetricShadow::Add_Silhouette_Indices(int index, short start, short end)
{
    m_silhouetteIndex[index][m_numSilhouetteIndices[index]++] = start;
    m_silhouetteIndex[index][m_numSilhouetteIndices[index]++] = end;
}

bool W3DVolumetricShadow::Allocate_Shadow_Volume(int volume_index, int mesh_index)
{
    if (volume_index < 0 || volume_index >= 1) {
        return false;
    }

    Geometry *geo = m_shadowVolume[volume_index][mesh_index];

    if (geo == nullptr) {
        geo = new Geometry;
        m_shadowVolumeCount[mesh_index]++;
    }

    if (geo == nullptr) {
        m_shadowVolumeCount[mesh_index]--;
        return false;
    }

    m_shadowVolume[volume_index][mesh_index] = geo;

    int num = m_maxSilhouetteEntries[mesh_index];

    if (geo->Get_Flags() & 1 && !geo->Create(2 * num, num)) {
        delete geo;
        return false;
    }

    return true;
}

void W3DVolumetricShadow::Delete_Shadow_Volume(int volume_index)
{
    if (volume_index >= 0 && volume_index < 1) {
        for (int mesh_index = 0; mesh_index < MAX_SHADOW_CASTER_MESHES; mesh_index++) {
            if (m_shadowVolume[volume_index][mesh_index] != nullptr) {
                delete m_shadowVolume[volume_index][mesh_index];
                m_shadowVolume[volume_index][mesh_index] = nullptr;
                m_shadowVolumeCount[mesh_index]--;
            }
        }
    }
}

void W3DVolumetricShadow::Reset_Shadow_Volume(int volume_index, int mesh_index)
{
    if (volume_index >= 0 && volume_index < 1) {
        Geometry *geo = m_shadowVolume[volume_index][mesh_index];

        if (geo != nullptr) {
            if (m_shadowVolumeVB[volume_index][mesh_index] != nullptr) {
                g_theW3DBufferManager->Release_Slot(m_shadowVolumeVB[volume_index][mesh_index]);
                m_shadowVolumeVB[volume_index][mesh_index] = nullptr;
            }

            if (m_shadowVolumeIB[volume_index][mesh_index] != nullptr) {
                g_theW3DBufferManager->Release_Slot(m_shadowVolumeIB[volume_index][mesh_index]);
                m_shadowVolumeIB[volume_index][mesh_index] = nullptr;
            }

            geo->Release();
        }
    }
}

bool W3DVolumetricShadow::Allocate_Silhouette(int mesh_index, int num_vertices)
{
    m_silhouetteIndex[mesh_index] = new short[5 * num_vertices];

    if (m_silhouetteIndex[mesh_index] == nullptr) {
        return false;
    }

    m_numSilhouetteIndices[mesh_index] = 0;
    m_maxSilhouetteEntries[mesh_index] = 5 * num_vertices;
    return true;
}

void W3DVolumetricShadow::Delete_Silhouette(int mesh_index)
{
    if (m_silhouetteIndex[mesh_index] != nullptr) {
        delete[] m_silhouetteIndex[mesh_index];
        m_silhouetteIndex[mesh_index] = nullptr;
    }

    m_numSilhouetteIndices[mesh_index] = 0;
}

void W3DVolumetricShadow::Reset_Silhouette(int mesh_index)
{
    m_numSilhouetteIndices[mesh_index] = 0;
}

void W3DVolumetricShadow::Release()
{
    g_theW3DVolumetricShadowManager->Remove_Shadow(this);
}

void W3DVolumetricShadow::Update()
{
    static Vector3 originCompareVector(0.0f, 0.0f, 0.0f);
    Vector3 pos;

    if (m_geometry != nullptr) {
        pos = m_robj->Get_Position();

        if (pos != originCompareVector) {
            float height;

            if (g_theTerrainLogic != nullptr) {
                height = g_theTerrainLogic->Get_Ground_Height(pos.X, pos.Y, nullptr);
            } else {
                height = g_theTerrainRenderObject->Get_Height_Map_Height(pos.X, pos.Y, nullptr);
            }

            if (GameMath::Fabs(pos.Z - height) >= 2.0f) {
                float radius = 1.5f * m_boundsRadius;
                float dist_x = pos.X - s_bcX;

                if (s_beX + radius < GameMath::Fabs(dist_x)) {
                    return;
                }

                float dist_y = pos.Y - s_bcY;

                if (s_beY + radius < GameMath::Fabs(dist_y)) {
                    return;
                }

                float dist_z = pos.Z - s_bcZ;

                if (s_beZ + radius < GameMath::Fabs(dist_z)) {
                    return;
                }

                Update_Volumes(GameMath::Fabs(pos.Z - g_theTerrainRenderObject->Get_Min_Height()) + 0.1000000014901161f);
                return;
            }

            if (s_beX + m_boundsRadius >= GameMath::Fabs(pos.X - s_bcX)) {
                if (s_beY + m_boundsRadius >= GameMath::Fabs(pos.Y - s_bcY)) {
                    if (s_beZ + m_boundsRadius >= GameMath::Fabs(pos.Z - s_bcZ)) {
                        if (m_optimalExtrusionPadding == 0.0f) {
                            Update_Optimal_Extrusion_Padding();
                        }

                        Update_Volumes(m_optimalExtrusionPadding);
                        return;
                    }
                }
            }
        }
    }
}

void W3DVolumetricShadow::Update_Volumes(float zoffset)
{
    HLodClass *hlod = nullptr;

    if (m_robj != nullptr) {
        hlod = static_cast<HLodClass *>(m_robj);
    }

    static AABoxClass aaBox;
    static SphereClass sphere;
    captainslog_dbgassert(hlod != nullptr, "Update_Volumes : hlod is NULL!");
    bool is_visible = m_robj->Is_Really_Visible() != 0;
    int light_index = 0;
    RenderObjClass *robj = nullptr;
    int mesh_index;

label1:
    if (light_index < 1) {
        for (mesh_index = 0;; mesh_index++) {
            if (mesh_index >= m_geometry->Get_Mesh_Count()) {
                light_index++;
                goto label1;
            }

            int model_index;
            model_index = m_geometry->Get_Mesh(mesh_index)->m_modelIndex;

            if (model_index < 0) {
                if (m_robj != nullptr) {
                    robj = m_robj;
                }
            } else {
                RenderObjClass *lod = hlod->Peek_Lod_Model(0, model_index);

                if (lod != nullptr) {
                    robj = lod;
                }
            }

            if (robj != nullptr) {
                if (robj->Is_Not_Hidden_At_All() != 0) {
                    Update_Mesh_Volume(mesh_index,
                        light_index,
                        &robj->Get_Transform(),
                        robj->Get_Bounding_Box(),
                        m_robj->Get_Position().Z - zoffset);

                    if (m_shadowVolume[light_index][mesh_index] != nullptr) {
                        break;
                    }
                }
            }
        label2:;
        }

        if (m_shadowVolume[light_index][mesh_index]->Get_Visible_State() == Geometry::STATE_UNKNOWN) {
            if (is_visible) {
            label3:
                m_shadowVolume[light_index][mesh_index]->Set_Visible_State(Geometry::STATE_VISIBLE);
                goto label4;
            }

            sphere = m_shadowVolume[light_index][mesh_index]->Get_Bounding_Sphere();
            sphere.Center += robj->Get_Transform().Get_Translation();

            Geometry::VisibleState state;
            state = static_cast<Geometry::VisibleState>(CollisionMath::Overlap_Test(*g_shadowCameraFrustum, sphere));

            if (state == Geometry::STATE_UNKNOWN) {
                aaBox = m_shadowVolume[light_index][mesh_index]->Get_Bounding_Box();
                aaBox.Translate(robj->Get_Transform().Get_Translation());

                if (CollisionMath::Overlap_Test(*g_shadowCameraFrustum, aaBox) != CollisionMath::OUTSIDE) {
                    goto label3;
                }

                m_shadowVolume[light_index][mesh_index]->Set_Visible_State(Geometry::STATE_INVISIBLE);
            } else {
                m_shadowVolume[light_index][mesh_index]->Set_Visible_State(state);
            }
        }
    label4:
        if (m_shadowVolume[light_index][mesh_index]->Get_Visible_State() == Geometry::STATE_VISIBLE) {
            W3DBufferManager::W3DVertexBufferSlot *vertexbuffer = m_shadowVolumeVB[light_index][mesh_index];

            if (vertexbuffer != nullptr) {
                W3DBufferManager::W3DRenderTask *task = vertexbuffer->m_VB->m_renderTaskList;
                vertexbuffer->m_VB->m_renderTaskList = &m_shadowVolumeRenderTask[light_index][mesh_index];
                vertexbuffer->m_VB->m_renderTaskList->m_nextTask = task;
            } else {
                g_theW3DVolumetricShadowManager->Add_Dynamic_Shadow_Task(&m_shadowVolumeRenderTask[light_index][mesh_index]);
            }
        }
        goto label2;
    }
}

void W3DVolumetricShadow::Update_Mesh_Volume(
    int mesh_index, int light_index, const Matrix3D *mesh_xform, const AABoxClass &mesh_box, float floor_z)
{
#ifdef BUILD_WITH_D3D8
    Vector3 light_pos_object;
    Matrix4 world_to_object;
    Vector3 object_center;
    Vector3 to_light;
    Vector3 light_ray;
    Vector3 light_pos_world;
    bool is_mesh_rotating = false;
    bool is_light_moving = false;
    Matrix4 object_to_world(*mesh_xform);

    if (GameMath::Fabs(reinterpret_cast<Vector3 &>(m_objectXformHistory[light_index][mesh_index][0])
            * reinterpret_cast<Vector3 &>(object_to_world[0]))
        >= s_cosAngleToCare) {
        if (GameMath::Fabs(reinterpret_cast<Vector3 &>(m_objectXformHistory[light_index][mesh_index][1])
                * reinterpret_cast<Vector3 &>(object_to_world[1]))
            >= s_cosAngleToCare) {
            if (GameMath::Fabs(reinterpret_cast<Vector3 &>(m_objectXformHistory[light_index][mesh_index][2])
                    * reinterpret_cast<Vector3 &>(object_to_world[2]))
                < s_cosAngleToCare) {
                is_mesh_rotating = true;
            }
        } else {
            is_mesh_rotating = true;
        }
    } else {
        is_mesh_rotating = true;
    }

    light_pos_world = g_theW3DShadowManager->Get_Light_Pos_World(light_index);
    object_center = mesh_xform->Get_Translation();

    if (m_shadowLengthScale != 0.0f) {
        float dist_xy = GameMath::Sqrt(light_pos_world.X * light_pos_world.X + light_pos_world.Y * light_pos_world.Y)
            * m_shadowLengthScale;

        if (dist_xy > light_pos_world.Z) {
            light_pos_world.Z = dist_xy;
        }
    }

    if (light_pos_world != m_lightPosHistory[light_index][mesh_index]) {
        to_light = object_center - light_pos_world;
        to_light.Normalize();
        light_ray = object_center - m_lightPosHistory[light_index][mesh_index];
        light_ray.Normalize();

        if (GameMath::Fabs(to_light * light_ray < s_cosAngleToCare)) {
            is_light_moving = true;
        }
    } else {
        if (GameMath::Fabs(object_center.Z - m_objectXformHistory[light_index][mesh_index][2].W) > 0.1000000014901161f) {
            is_light_moving = true;
        }
    }

    if (is_light_moving || is_mesh_rotating) {
        float det;
        D3DXMatrixInverse(
            reinterpret_cast<D3DXMATRIX *>(&world_to_object), &det, reinterpret_cast<D3DXMATRIX *>(&object_to_world));
        Matrix4::Transform_Vector(world_to_object, light_pos_world, &light_pos_object);
        AABoxClass box(mesh_box);
        SphereClass sphere;
        Vector3 corners[8];
        Vector3 v1;

        corners[0] = box.m_center + box.m_extent;
        corners[1] = corners[0];
        corners[1].X -= 2.0f * box.m_extent.X;
        corners[2] = corners[1];
        corners[2].Y -= 2.0f * box.m_extent.Y;
        corners[3] = corners[2];
        corners[3].X += 2.0f * box.m_extent.X;

        v1 = corners[0] - light_pos_world;
        float f2 = 1.0f / v1.Length();
        v1 *= f2;
        float f3 = GameMath::Fabs((corners[0].Z - floor_z) / v1.Z);
        float shadow_extrude_distance = f3;
        corners[4] = corners[0] + (v1 * f3);

        shadow_extrude_distance = shadow_extrude_distance * f2;
        v1 = corners[1] - light_pos_world;
        f2 = 1.0f / v1.Length();
        v1 *= f2;
        float f5 = GameMath::Fabs((corners[1].Z - floor_z) / v1.Z);
        corners[5] = corners[1] + (v1 * f5);
        f5 = f5 * f2;

        if (f5 > shadow_extrude_distance) {
            shadow_extrude_distance = f5;
        }

        v1 = corners[2] - light_pos_world;
        f2 = 1.0f / v1.Length();
        v1 *= f2;
        f3 = GameMath::Fabs((corners[2].Z - floor_z) / v1.Z);
        corners[6] = corners[2] + v1 * f3;
        f3 = f3 * f2;

        if (f3 > shadow_extrude_distance) {
            shadow_extrude_distance = f3;
        }

        v1 = corners[3] - light_pos_world;
        f2 = 1.0f / v1.Length();
        v1 *= f2;
        f5 = GameMath::Fabs((corners[3].Z - floor_z) / v1.Z);
        corners[7] = corners[3] + (v1 * f5);
        f5 = f5 * f2;

        if (f5 > shadow_extrude_distance) {
            shadow_extrude_distance = f5;
        }

        box.Init(corners, 8);
        sphere.Init(box.m_center, box.m_extent.Length());

        CollisionMath::OverlapType res = CollisionMath::Overlap_Test(*g_shadowCameraFrustum, sphere);

        if (res == CollisionMath::OVERLAPPED) {
            res = CollisionMath::Overlap_Test(*g_shadowCameraFrustum, box);
        }

        if (res == CollisionMath::OUTSIDE) {
            if (m_shadowVolume[light_index][mesh_index] != nullptr) {
                box.Translate(-object_center);
                m_shadowVolume[light_index][mesh_index]->Set_Bounding_Box(box);
                sphere.Center -= object_center;
                m_shadowVolume[light_index][mesh_index]->Set_Bounding_Sphere(sphere);
                m_shadowVolume[light_index][mesh_index]->Set_Visible_State(Geometry::STATE_INVISIBLE);
            }
        } else {
            if (m_numSilhouetteIndices[mesh_index] != 0) {
                m_geometry->Get_Mesh(mesh_index)->Build_Polygon_Normals();
            }

            Reset_Silhouette(mesh_index);
            Build_Silhouette(mesh_index, &light_pos_object);

            if (m_shadowVolume[light_index][mesh_index] == nullptr) {
                Allocate_Shadow_Volume(light_index, mesh_index);
            }

            if (m_shadowVolumeVB[light_index][mesh_index] && (is_mesh_rotating || is_light_moving)) {
                if (is_mesh_rotating) {
                    m_shadowVolume[light_index][mesh_index]->Set_Flags(
                        m_shadowVolume[light_index][mesh_index]->Get_Flags() | 1);
                }

                Reset_Shadow_Volume(light_index, mesh_index);
                Allocate_Shadow_Volume(light_index, mesh_index);
            }

            if ((m_shadowVolume[light_index][mesh_index]->Get_Flags() & 1) != 0) {
                Construct_Volume(&light_pos_object, shadow_extrude_distance, light_index, mesh_index);
            } else {
                Construct_Volume_VB(&light_pos_object, shadow_extrude_distance, light_index, mesh_index);
            }

            m_objectXformHistory[light_index][mesh_index] = object_to_world;
            m_lightPosHistory[light_index][mesh_index] = light_pos_world;
            box.Translate(-object_center);
            m_shadowVolume[light_index][mesh_index]->Set_Bounding_Box(box);
            sphere.Center -= object_center;
            m_shadowVolume[light_index][mesh_index]->Set_Bounding_Sphere(sphere);
            m_shadowVolume[light_index][mesh_index]->Set_Visible_State(Geometry::STATE_VISIBLE);
        }
    } else if (m_shadowVolume[light_index][mesh_index] != nullptr) {
        m_shadowVolume[light_index][mesh_index]->Set_Visible_State(Geometry::STATE_UNKNOWN);
    }
#endif
}

void W3DVolumetricShadow::Build_Silhouette(int mesh_index, Vector3 *light_pos_object)
{
    Vector3 v;
    int mesh_edge_start = 0;
    W3DShadowGeometryMesh *mesh = m_geometry->Get_Mesh(mesh_index);
    mesh_edge_start = m_numSilhouetteIndices[mesh_index];
    int num_polys = mesh->Get_Num_Polygon();

    for (int poly_index = 0; poly_index < num_polys; poly_index++) {
        PolyNeighbor *us = mesh->Get_Poly_Neighbor(poly_index);
        us->status = 0;
        Vector3 *normal = mesh->Get_Polygon_Normal(poly_index);

        short poly[3];
        mesh->Get_Polygon_Index(poly_index, poly);
        Vector3 *vertex = mesh->Get_Vertex(poly[0]);
        v = *vertex - *light_pos_object;

        if (v * *normal < 0.0f) {
            us->status |= 1;
        }
    }

    for (int poly_index = 0; poly_index < num_polys; poly_index++) {
        PolyNeighbor *us = mesh->Get_Poly_Neighbor(poly_index);
        bool visible_neighborless = false;

        for (int i = 0; i < 3; i++) {
            PolyNeighbor *neighbor = nullptr;

            if (us->neighbor[i].neighborIndex != -1) {
                neighbor = mesh->Get_Poly_Neighbor(us->neighbor[i].neighborIndex);

                if ((neighbor->status & 2) != 0) {
                    continue;
                }
            }

            if ((us->status & 1) != 0) {
                if (neighbor != nullptr) {
                    if ((neighbor->status & 1) == 0) {
                        Add_Silhouette_Edge(mesh_index, us, neighbor);
                    }
                } else {
                    visible_neighborless = true;
                }
            } else if (neighbor && (neighbor->status & 1) != 0) {
                Add_Silhouette_Edge(mesh_index, neighbor, us);
            }
        }

        if (visible_neighborless) {
            Add_Neighborless_Edges(mesh_index, us);
        }

        us->status |= 2u;
    }

    m_numIndicesPerMesh[mesh_index] = m_numSilhouetteIndices[mesh_index] - mesh_edge_start;
}

void W3DVolumetricShadow::Update_Optimal_Extrusion_Padding()
{
    if (m_robj != nullptr) {
        Vector3 light_pos = g_theW3DShadowManager->Get_Light_Pos_World(0);

        if (m_shadowLengthScale != 0.0f) {
            float dist_xy = GameMath::Sqrt(light_pos.X * light_pos.X + light_pos.Y * light_pos.Y) * m_shadowLengthScale;

            if (dist_xy > light_pos.Z) {
                light_pos.Z = dist_xy;
            }
        }

        Vector3 robj_pos = m_robj->Get_Position();
        Vector3 v3(robj_pos);
        float f2 = robj_pos.Z;
        AABoxClass box = m_robj->Get_Bounding_Box();
        Vector3 v4;
        Vector3 v5;
        LineSegClass line;
        CastResultStruct cast_res;
        Vector3 box_xy_points[4];
        RayCollisionTestClass ray(line, &cast_res);

        box_xy_points[0] = box.m_center + box.m_extent;
        box_xy_points[1] = box_xy_points[0];
        box_xy_points[1].X -= 2.0f * box.m_extent.X;
        box_xy_points[2] = box_xy_points[1];
        box_xy_points[2].Y -= 2.0f * box.m_extent.Y;
        box_xy_points[3] = box_xy_points[2];
        box_xy_points[3].X += 2.0f * box.m_extent.X;

        for (int i = 0; i < 4; i++) {
            v4 = box_xy_points[i] - light_pos;
            v4.Normalize();
            ray.m_ray.Set(box_xy_points[i], box_xy_points[i] + (v4 * 5120.0f));
            cast_res.Reset();

            if (g_theTerrainRenderObject->Cast_Ray(ray) && !ray.m_result->start_bad) {
                v5 = cast_res.contact_point - box_xy_points[i];
                v5.Z = 0.0f;
                float f3 = v5.Length();
                float f4 = f3 / 20.0f;
                int i1 = GameMath::Fast_To_Int_Ceil(f4);
                float f5 = 1.0f / i1;
                Vector3 v7;
                float f6 = f5;

                for (int j = 0; j < i1; j++) {
                    v7 = box_xy_points[i] + (v5 * f6);
                    v7.Z = 0.0f;
                    float height = g_theTerrainRenderObject->Get_Height_Map_Height(v7.X, v7.Y, nullptr);

                    if (robj_pos.Z - 5.0f > height) {
                        if (j != 0) {
                            Vector3 v8 = box_xy_points[i] - v3;
                            float f8 = GameMath::Asin(v8.Z / v8.Length());

                            if (f8 >= DEG_TO_RADF(90.f) || f8 <= 0.0f) {
                                f8 = DEG_TO_RADF(80.f);
                            }

                            float f9 = GameMath::Tan(f8);

                            if (f9 > m_shadowLengthScale) {
                                Set_Shadow_Length_Scale(f9);
                            }
                        } else {
                            f2 = height;
                            Set_Shadow_Length_Scale(GameMath::Tan(DEG_TO_RADF(80.f)));
                        }

                        break;
                    }

                    if (height < f2) {
                        f2 = height;
                        v3 = v7;
                        v3.Z = f2;
                    }

                    f6 = f6 + f5;
                }
            }
        }

        m_optimalExtrusionPadding = robj_pos.Z - f2 + 0.1f;
        captainslog_dbgassert(
            m_optimalExtrusionPadding <= 159.375f, "Warning: Volumetric Shadow Update_Optimal_Extrusion_Padding too large");
    }
}

void W3DVolumetricShadow::Construct_Volume(
    Vector3 *light_pos_object, float shadow_extrude_distance, int volume_index, int mesh_index)
{
    Vector3 v1;

    if (volume_index == 0) {
        if (light_pos_object != nullptr) {
            Geometry *geom = m_shadowVolume[0][mesh_index];

            if (geom != nullptr) {
                int vertex_count = 0;
                int polygon_count = 0;
                int indices_per_mesh = m_numIndicesPerMesh[mesh_index];

                if (indices_per_mesh != 0) {
                    W3DShadowGeometryMesh *geom_mesh = m_geometry->Get_Mesh(mesh_index);
                    geom->Set_Num_Active_Polygon(0);
                    geom->Set_Num_Active_Vertex(0);
                    short *silhouette_indices = m_silhouetteIndex[mesh_index];
                    short strip_start_index = silhouette_indices[0];
                    short s3 = 0;
                    Vector3 *vertex = geom_mesh->Get_Vertex(silhouette_indices[0]);
                    v1 = *vertex - *light_pos_object;
                    v1 *= shadow_extrude_distance;
                    v1 += *vertex;
                    geom->Set_Vertex(vertex_count, vertex);
                    geom->Set_Vertex(vertex_count + 1, &v1);
                    vertex_count = 2;
                    int i3 = 0;
                    int i4 = 1;

                    for (int i = 0; i < indices_per_mesh; i += 2) {
                        short current_edge_end = silhouette_indices[i + 1];
                        int j;

                        for (j = i + 2; j < indices_per_mesh; j += 2) {
                            if (silhouette_indices[j] == current_edge_end) {
                                int temp_index = *reinterpret_cast<int *>(&silhouette_indices[i + 2]);
                                *reinterpret_cast<int *>(&silhouette_indices[i + 2]) =
                                    *reinterpret_cast<int *>(&silhouette_indices[j]);
                                *reinterpret_cast<int *>(&silhouette_indices[j]) = temp_index;
                                break;
                            }
                        }

                        unsigned short index_list[3];

                        if (j < indices_per_mesh) {
                            Vector3 *edge_vertex_2 = geom_mesh->Get_Vertex(current_edge_end);
                            geom->Set_Vertex(vertex_count, edge_vertex_2);

                            index_list[0] = i3;
                            index_list[1] = i4;
                            index_list[2] = vertex_count;
                            geom->Set_Polygon_Index(polygon_count, index_list);

                            v1 = *edge_vertex_2 - *light_pos_object;
                            v1 *= shadow_extrude_distance;
                            v1 += *edge_vertex_2;
                            geom->Set_Vertex(vertex_count + 1, &v1);

                            index_list[0] = vertex_count;
                            index_list[1] = i4;
                            index_list[2] = vertex_count + 1;
                            geom->Set_Polygon_Index(polygon_count + 1, index_list);

                            i3 = vertex_count;
                            i4 = vertex_count + 1;
                            vertex_count += 2;
                            polygon_count += 2;
                        } else {
                            if (current_edge_end == strip_start_index) {
                                index_list[0] = i3;
                                index_list[1] = i4;
                                index_list[2] = s3;
                                geom->Set_Polygon_Index(polygon_count, index_list);

                                index_list[0] = s3;
                                index_list[1] = i4;
                                index_list[2] = s3 + 1;
                                geom->Set_Polygon_Index(polygon_count + 1, index_list);
                            } else {
                                Vector3 *vertex3 = geom_mesh->Get_Vertex(current_edge_end);
                                geom->Set_Vertex(vertex_count, vertex3);

                                index_list[0] = i3;
                                index_list[1] = i4;
                                index_list[2] = vertex_count;
                                geom->Set_Polygon_Index(polygon_count, index_list);

                                v1 = *vertex3 - *light_pos_object;
                                v1 *= shadow_extrude_distance;
                                v1 += *vertex3;
                                geom->Set_Vertex(vertex_count + 1, &v1);

                                index_list[0] = vertex_count;
                                index_list[1] = i4;
                                index_list[2] = vertex_count + 1;
                                geom->Set_Polygon_Index(polygon_count + 1, index_list);

                                i3 = vertex_count;
                                i4 = vertex_count + 1;
                                vertex_count += 2;
                            }

                            if (i + 2 >= indices_per_mesh) {
                                polygon_count += 2;
                                break;
                            }

                            Vector3 *vertex4 = geom_mesh->Get_Vertex(silhouette_indices[i + 2]);
                            v1 = *vertex4 - *light_pos_object;
                            v1 *= shadow_extrude_distance;
                            v1 += *vertex4;

                            i3 = vertex_count;
                            i4 = vertex_count + 1;
                            strip_start_index = silhouette_indices[i + 2];
                            s3 = vertex_count;
                            geom->Set_Vertex(vertex_count, vertex4);
                            geom->Set_Vertex(i4, &v1);
                            vertex_count += 2;
                            polygon_count += 2;
                        }
                    }

                    geom->Set_Num_Active_Polygon(polygon_count);
                    geom->Set_Num_Active_Vertex(vertex_count);
                }
            }
        }
    }
}

void W3DVolumetricShadow::Render_Volume(int mesh_index, int light_index)
{
    HLodClass *hlod = nullptr;

    if (m_robj != nullptr) {
        hlod = static_cast<HLodClass *>(m_robj);
    }

    int model_index = m_geometry->Get_Mesh(mesh_index)->m_modelIndex;
    RenderObjClass *robj = nullptr;

    if (model_index < 0) {
        if (m_robj != nullptr) {
            robj = m_robj;
        }
    } else {
        RenderObjClass *lod = hlod->Peek_Lod_Model(0, model_index);

        if (lod != nullptr) {
            robj = lod;
        }
    }

    if (robj != nullptr) {
        if ((m_shadowVolume[0][mesh_index]->Get_Flags() & 1) != 0) {
            Render_Dynamic_Mesh_Volume(mesh_index, light_index, &robj->Get_Transform());
        } else {
            Render_Mesh_Volume(mesh_index, light_index, &robj->Get_Transform());
        }
    }
}

void W3DVolumetricShadow::Render_Mesh_Volume(int mesh_index, int light_index, const Matrix3D *mesh_xform)
{
#ifdef BUILD_WITH_D3D8
    IDirect3DDevice8 *device = DX8Wrapper::Get_D3D_Device8();

    if (device != nullptr) {
        Geometry *geom = m_shadowVolume[light_index][mesh_index];
        int vertex_count = geom->Get_Num_Active_Vertex();
        int polygon_count = geom->Get_Num_Active_Polygon();
        int index_count = 3 * polygon_count;

        if (vertex_count != 0) {
            if (polygon_count != 0) {
                Matrix4 tm(*mesh_xform);
                tm = tm.Transpose();
                device->SetTransform(D3DTS_WORLD, reinterpret_cast<D3DMATRIX *>(&tm));
                W3DBufferManager::W3DVertexBufferSlot *vb = m_shadowVolumeVB[light_index][mesh_index];

                if (vb != nullptr) {
                    if (vb->m_VB->m_DX8VertexBuffer->Get_DX8_Vertex_Buffer() != s_lastActiveVertexBuffer) {
                        s_lastActiveVertexBuffer = vb->m_VB->m_DX8VertexBuffer->Get_DX8_Vertex_Buffer();
                        device->SetStreamSource(
                            0, s_lastActiveVertexBuffer, vb->m_VB->m_DX8VertexBuffer->FVF_Info().Get_FVF_Size());
                    }

                    captainslog_dbgassert(vb->m_size >= vertex_count, "Overflowing Shadow Vertex Buffer Slot");
                    W3DBufferManager::W3DIndexBufferSlot *ib = m_shadowVolumeIB[light_index][mesh_index];

                    if (ib != nullptr) {
                        captainslog_dbgassert(ib->m_size >= index_count, "Overflowing Shadow Index Buffer Slot");
                        device->SetIndices(ib->m_IB->m_DX8IndexBuffer->Get_DX8_Index_Buffer(), vb->m_start);

                        if (DX8Wrapper::Is_Triangle_Draw_Enabled()) {
                            device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, vertex_count, ib->m_start, polygon_count);
                        }
                    }
                }
            }
        }
    }
#endif
}

#ifdef GAME_DEBUG_STRUCTS
void W3DVolumetricShadow::Gather_Draw_Stats(DebugDrawStats *stats)
{
    int draw_calls = 0;
    if (m_geometry != nullptr && m_isEnabled && !m_isInvisibleEnabled && g_theWriteableGlobalData->m_shadowVolumes) {
        HLodClass *hlod = nullptr;

        if (m_robj != nullptr) {
            hlod = reinterpret_cast<HLodClass *>(m_robj);
        }

        for (int i = 0; i < 1; i++) {
            for (int j = 0; j < m_geometry->Get_Mesh_Count(); j++) {
                int model_index = m_geometry->Get_Mesh(j)->m_modelIndex;
                RenderObjClass *robj = nullptr;

                if (model_index < 0) {
                    if (m_robj != nullptr) {
                        robj = m_robj;
                    }
                } else {
                    RenderObjClass *lod = hlod->Peek_Lod_Model(0, model_index);

                    if (lod != nullptr) {
                        robj = lod;
                    }
                }

                if (robj != nullptr) {
                    if (robj->Is_Not_Hidden_At_All()) {
                        draw_calls++;
                    }
                }
            }
        }
    }

    stats->Add_Extra_Draw_Calls(2 * draw_calls);
}
#endif

void W3DVolumetricShadow::Render_Dynamic_Mesh_Volume(int mesh_index, int light_index, const Matrix3D *mesh_xform)
{
#ifdef BUILD_WITH_D3D8
    struct SHADOW_STATIC_VOLUME_VERTEX
    {
        float x;
        float y;
        float z;
    };

    IDirect3DDevice8 *device = DX8Wrapper::Get_D3D_Device8();

    if (device != nullptr) {
        Geometry *geom = m_shadowVolume[light_index][mesh_index];
        int vertex_count = geom->Get_Num_Active_Vertex();
        int polygon_count = geom->Get_Num_Active_Polygon();
        int index_count = 3 * polygon_count;

        if (vertex_count != 0) {
            if (polygon_count != 0) {
                SHADOW_STATIC_VOLUME_VERTEX *vertices;

                if (g_nShadowVertsInBuf <= SHADOW_VERTEX_SIZE - vertex_count) {
                    if (FAILED(g_shadowVertexBufferD3D->Lock(12 * g_nShadowVertsInBuf,
                            12 * vertex_count,
                            reinterpret_cast<BYTE **>(&vertices),
                            D3DLOCK_NOOVERWRITE))) {
                        return;
                    }
                } else {
                    if (FAILED(g_shadowVertexBufferD3D->Lock(
                            0, 12 * vertex_count, reinterpret_cast<BYTE **>(&vertices), D3DLOCK_DISCARD))) {
                        return;
                    }

                    g_nShadowVertsInBuf = 0;
                    g_nShadowStartBatchVertex = 0;
                }

                if (vertices != nullptr) {
                    memcpy(vertices, geom->Get_Vertex(0), sizeof(Vector3) * vertex_count);
                }

                g_shadowVertexBufferD3D->Unlock();
                unsigned short *indices;

                if (g_nShadowIndicesInBuf <= SHADOW_INDEX_SIZE - index_count) {
                    if (FAILED(g_shadowIndexBufferD3D->Lock(2 * g_nShadowIndicesInBuf,
                            2 * index_count,
                            reinterpret_cast<BYTE **>(&indices),
                            D3DLOCK_NOOVERWRITE))) {
                        return;
                    }
                } else {
                    if (FAILED(g_shadowIndexBufferD3D->Lock(
                            0, 2 * index_count, reinterpret_cast<BYTE **>(&indices), D3DLOCK_DISCARD))) {
                        return;
                    }

                    g_nShadowIndicesInBuf = 0;
                    g_nShadowStartBatchIndex = 0;
                }

                if (indices != nullptr) {
                    memcpy(indices, geom->Get_Polygon_Index(0, indices), 6 * polygon_count);
                }

                g_shadowIndexBufferD3D->Unlock();
                device->SetIndices(g_shadowIndexBufferD3D, g_nShadowStartBatchVertex);
                Matrix4 tm(*mesh_xform);
                tm = tm.Transpose();
                device->SetTransform(D3DTS_WORLD, reinterpret_cast<D3DMATRIX *>(&tm));

                if (g_shadowVertexBufferD3D != s_lastActiveVertexBuffer) {
                    device->SetStreamSource(0, g_shadowVertexBufferD3D, sizeof(SHADOW_STATIC_VOLUME_VERTEX));
                    s_lastActiveVertexBuffer = g_shadowVertexBufferD3D;
                }

                if (DX8Wrapper::Is_Triangle_Draw_Enabled()) {
                    device->DrawIndexedPrimitive(
                        D3DPT_TRIANGLELIST, 0, vertex_count, g_nShadowStartBatchIndex, polygon_count);
                }

                g_nShadowVertsInBuf += vertex_count;
                g_nShadowStartBatchVertex = g_nShadowVertsInBuf;
                g_nShadowIndicesInBuf += index_count;
                g_nShadowStartBatchIndex = g_nShadowIndicesInBuf;
            }
        }
    }
#endif
}

void W3DVolumetricShadow::Construct_Volume_VB(
    Vector3 *light_pos_object, float shadow_extrude_distance, int volume_index, int mesh_index)
{
    Vector3 v1;
    Vector3 v2;

    if (volume_index == 0) {
        if (light_pos_object != nullptr) {
            Geometry *shadow_volume = m_shadowVolume[0][mesh_index];

            if (shadow_volume != nullptr) {
                int index_range = 0;
                int polygon_count = 0;
                int indices_per_mesh = m_numIndicesPerMesh[mesh_index];

                if (indices_per_mesh != 0) {
                    short *silhouette_indices = m_silhouetteIndex[mesh_index];
                    short first_index = silhouette_indices[0];
                    index_range = 2;

                    for (int i = 0; i < indices_per_mesh; i += 2) {
                        short second_index = silhouette_indices[i + 1];
                        int j;

                        for (j = i + 2; j < indices_per_mesh; j += 2) {
                            if (silhouette_indices[j] == second_index) {
                                int temp_index = *reinterpret_cast<int *>(&silhouette_indices[i + 2]);
                                *reinterpret_cast<int *>(&silhouette_indices[i + 2]) =
                                    *reinterpret_cast<int *>(&silhouette_indices[j]);
                                *reinterpret_cast<int *>(&silhouette_indices[j]) = temp_index;
                                break;
                            }
                        }

                        if (j < indices_per_mesh) {
                            index_range += 2;
                            polygon_count += 2;
                        } else {
                            if (second_index != first_index) {
                                index_range += 2;
                            }

                            if (i + 2 >= indices_per_mesh) {
                                polygon_count += 2;
                                break;
                            }

                            first_index = silhouette_indices[i + 2];
                            index_range += 2;
                            polygon_count += 2;
                        }
                    }

                    captainslog_dbgassert(
                        m_shadowVolumeVB[0][mesh_index] == nullptr, "Updating Existing Static Vertex Buffer Shadow");
                    m_shadowVolumeVB[0][mesh_index] =
                        g_theW3DBufferManager->Get_Slot(W3DBufferManager::VBM_FVF_XYZ, index_range);
                    W3DBufferManager::W3DVertexBufferSlot *shadow_vb = m_shadowVolumeVB[0][mesh_index];
                    captainslog_dbgassert(shadow_vb != nullptr, "Can't allocate vertex buffer slot for shadow volume");
                    captainslog_dbgassert(shadow_vb->m_size >= index_range, "Overflowing Shadow Vertex Buffer Slot");
                    captainslog_dbgassert(
                        m_shadowVolume[0][mesh_index]->Get_Num_Polygon() == 0, "Updating Existing Static Shadow Volume");

                    captainslog_dbgassert(
                        m_shadowVolumeIB[0][mesh_index] == nullptr, "Updating Existing Static Index Buffer Shadow");
                    m_shadowVolumeIB[0][mesh_index] = g_theW3DBufferManager->Get_Slot(3 * polygon_count);
                    W3DBufferManager::W3DIndexBufferSlot *shadow_ib = m_shadowVolumeIB[0][mesh_index];
                    captainslog_dbgassert(shadow_ib != nullptr, "Can't allocate index buffer slot for shadow volume");
                    captainslog_dbgassert(shadow_ib->m_size >= 3 * polygon_count, "Overflowing Shadow Index Buffer Slot");

                    if (shadow_ib != nullptr && shadow_vb != nullptr) {
                        W3DShadowGeometryMesh *geom_mesh = m_geometry->Get_Mesh(mesh_index);
                        VertexBufferClass::AppendLockClass lock_vtx_buffer(
                            shadow_vb->m_VB->m_DX8VertexBuffer, shadow_vb->m_start, index_range);
                        Vector3 *vertices = static_cast<Vector3 *>(lock_vtx_buffer.Get_Vertex_Array());

                        if (vertices != nullptr) {
                            IndexBufferClass::AppendLockClass lock_idx_buffer(
                                shadow_ib->m_IB->m_DX8IndexBuffer, shadow_ib->m_start, 3 * polygon_count);
                            unsigned short *indices = lock_idx_buffer.Get_Index_Array();

                            if (indices != nullptr) {
                                shadow_volume->Set_Num_Active_Polygon(polygon_count);
                                shadow_volume->Set_Num_Active_Vertex(index_range);
                                short *silhouette_indices_2 = m_silhouetteIndex[mesh_index];
                                short first_index_2 = silhouette_indices_2[0];
                                short s5 = 0;
                                Vector3 *vertex4 = geom_mesh->Get_Vertex(silhouette_indices_2[0]);
                                v1 = *vertex4 - *light_pos_object;
                                v1 *= shadow_extrude_distance;
                                v1 += *vertex4;
                                *vertices++ = *vertex4;
                                *vertices++ = v1;
                                index_range = 2;
                                polygon_count = 0;
                                int i1 = 0;
                                int i3 = 1;

                                for (int i = 0; i < indices_per_mesh; i += 2) {
                                    short second_index_2 = silhouette_indices_2[i + 1];

                                    if (i + 2 < indices_per_mesh && silhouette_indices_2[i + 2] == second_index_2) {
                                        Vector3 *vertex1 = geom_mesh->Get_Vertex(second_index_2);
                                        *vertices++ = *vertex1;
                                        indices[0] = i1;
                                        indices[1] = i3;
                                        indices[4] = i3;
                                        indices[2] = index_range;
                                        indices[3] = index_range;
                                        indices[5] = index_range + 1;
                                        indices += 6;
                                        v1 = *vertex1 - *light_pos_object;
                                        v1 *= shadow_extrude_distance;
                                        v1 += *vertex1;
                                        *vertices++ = v1;
                                        i1 = index_range;
                                        i3 = index_range + 1;
                                        index_range += 2;
                                        polygon_count += 2;
                                    } else {
                                        if (second_index_2 == first_index_2) {
                                            indices[0] = i1;
                                            indices[1] = i3;
                                            indices[4] = i3;
                                            indices[2] = s5;
                                            indices[3] = s5;
                                            indices[5] = s5 + 1;
                                            indices += 6;
                                        } else {
                                            Vector3 *vertex2 = geom_mesh->Get_Vertex(second_index_2);
                                            *vertices++ = *vertex2;
                                            indices[0] = i1;
                                            indices[1] = i3;
                                            indices[4] = i3;
                                            indices[2] = index_range;
                                            indices[3] = index_range;
                                            indices[5] = index_range + 1;
                                            indices += 6;
                                            v1 = *vertex2 - *light_pos_object;
                                            v1 *= shadow_extrude_distance;
                                            v1 += *vertex2;
                                            *vertices++ = v1;
                                            i1 = index_range;
                                            i3 = index_range + 1;
                                            index_range += 2;
                                        }

                                        if (i + 2 >= indices_per_mesh) {
                                            polygon_count += 2;
                                            break;
                                        }

                                        Vector3 *vertex3 = geom_mesh->Get_Vertex(silhouette_indices_2[i + 2]);
                                        v1 = *vertex3 - *light_pos_object;
                                        v1 *= shadow_extrude_distance;
                                        v1 += *vertex3;
                                        i1 = index_range;
                                        i3 = index_range + 1;
                                        first_index_2 = silhouette_indices_2[i + 2];
                                        s5 = index_range;
                                        *vertices++ = *vertex3;
                                        *vertices++ = v1;
                                        index_range += 2;
                                        polygon_count += 2;
                                    }
                                }
                            }
                        }
                    } else {
                        if (shadow_ib != nullptr) {
                            g_theW3DBufferManager->Release_Slot(shadow_ib);
                        }

                        if (shadow_vb != nullptr) {
                            g_theW3DBufferManager->Release_Slot(shadow_vb);
                        }

                        m_shadowVolumeIB[0][mesh_index] = 0;
                        m_shadowVolumeVB[0][mesh_index] = 0;
                    }
                }
            }
        }
    }
}
