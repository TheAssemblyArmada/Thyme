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
#include "hlod.h"
#include "mesh.h"
#include "meshmdl.h"
#include "missing.h"
#include <cmath>
#include <new>

class MeshModelClass;

W3DShadowGeometryMesh::W3DShadowGeometryMesh() :
    m_polyNeighbors(nullptr),
    m_numPolyNeighbors(0),
    m_parentVerts(nullptr),
    m_polygonNormals(nullptr),
    // BUGFIX Init all members
    m_mesh(nullptr),
    m_modelIndex(0),
    m_verts(nullptr),
    m_parentGeometry(nullptr),
    m_numVerts(0),
    m_numPolygons(0),
    m_polygons(nullptr)
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

        m_polyNeighbors[i].myIndex = (short)i;

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

                                // TODO what is this
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
                            m_polyNeighbors[i].neighbor[edge].neighborIndex = (short)j;
                            m_polyNeighbors[i].neighbor[edge].neighborEdgeIndex[0] = (short)index1;
                            m_polyNeighbors[i].neighbor[edge].neighborEdgeIndex[1] = (short)index2;
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
    // TODO deduplicate Init_From_HLOD Init_From_Mesh code

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

                // TODO Look into. Is this why infantry can't have shadows?
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
    // TODO deduplicate Init_From_HLOD Init_From_Mesh code

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
