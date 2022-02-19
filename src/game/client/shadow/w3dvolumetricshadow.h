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
#include "hash.h"
#include "refcount.h"
#include "vector3.h"
#include "vector3i.h"
#include <new>

class RenderObjClass;
class W3DShadowGeometry;
class MeshClass;
class Vector3;

typedef Vector3i16 TriIndex;

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
    friend W3DShadowGeometry;

public:
    W3DShadowGeometryMesh();
    ~W3DShadowGeometryMesh();

    PolyNeighbor *Get_Poly_Neighbor(int32_t index);
    void Build_Polygon_Neighbors();
    bool Allocate_Neighbors(int32_t num_polys);
    void Delete_Neighbors();

    Vector3 *Get_Polygon_Normal(int32_t index) const;

    int32_t Get_Num_Polygon() const { return m_numPolygons; }

    void Build_Polygon_Normals();

    void Get_Polygon_Normal(int32_t index, Vector3 *normal) const;
    void Get_Polygon_Index(int32_t polygon_index, short *index_list) const;

    Vector3 *Get_Vertex(int32_t index) const { return &m_verts[index]; }
    int32_t Get_Num_Vertex() const { return m_numVerts; }

#ifdef GAME_DLL
    W3DShadowGeometryMesh *Hook_Ctor() { return new (this) W3DShadowGeometryMesh(); }
#endif

private:
    MeshClass *m_mesh;
    int32_t m_modelIndex;
    Vector3 *m_verts;
    Vector3 *m_polygonNormals;
    int32_t m_numVerts;
    int32_t m_numPolygons;
    const TriIndex *m_polygons;
    unsigned short *m_parentVerts;
    PolyNeighbor *m_polyNeighbors;
    int32_t m_numPolyNeighbors;
    W3DShadowGeometry *m_parentGeometry;
};

class W3DShadowGeometry : public RefCountClass, public HashableClass
{

public:
    W3DShadowGeometry();

    virtual ~W3DShadowGeometry() {}

    const char *Get_Key() override { return m_name; }

    void Set_Name(const char *name);

    int32_t Init(RenderObjClass *robj);
    int32_t Init_From_HLOD(RenderObjClass *robj);
    int32_t Init_From_Mesh(RenderObjClass *robj);

    const char *Get_Name() const { return m_name; }
    W3DShadowGeometryMesh *Get_Mesh(int32_t index) { return &m_meshList[index]; }
    int32_t Get_Mesh_Count() const { return m_meshCount; }
    int32_t Get_Num_Total_Vertex() const { return m_numTotalsVerts; }

protected:
    char m_name[32];
    W3DShadowGeometryMesh m_meshList[MAX_SHADOW_CASTER_MESHES];
    int32_t m_meshCount;
    int32_t m_numTotalsVerts;
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

    int32_t Load_Geom(RenderObjClass *robj, const char *name);

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
