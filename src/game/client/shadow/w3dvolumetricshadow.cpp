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
#include "drawable.h"
#include "drawmodule.h"
#include "globaldata.h"
#include "hlod.h"
#include "kindof.h"
#include "mesh.h"
#include "meshmdl.h"
#include "missing.h"
#include "view.h"
#include "w3dbuffermanager.h"
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
            Build_Polygon_Normal(i, &normals[i]);
        }

        m_polygonNormals = normals;
    }
}

void W3DShadowGeometryMesh::Build_Polygon_Normal(int index, Vector3 *normal) const
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

Geometry::Geometry() :
    m_verts(nullptr),
    m_indices(nullptr),
    m_numPolygon(0),
    m_numVertex(0),
    m_flags(0),
    // BUGFIX original didn't init these
    m_numActivePolygon(0),
    m_numActiveVertex(0),
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

        m_indices = new short[3 * num_poly];

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

W3DVolumetricShadow::W3DVolumetricShadow()
{
    m_next = nullptr;
    m_geometry = nullptr;

    m_shadowLengthScale = 0.0;
    m_optimalExtrusionPadding = 0.0;
    m_robj = nullptr;

    m_isEnabled = true;
    m_isInvisibleEnabled = false;

    for (int i = 0; i < MAX_SHADOW_CASTER_MESHES; ++i) {

        m_numSilhouetteIndices[i] = 0;
        m_maxSilhouetteEntries[i] = 0;

        m_silhouetteIndex[i] = nullptr;
        m_shadowVolumeCount[i] = 0;
    }

    for (int i = 0; i < 1; ++i) {

        for (int j = 0; j < MAX_SHADOW_CASTER_MESHES; ++j) {

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
    for (int i = 0; i < MAX_SHADOW_CASTER_MESHES; ++i) {
        Delete_Silhouette(i);
    }
    for (int volume_index = 0; volume_index < 1; ++volume_index) {
        for (int mesh_index = 0; mesh_index < MAX_SHADOW_CASTER_MESHES; ++mesh_index) {

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

void W3DVolumetricShadow::Set_Geometry(W3DShadowGeometry *geo)
{
    unsigned short num_verticies = 0;
    unsigned short new_num_verticies = 0;

    for (int i = 0; i < MAX_SHADOW_CASTER_MESHES; ++i) {
        if (m_geometry != nullptr) {
            W3DShadowGeometryMesh *mesh = m_geometry->Get_Mesh(i);
            num_verticies = mesh->Get_Num_Vertex();
        }

        if (geo != nullptr) {
            W3DShadowGeometryMesh *mesh = geo->Get_Mesh(i);
            new_num_verticies = mesh->Get_Num_Vertex();
        }

        if (new_num_verticies > num_verticies) {
            Delete_Silhouette(i);
            if (!Allocate_Silhouette(i, new_num_verticies)) {
                return;
            }
        }
    }

    m_geometry = geo;
}

void W3DVolumetricShadow::Add_Silhouette_Edge(int mesh_index, PolyNeighbor *visible, PolyNeighbor *hidden)
{
    int edge = 0;

    W3DShadowGeometryMesh *geo_mesh = m_geometry->Get_Mesh(mesh_index);

    captainslog_assert(visible != nullptr && hidden != nullptr);

    for (int i = 0; i < 3; ++i) {
        if (visible->neighbor[i].neighborIndex == hidden->myIndex) {
            edge = i;
            break;
        }
    }

    short start;
    short end;

    short index_list[4];

    geo_mesh->Get_Polygon_Index(visible->myIndex, index_list);

    if (index_list[0] != visible->neighbor[edge].neighborEdgeIndex[0]
        && index_list[0] != visible->neighbor[edge].neighborEdgeIndex[1]) {

        start = index_list[1];
        end = index_list[2];

    } else if (index_list[1] != visible->neighbor[edge].neighborEdgeIndex[0]
        && index_list[1] != visible->neighbor[edge].neighborEdgeIndex[1]) {

        start = index_list[2];
        end = index_list[0];

    } else {

        start = index_list[0];
        end = index_list[1];
    }

    Add_Silhouette_Indices(mesh_index, start, end);
}

void W3DVolumetricShadow::Add_Neighborless_Edges(int mesh_index, PolyNeighbor *us)
{
    captainslog_assert(us != nullptr);

    W3DShadowGeometryMesh *geo_mesh = m_geometry->Get_Mesh(mesh_index);

    short start;
    short end;

    short index_list[4];

    geo_mesh->Get_Polygon_Index(us->myIndex, index_list);

    for (int i = 0; i < 3; ++i) {
        start = index_list[i];

        if (i == 2) {
            end = index_list[0];
        } else {
            end = index_list[i + 1];
        }

        bool add = true;

        for (int j = 0; j < 3; ++j) {
            if (us->neighbor[j].neighborIndex != -1
                && (us->neighbor[j].neighborEdgeIndex[0] == start && us->neighbor[j].neighborEdgeIndex[1] == end
                    || us->neighbor[j].neighborEdgeIndex[1] == start && us->neighbor[j].neighborEdgeIndex[0] == end)) {
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
    captainslog_assert(m_numSilhouetteIndices[index] < m_maxSilhouetteEntries[index]);

    m_silhouetteIndex[index][m_numSilhouetteIndices[index]++] = start;
    m_silhouetteIndex[index][m_numSilhouetteIndices[index]++] = end;
}

void W3DVolumetricShadow::Build_Silhouette(int mesh_index, Vector3 *light_pos)
{
    short index_list[3];
    PolyNeighbor *us;
    int poly_index;

    W3DShadowGeometryMesh *geo_mesh = m_geometry->Get_Mesh(mesh_index);

    int mesh_edge_start = m_numSilhouetteIndices[mesh_index];
    int num_polys = geo_mesh->Get_Num_Polygon();

    for (poly_index = 0; poly_index < num_polys; ++poly_index) {
        us = geo_mesh->Get_Poly_Neighbor(poly_index);
        us->status = 0;

        Vector3 *normal = geo_mesh->Get_Polygon_Normal(poly_index);

        geo_mesh->Get_Polygon_Index(poly_index, index_list);

        Vector3 delta = (*geo_mesh->Get_Vertex(index_list[0]) - *light_pos);
        if (delta * *normal < 0.0f) {
            us->status |= 1;
        }
    }

    for (poly_index = 0; poly_index < num_polys; ++poly_index) {
        us = geo_mesh->Get_Poly_Neighbor(poly_index);
        bool neighborless = false;

        for (int i = 0; i < 3; ++i) {
            PolyNeighbor *neighbor = nullptr;

            if (us->neighbor[i].neighborIndex != -1) {
                neighbor = geo_mesh->Get_Poly_Neighbor(us->neighbor[i].neighborIndex);
                if (neighbor->status & 2) {
                    continue;
                }
            }

            if (us->status & 1) {
                if (neighbor == nullptr) {
                    neighborless = true;
                } else if (!(neighbor->status & 1)) {
                    Add_Silhouette_Edge(mesh_index, us, neighbor);
                }
            } else if (neighbor != nullptr) {
                if (neighbor->status & 1) {
                    Add_Silhouette_Edge(mesh_index, neighbor, us);
                }
            }
        }

        if (neighborless) {
            Add_Neighborless_Edges(mesh_index, us);
        }

        us->status |= 2;
    }

    m_numIndicesPerMesh[mesh_index] = m_numSilhouetteIndices[mesh_index] - mesh_edge_start;
}

bool W3DVolumetricShadow::Allocate_Shadow_Volume(int volume_index, int mesh_index)
{
    if (volume_index < 0 || volume_index >= 1) {
        captainslog_assert(0);
        return false;
    }

    Geometry *geo = m_shadowVolume[volume_index][mesh_index];

    if (geo == nullptr) {
        geo = new Geometry;
        ++m_shadowVolumeCount[mesh_index];
    }

    if (geo == nullptr) {
        captainslog_assert(0);
        --m_shadowVolumeCount[mesh_index];
        return false;
    }

    m_shadowVolume[volume_index][mesh_index] = geo;

    int num = m_maxSilhouetteEntries[mesh_index];

    if (geo->Get_Flags() & 1 && !geo->Create(2 * num, num)) {

        captainslog_assert(0);

        delete geo;

        return false;
    }

    return true;
}

void W3DVolumetricShadow::Delete_Shadow_Volume(int volume_index)
{
    if (volume_index < 0 || volume_index >= 1) {
        captainslog_assert(0);
        return;
    }

    for (int mesh_index = 0; mesh_index < MAX_SHADOW_CASTER_MESHES; ++mesh_index) {
        if (m_shadowVolume[volume_index][mesh_index]) {
            Geometry *geo = m_shadowVolume[volume_index][mesh_index];

            delete geo;

            m_shadowVolume[volume_index][mesh_index] = nullptr;
            --m_shadowVolumeCount[mesh_index];
        }
    }
}

void W3DVolumetricShadow::Reset_Shadow_Volume(int volume_index, int mesh_index)
{
    if (volume_index < 0 || volume_index >= 1) {
        captainslog_assert(0);
        return;
    }

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

bool W3DVolumetricShadow::Allocate_Silhouette(int index, int count)
{
    short num_entries = 5 * count;

    captainslog_assert(m_silhouetteIndex[index] == nullptr && m_numSilhouetteIndices[index] == 0 && num_entries > 0);

    m_silhouetteIndex[index] = new short[num_entries];

    if (m_silhouetteIndex[index] == nullptr) {
        captainslog_assert(0);
        return false;
    }

    m_numSilhouetteIndices[index] = 0;
    m_maxSilhouetteEntries[index] = num_entries;

    return true;
}

void W3DVolumetricShadow::Delete_Silhouette(int index)
{
    if (m_silhouetteIndex[index] != nullptr) {
        delete[] m_silhouetteIndex[index];
    }

    m_silhouetteIndex[index] = nullptr;
    m_numSilhouetteIndices[index] = 0;
}

void W3DVolumetricShadow::Reset_Silhouette(int index)
{
    m_numSilhouetteIndices[index] = 0;
}

void W3DVolumetricShadow::Release()
{
    // TODO
    // g_theW3DVolumetricShadowManager->Remove_Shadow(this);
}

void W3DVolumetricShadow::Set_Render_Object(RenderObjClass *robj)
{
    captainslog_assert(m_robj == nullptr);
    m_robj = robj;
}

W3DVolumetricShadowManager::W3DVolumetricShadowManager() :
    m_shadowList(nullptr),
    // BUGFIX original didn't init these
    m_dynamicShadowVolumesToRender(nullptr),
    m_W3DShadowGeometryManager(nullptr)
{
    m_W3DShadowGeometryManager = new W3DShadowGeometryManager;

    g_theW3DBufferManager = new W3DBufferManager;
}

W3DVolumetricShadowManager::~W3DVolumetricShadowManager()
{
    Release_Resources();

    if (m_W3DShadowGeometryManager != nullptr) {
        delete m_W3DShadowGeometryManager;
    }

    m_W3DShadowGeometryManager = nullptr;

    if (g_theW3DBufferManager != nullptr) {
        delete g_theW3DBufferManager;
    }

    g_theW3DBufferManager = nullptr;
}

int W3DVolumetricShadowManager::Init()
{
    return 1;
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
    }
    if (g_shadowVertexBufferD3D != nullptr) {
        g_shadowVertexBufferD3D->Release();
    }
    g_shadowIndexBufferD3D = nullptr;
    g_shadowVertexBufferD3D = nullptr;
#endif

    if (g_theW3DBufferManager != nullptr) {
        g_theW3DBufferManager->Release_Resources();

        Invalidate_Cached_Light_Positions();
    }
}

int W3DVolumetricShadowManager::Re_Acquire_Resources()
{
    Release_Resources();
#ifdef BUILD_WITH_D3D8
    IDirect3DDevice8 *device = DX8Wrapper::Get_D3D_Device8();

    captainslog_dbgassert(device, "Trying to ReAquireResources on W3DVolumetricShadowManager without device");

    if (device->CreateIndexBuffer(sizeof(unsigned short) * SHADOW_INDEX_SIZE,
            D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
            D3DFMT_INDEX16,
            D3DPOOL_DEFAULT,
            &g_shadowIndexBufferD3D)
        < 0) {
        return 0;
    }

    if (!g_shadowVertexBufferD3D
        && device->CreateVertexBuffer(sizeof(Vector3) * SHADOW_VERTEX_SIZE,
               D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
               0,
               D3DPOOL_DEFAULT,
               &g_shadowVertexBufferD3D)
            < 0) {
        return 0;
    }

    if (g_theW3DBufferManager != nullptr && !g_theW3DBufferManager->ReAcquire_Resources()) {
        return 0;
    }
#endif
    return 1;
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

    W3DShadowGeometry *geo = m_W3DShadowGeometryManager->Get_Geom(name);

    if (geo == nullptr) {
        m_W3DShadowGeometryManager->Load_Geom(robj, name);
        geo = m_W3DShadowGeometryManager->Get_Geom(name);
        if (geo == nullptr) {
            return nullptr;
        }
    }

    W3DVolumetricShadow *shadow = new W3DVolumetricShadow;

    if (shadow == nullptr) {
        return nullptr;
    }

    shadow->Set_Render_Object(robj);
    shadow->Set_Geometry(geo);

    SphereClass sphere;
    robj->Get_Obj_Space_Bounding_Sphere(sphere);

    shadow->Set_Bounds_Radius(sphere.Radius * 1.0f);

    float scale = 0.0;

    if (shadow_info->m_sizeX != 0.0f) {
        scale = tan(shadow_info->m_sizeX / 180.0f * 3.1415927f);
    }

    shadow->Set_Shadow_Length_Scale(scale);

    if (drawable == nullptr || !drawable->Is_KindOf(KINDOF_IMMOBILE)) {
        shadow->Set_Optimal_Extrusion_Padding(0.1f);
    }

    shadow->m_next = m_shadowList;

    m_shadowList = shadow;

    return shadow;
}

void W3DVolumetricShadowManager::Remove_Shadow(W3DVolumetricShadow *shadow)
{
    W3DVolumetricShadow *s = nullptr;

    for (W3DVolumetricShadow *i = m_shadowList; i != nullptr; i = i->m_next) {
        if (i == shadow) {
            if (s != nullptr) {
                s->m_next = shadow->m_next;
            } else {
                m_shadowList = shadow->m_next;
            }
            delete shadow;
            return;
        }
        s = i;
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

void W3DVolumetricShadowManager::Invalidate_Cached_Light_Positions()
{
    if (m_shadowList != nullptr) {
        Vector3 pos(0.0f, 0.0f, 0.0f);

        for (W3DVolumetricShadow *i = m_shadowList; i != nullptr; i = i->m_next) {
            for (int vol = 0; vol < 1; ++vol) {
                for (int mesh = 0; mesh < MAX_SHADOW_CASTER_MESHES; ++mesh) {
                    i->Set_Light_Pos_History(vol, mesh, pos);
                }
            }
        }
    }
}

void W3DVolumetricShadowManager::Render_Stencil_Shadows()
{
#ifdef BUILD_WITH_D3D8
    struct _TRANS_LIT_VERTEX
    {
        Vector4 p;
        unsigned int c;
    };

    IDirect3DDevice8 *device = DX8Wrapper::Get_D3D_Device8();

    if (device != nullptr) {
        _TRANS_LIT_VERTEX vertex[4];

        int y;
        int x;
        g_theTacticalView->Get_Origin(&x, &y);
        int width = g_theTacticalView->Get_Width();
        int height = g_theTacticalView->Get_Height();

        vertex[0].p = Vector4((float)(width + x), (float)(height + y), 0.0f, 1.0f);
        vertex[1].p = Vector4((float)(width + x), 0.0f, 0.0f, 1.0f);
        vertex[2].p = Vector4((float)x, (float)(height + y), 0.0f, 1.0f);
        vertex[3].p = Vector4((float)x, 0.0f, 0.0f, 1.0f);

        vertex[0].c = g_theW3DShadowManager->Get_Shadow_Color();
        vertex[1].c = g_theW3DShadowManager->Get_Shadow_Color();
        vertex[2].c = g_theW3DShadowManager->Get_Shadow_Color();
        vertex[3].c = g_theW3DShadowManager->Get_Shadow_Color();

        device->SetVertexShader(D3DFVF_DIFFUSE | D3DFVF_XYZRHW);

        device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

        device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
        device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);

        device->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
        device->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);

        device->SetRenderState(D3DRS_STENCILENABLE, TRUE);
        device->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_LESSEQUAL);
        device->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
        device->SetRenderState(D3DRS_STENCILMASK, ~g_theW3DShadowManager->Get_Stencil_Mask());
        device->SetRenderState(D3DRS_STENCILREF, 1);

        device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);

        if (DX8Wrapper::Is_Triangle_Draw_Enabled()) {
            device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertex, sizeof(_TRANS_LIT_VERTEX));
        }

        device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);

        device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        device->SetRenderState(D3DRS_STENCILENABLE, FALSE);
    }
#endif
}

#ifdef GAME_DEBUG_STRUCTS
void W3DVolumetricShadow::Gather_Draw_Stats(DebugDrawStats *stats)
{
    HLodClass *hlod;
    MeshClass *mesh;

    int count = 0;
    if (m_geometry != nullptr) {

        if (m_isEnabled && !m_isInvisibleEnabled) {

            if (g_theWriteableGlobalData->m_shadowVolumes) {

                if (m_robj != nullptr) {
                    hlod = static_cast<HLodClass *>(m_robj);
                }

                for (int i = 0; i < 1; ++i) {
                    for (int j = 0; j < m_geometry->Get_Mesh_Count(); ++j) {

                        int model_index = m_geometry->Get_Mesh(j)->m_modelIndex;

                        if (model_index >= 0) {
                            RenderObjClass *lod_model = hlod->Peek_Lod_Model(0, model_index);

                            if (lod_model != nullptr) {
                                mesh = static_cast<MeshClass *>(lod_model);
                            }

                        } else {
                            if (m_robj != nullptr) {
                                mesh = static_cast<MeshClass *>(m_robj);
                            }
                        }

                        if (mesh != nullptr) {
                            if (mesh->Is_Not_Hidden_At_All()) {
                                ++count;
                            }
                        }
                    }
                }
            }
        }
    }
    stats->Add_Extra_Draw_Calls(2 * count);
}
#endif