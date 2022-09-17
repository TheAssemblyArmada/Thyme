/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Mesh Geometry Class
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
#include "coltest.h"
#include "inttest.h"
#include "multilist.h"
#include "refcount.h"
#include "sharebuf.h"
#include "simplevec.h"
#include "vector3.h"
#include "vector3i.h"
#include "vector4.h"
#include "w3derr.h"
#include "w3dmpo.h"

class AABoxClass;
class OBBoxClass;
class SphereClass;
class ChunkLoadClass;
class AABTreeClass;
class HTreeClass;
class RenderInfoClass;

typedef Vector3i16 TriIndex;

class MeshGeometryClass : public W3DMPO, public RefCountClass, public MultiListObjectClass
{
public:
    enum FlagsType
    {
        DIRTY_BOUNDS = 0x00000001,
        DIRTY_PLANES = 0x00000002,
        DIRTY_VNORMALS = 0x00000004,

        SORT = 0x00000010,
        DISABLE_BOUNDING_BOX = 0x00000020,
        DISABLE_BOUNDING_SPHERE = 0x00000040,
        DISABLE_PLANE_EQ = 0x00000080,
        TWO_SIDED = 0x00000100,

        ALIGNED = 0x00000200,
        SKIN = 0x00000400,
        ORIENTED = 0x00000800,
        CAST_SHADOW = 0x00001000,

        PRELIT_MASK = 0x0000E000,
        PRELIT_VERTEX = 0x00002000,
        PRELIT_LIGHTMAP_MULTI_PASS = 0x00004000,
        PRELIT_LIGHTMAP_MULTI_TEXTURE = 0x00008000,

        ALLOW_NPATCHES = 0x00010000,
    };

    MeshGeometryClass();
    MeshGeometryClass(const MeshGeometryClass &that);
    virtual ~MeshGeometryClass() override;

    MeshGeometryClass &operator=(const MeshGeometryClass &that);
    void Reset_Geometry(int polycount, int vertcount);
    void Compute_Plane(int pidx, PlaneClass *set_plane) const;
    bool Has_Cull_Tree() { return m_cullTree != nullptr; }
    virtual W3DErrorType Load_W3D(ChunkLoadClass &cload);
    void Scale(const Vector3 &sc);
    bool Contains(const Vector3 &point);

    void Generate_Rigid_APT(const Vector3 &view_dir, SimpleDynVecClass<uint32_t> &apt);
    void Generate_Rigid_APT(const OBBoxClass &local_box, SimpleDynVecClass<uint32_t> &apt);
    void Generate_Rigid_APT(const OBBoxClass &local_box, const Vector3 &view_dir, SimpleDynVecClass<uint32_t> &apt);
    void Generate_Skin_APT(const OBBoxClass &world_box, SimpleDynVecClass<uint32_t> &apt, const Vector3 *world_vertex_locs);

    bool Cast_Ray(RayCollisionTestClass &raytest);
    bool Cast_AABox(AABoxCollisionTestClass &boxtest);
    bool Cast_OBBox(OBBoxCollisionTestClass &boxtest);
    bool Intersect_OBBox(OBBoxIntersectionTestClass &boxtest);
    bool Cast_World_Space_AABox(AABoxCollisionTestClass &boxtest, const Matrix3D &transform);

    const char *Get_Name() const;
    const char *Get_User_Text();
    int Get_Flag(FlagsType flag) { return m_flags & flag; }
    int Get_Sort_Level() const { return m_sortLevel; }
    int Get_Polygon_Count() const { return m_polyCount; }
    int Get_Vertex_Count() const { return m_vertexCount; }
    const TriIndex *Get_Polygon_Array() { return Get_Polys(); }
    Vector3 *Get_Vertex_Array()
    {
        captainslog_assert(m_vertex);
        return m_vertex->Get_Array();
    }
    const Vector3 *Get_Vertex_Normal_Array();
    const Vector4 *Get_Plane_Array(bool create = true);
    const uint32_t *Get_Vertex_Shade_Index_Array(bool create = true) { return Get_Shade_Indices(create); }
    const uint16_t *Get_Vertex_Bone_Links() { return Get_Bone_Links(); }
    uint8_t *Get_Poly_Surface_Type_Array()
    {
        captainslog_assert(m_polySurfaceType);
        return m_polySurfaceType->Get_Array();
    }
    void Get_Bounding_Box(AABoxClass *set_box);
    void Get_Bounding_Sphere(SphereClass *set_sphere);
    uint8_t Get_Poly_Surface_Type(int poly_index) const;

    void Set_Name(const char *newname);
    void Set_User_Text(char *usertext);
    void Set_Sort_Level(int level) { m_sortLevel = level; }
    void Set_Flag(FlagsType flag, bool onoff);

protected:
    TriIndex *Get_Polys()
    {
        captainslog_assert(m_poly);
        return m_poly->Get_Array();
    }
    Vector3 *Get_Vert_Normals();
    Vector4 *Get_Planes(bool create = true);
    uint32_t *Get_Shade_Indices(bool create = true);
    uint16_t *Get_Bone_Links(bool create = true);

    int Cast_Semi_Infinite_Axis_Aligned_Ray(const Vector3 &start_point, int axis_dir, unsigned char &flags);
    bool Cast_AABox_Identity(AABoxCollisionTestClass &boxtest, const Vector3 &trans);
    bool Cast_AABox_Z90(AABoxCollisionTestClass &boxtest, const Vector3 &trans);
    bool Cast_AABox_Z180(AABoxCollisionTestClass &boxtest, const Vector3 &trans);
    bool Cast_AABox_Z270(AABoxCollisionTestClass &boxtest, const Vector3 &trans);
    bool Intersect_OBBox_Brute_Force(OBBoxIntersectionTestClass &localtest);
    bool Cast_Ray_Brute_Force(RayCollisionTestClass &raytest);
    bool Cast_AABox_Brute_Force(AABoxCollisionTestClass &boxtest);
    bool Cast_OBBox_Brute_Force(OBBoxCollisionTestClass &boxtest);

    virtual void Compute_Plane_Equations(Vector4 *array);
    virtual void Compute_Vertex_Normals(Vector3 *array);
    virtual void Compute_Bounds(Vector3 *verts);
    void Generate_Culling_Tree();

public:
    void Get_Deformed_Vertices(Vector3 *dst_vert, Vector3 *dst_norm, const HTreeClass *htree);
    void Get_Deformed_Vertices(Vector3 *dst_vert, const HTreeClass *htree);
    void Get_Deformed_Screenspace_Vertices(
        Vector4 *dst_vert, const RenderInfoClass &rinfo, const Matrix3D &mesh_tm, const HTreeClass *htree);

protected:
    W3DErrorType Read_Chunks(ChunkLoadClass &cload);
    W3DErrorType Read_Vertices(ChunkLoadClass &cload);
    W3DErrorType Read_Vertex_Normals(ChunkLoadClass &cload);
    W3DErrorType Read_Triangles(ChunkLoadClass &cload);
    W3DErrorType Read_User_Text(ChunkLoadClass &cload);
    W3DErrorType Read_Vertex_Influences(ChunkLoadClass &cload);
    W3DErrorType Read_Vertex_Shade_Indices(ChunkLoadClass &cload);
    W3DErrorType Read_AABTree(ChunkLoadClass &cload);

    ShareBufferClass<char> *m_meshName;
    ShareBufferClass<char> *m_userText;
    int m_flags;
    char m_sortLevel;
    uint32_t m_w3dAttributes;
    int m_polyCount;
    int m_vertexCount;
    ShareBufferClass<TriIndex> *m_poly;
    ShareBufferClass<Vector3> *m_vertex;
    ShareBufferClass<Vector3> *m_vertexNorm;
    ShareBufferClass<Vector4> *m_planeEq;
    ShareBufferClass<uint32_t> *m_vertexShadeIdx;
    ShareBufferClass<uint16_t> *m_vertexBoneLink;
    ShareBufferClass<uint8_t> *m_polySurfaceType;
    Vector3 m_boundBoxMin;
    Vector3 m_boundBoxMax;
    Vector3 m_boundSphereCenter;
    float m_boundSphereRadius;
    AABTreeClass *m_cullTree;
};

inline void MeshGeometryClass::Set_Flag(FlagsType flag, bool onoff)
{
    if (onoff) {
        m_flags |= flag;
    } else {
        m_flags &= ~flag;
    }
}

inline uint32_t *MeshGeometryClass::Get_Shade_Indices(bool create)
{
    if (create && !m_vertexShadeIdx) {
        m_vertexShadeIdx = New_Share_Buffer<uint32_t>(m_vertexCount, "MeshGeometryClass::VertexShadeIdx");
    }

    if (m_vertexShadeIdx) {
        return m_vertexShadeIdx->Get_Array();
    }

    return nullptr;
}

inline uint16_t *MeshGeometryClass::Get_Bone_Links(bool create)
{
    if (create && !m_vertexBoneLink) {
        m_vertexBoneLink = New_Share_Buffer<uint16_t>(m_vertexCount, "MeshGeometryClass::VertexBoneLink");
    }

    if (m_vertexBoneLink) {
        return m_vertexBoneLink->Get_Array();
    }

    return nullptr;
}

inline uint8_t MeshGeometryClass::Get_Poly_Surface_Type(int poly_index) const
{
    captainslog_assert(m_polySurfaceType);
    captainslog_assert(poly_index >= 0 && poly_index < m_polyCount);
    uint8_t *type = m_polySurfaceType->Get_Array();
    return type[poly_index];
}
