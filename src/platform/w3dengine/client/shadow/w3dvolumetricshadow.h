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
#pragma once

#include "always.h"
#include "aabox.h"
#include "colmath.h"
#include "drawmodule.h"
#include "hash.h"
#include "matrix4.h"
#include "refcount.h"
#include "sphere.h"
#include "vector3.h"
#include "vector3i.h"
#include "w3dbuffermanager.h"
#include "w3dshadow.h"
#include <new>
#ifdef BUILD_WITH_D3D8
#include "dx8wrapper.h"
#endif

class RenderObjClass;
class W3DShadowGeometry;
class MeshClass;
class Vector3;

typedef Vector3i16 TriIndex;

enum
{
    // these were globals but theres little point in having them as such
    SHADOW_VERTEX_SIZE = 4096,
    SHADOW_INDEX_SIZE = 8192,
};

enum
{
    MAX_SHADOW_CASTER_MESHES = 160,
};

#pragma pack(push, 2)

struct NeighborEdge
{
    short neighborIndex;
    short neighborEdgeIndex[2];
};

struct PolyNeighbor
{
    short myIndex;
    char status;
    NeighborEdge neighbor[3];
};
#pragma pack(pop)

static_assert(sizeof(PolyNeighbor) == 0x16, "PolyNeighbor size incorrect!");

class W3DShadowGeometryMesh
{
public:
    W3DShadowGeometryMesh();
    ~W3DShadowGeometryMesh();

    PolyNeighbor *Get_Poly_Neighbor(int index);
    void Build_Polygon_Neighbors();
    bool Allocate_Neighbors(int num_polys);
    void Delete_Neighbors();

    Vector3 *Get_Polygon_Normal(int index) const;

    int Get_Num_Polygon() const { return m_numPolygons; }

    void Build_Polygon_Normals();

    void Get_Polygon_Normal(int index, Vector3 *normal) const;
    void Get_Polygon_Index(int polygon_index, short *index_list) const;

    Vector3 *Get_Vertex(int index) const { return &m_verts[index]; }
    int Get_Num_Vertex() const { return m_numVerts; }

#ifdef GAME_DLL
    W3DShadowGeometryMesh *Hook_Ctor() { return new (this) W3DShadowGeometryMesh(); }
#endif

private:
    MeshClass *m_mesh;
    int m_modelIndex;
    Vector3 *m_verts;
    Vector3 *m_polygonNormals;
    int m_numVerts;
    int m_numPolygons;
    const TriIndex *m_polygons;
    unsigned short *m_parentVerts;
    PolyNeighbor *m_polyNeighbors;
    int m_numPolyNeighbors;
    W3DShadowGeometry *m_parentGeometry;
    friend class W3DVolumetricShadow;
    friend W3DShadowGeometry;
};

class W3DShadowGeometry : public RefCountClass, public HashableClass
{

public:
    W3DShadowGeometry();

    virtual ~W3DShadowGeometry() {}

    const char *Get_Key() override { return m_name; }

    void Set_Name(const char *name);

    int Init(RenderObjClass *robj);
    int Init_From_HLOD(RenderObjClass *robj);
    int Init_From_Mesh(RenderObjClass *robj);

    const char *Get_Name() const { return m_name; }
    W3DShadowGeometryMesh *Get_Mesh(int index) { return &m_meshList[index]; }
    int Get_Mesh_Count() const { return m_meshCount; }
    int Get_Num_Total_Vertex() const { return m_numTotalsVerts; }

protected:
    char m_name[32];
    W3DShadowGeometryMesh m_meshList[MAX_SHADOW_CASTER_MESHES];
    int m_meshCount;
    int m_numTotalsVerts;
};

class W3DShadowGeometryManager
{
    friend class W3DShadowGeometryManagerIterator;

public:
    W3DShadowGeometryManager();
    ~W3DShadowGeometryManager();

    void Free_All_Geoms();

    W3DShadowGeometry *Peek_Geom(const char *name);
    W3DShadowGeometry *Get_Geom(const char *name);

    bool Add_Geom(W3DShadowGeometry *new_geom);

    void Register_Missing(const char *name);
    bool Is_Missing(const char *name) const;

    int Load_Geom(RenderObjClass *robj, const char *name);

protected:
    HashTableClass *m_geomPtrTable;
    HashTableClass *m_missingGeomTable;
};

class W3DShadowGeometryManagerIterator : public HashTableIteratorClass
{
public:
    W3DShadowGeometryManagerIterator(W3DShadowGeometryManager &manager) : HashTableIteratorClass(*manager.m_geomPtrTable) {}
    virtual ~W3DShadowGeometryManagerIterator() {}

    W3DShadowGeometry *Get_Current_Geom();
};

class W3DVolumetricShadow;

struct W3DVolumetricShadowRenderTask : W3DBufferManager::W3DRenderTask
{
    W3DVolumetricShadow *m_parentShadow;
    unsigned char m_meshIndex;
    unsigned char m_lightIndex;
};

class W3DVolumetricShadowManager
{
public:
    W3DVolumetricShadowManager();
    ~W3DVolumetricShadowManager();

    bool Init();
    void Reset();

    void Release_Resources();
    bool Re_Acquire_Resources();

    W3DVolumetricShadow *Add_Shadow(RenderObjClass *robj, Shadow::ShadowTypeInfo *shadow_info, Drawable *drawable);

    void Remove_Shadow(W3DVolumetricShadow *shadow);
    void Remove_All_Shadows();

    void Add_Dynamic_Shadow_Task(W3DVolumetricShadowRenderTask *task);

    void Render_Stencil_Shadows();
    void Render_Shadows(bool force_stencil_fill);

    void Invalidate_Cached_Light_Positions();

private:
    W3DVolumetricShadow *m_shadowList;
    W3DVolumetricShadowRenderTask *m_dynamicShadowVolumesToRender;
    W3DShadowGeometryManager *m_W3DShadowGeometryManager;
};

struct Geometry
{
public:
    enum VisibleState
    {
        STATE_INVISIBLE = CollisionMath::OUTSIDE,
        STATE_VISIBLE = CollisionMath::INSIDE,
        STATE_UNKNOWN = CollisionMath::OVERLAPPED,
    };

    Geometry();
    ~Geometry();

    // not yet found
    Geometry(Geometry &);
    Geometry &operator=(Geometry &);

    bool Create(int num_vert, int num_poly);

    void Release();

    int Get_Flags() const { return m_flags; }
    void Set_Flags(int flags) { m_flags = flags; }

    int Get_Num_Polygon() const { return m_numPolygon; }
    int Get_Num_Vertex() const { return m_numVertex; }

    int Get_Num_Active_Polygon() const { return m_numActivePolygon; }
    int Set_Num_Active_Polygon(int num)
    {
        m_numActivePolygon = num;
        return num;
    }

    int Get_Num_Active_Vertex() const { return m_numActiveVertex; }
    int Set_Num_Active_Vertex(int num)
    {
        m_numActiveVertex = num;
        return num;
    }

    unsigned short *Get_Polygon_Index(int index, unsigned short *index_list) const
    {
        index_list[0] = m_indices[3 * index + 0];
        index_list[1] = m_indices[3 * index + 1];
        index_list[2] = m_indices[3 * index + 2];
        return &m_indices[index];
    }

    int Set_Polygon_Index(int index, const unsigned short *index_list)
    {
        m_indices[3 * index + 0] = index_list[0];
        m_indices[3 * index + 1] = index_list[1];
        m_indices[3 * index + 2] = index_list[2];
        return 3;
    }

    Vector3 *Get_Vertex(int index) const { return &m_verts[index]; }
    void Set_Vertex(int index, Vector3 *vert) { m_verts[index] = *vert; }

    // not yet found
    int Find_Vertex_In_Range(int, int, Vector3 *);

    AABoxClass &Get_Bounding_Box() { return m_boundingBox; }
    void Set_Bounding_Box(AABoxClass &box) { m_boundingBox = box; }

    SphereClass &Get_Bounding_Sphere() { return m_boundingSphere; }
    void Set_Bounding_Sphere(SphereClass &sphere) { m_boundingSphere = sphere; }

    VisibleState Get_Visible_State() const { return m_visibleState; }
    void Set_Visible_State(VisibleState state) { m_visibleState = state; }

private:
    Vector3 *m_verts;
    unsigned short *m_indices;

    int m_numPolygon;
    int m_numVertex;
    int m_numActivePolygon;
    int m_numActiveVertex;

    int m_flags;

    AABoxClass m_boundingBox;
    SphereClass m_boundingSphere;

    VisibleState m_visibleState;
};

class W3DVolumetricShadow : public Shadow
{
    friend class W3DVolumetricShadowManager;

public:
    W3DVolumetricShadow();
    ~W3DVolumetricShadow();

    void Set_Geometry(W3DShadowGeometry *geometry);

    void Update();
    void Update_Volumes(float zoffset);
    void Update_Mesh_Volume(
        int mesh_index, int light_index, const Matrix3D *mesh_xform, const AABoxClass &mesh_box, float floor_z);

    void Add_Silhouette_Edge(int mesh_index, PolyNeighbor *poly_neighbor, PolyNeighbor *hidden);
    void Add_Neighborless_Edges(int mesh_index, PolyNeighbor *poly_neighbor);
    void Add_Silhouette_Indices(int index, short start, short end);

    bool Allocate_Shadow_Volume(int volume_index, int mesh_index);
    void Delete_Shadow_Volume(int volume_index);
    void Reset_Shadow_Volume(int volume_index, int mesh_index);

    bool Allocate_Silhouette(int mesh_index, int num_vertices);
    void Delete_Silhouette(int mesh_index);
    void Reset_Silhouette(int mesh_index);
    void Build_Silhouette(int mesh_index, Vector3 *light_pos_object);

    void Release() override;

    void Update_Optimal_Extrusion_Padding();

    void Construct_Volume(Vector3 *light_pos_object, float shadow_extrude_distance, int volume_index, int mesh_index);
    void Construct_Volume_VB(Vector3 *light_pos_object, float shadow_extrude_distance, int volume_index, int mesh_index);

#ifdef GAME_DEBUG_STRUCTS
    virtual void Gather_Draw_Stats(DebugDrawStats *stats);
#endif

    void Render_Volume(int mesh_index, int light_index);
    void Render_Mesh_Volume(int mesh_index, int light_index, const Matrix3D *mesh_xform);
    void Render_Dynamic_Mesh_Volume(int mesh_index, int light_index, const Matrix3D *mesh_xform);

    void Set_Light_Pos_History(int volume_index, int mesh_index, const Vector3 &pos)
    {
        m_lightPosHistory[volume_index][mesh_index] = pos;
    }

    void Set_Shadow_Length_Scale(float scale) { m_shadowLengthScale = scale; }

    void Set_Optimal_Extrusion_Padding(float padding) { m_optimalExtrusionPadding = padding; }

    void Set_Render_Object(RenderObjClass *robj) { m_robj = robj; }

    void Set_Bounds_Radius(float radius) { m_boundsRadius = radius; }

private:
    W3DVolumetricShadow *m_next;

    W3DShadowGeometry *m_geometry;
    RenderObjClass *m_robj;
    float m_shadowLengthScale;
    float m_boundsRadius;
    float m_optimalExtrusionPadding;

    Geometry *m_shadowVolume[1][MAX_SHADOW_CASTER_MESHES];

    W3DBufferManager::W3DVertexBufferSlot *m_shadowVolumeVB[1][MAX_SHADOW_CASTER_MESHES];
    W3DBufferManager::W3DIndexBufferSlot *m_shadowVolumeIB[1][MAX_SHADOW_CASTER_MESHES];

    W3DVolumetricShadowRenderTask m_shadowVolumeRenderTask[1][MAX_SHADOW_CASTER_MESHES];

    int m_shadowVolumeCount[MAX_SHADOW_CASTER_MESHES];

    Vector3 m_lightPosHistory[1][MAX_SHADOW_CASTER_MESHES];
    Matrix4 m_objectXformHistory[1][MAX_SHADOW_CASTER_MESHES];

    short *m_silhouetteIndex[MAX_SHADOW_CASTER_MESHES];

    short m_numSilhouetteIndices[MAX_SHADOW_CASTER_MESHES];
    short m_maxSilhouetteEntries[MAX_SHADOW_CASTER_MESHES];

    int m_numIndicesPerMesh[MAX_SHADOW_CASTER_MESHES];
};

#ifdef GAME_DLL
extern W3DVolumetricShadowManager *&g_theW3DVolumetricShadowManager;
#else
extern W3DVolumetricShadowManager *g_theW3DVolumetricShadowManager;
#endif

#ifdef BUILD_WITH_D3D8
#ifndef GAME_DLL
extern IDirect3DVertexBuffer8 *g_shadowVertexBufferD3D;
extern IDirect3DIndexBuffer8 *g_shadowIndexBufferD3D;
#else
extern IDirect3DVertexBuffer8 *&g_shadowVertexBufferD3D;
extern IDirect3DIndexBuffer8 *&g_shadowIndexBufferD3D;
#endif
#endif

extern int g_nShadowVertsInBuf;
extern int g_nShadowStartBatchVertex;
extern int g_nShadowIndicesInBuf;
extern int g_nShadowStartBatchIndex;
