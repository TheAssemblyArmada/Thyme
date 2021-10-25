/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief aabtree
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
#include "aabtreebuilder.h"
#include "aaplane.h"
#include "bittype.h"
#include "colmath.h"
#include "obbox.h"
#include "refcount.h"
#include "simplevec.h"
#include "tri.h"
#include "vector3.h"
#include "vector3i.h"

class MeshClass;
class CameraClass;
class RayCollisionTestClass;
class AABoxCollisionTestClass;
class OBBoxCollisionTestClass;
class OBBoxIntersectionTestClass;
class ChunkLoadClass;
class ChunkSaveClass;
class MeshGeometryClass;
class OBBoxClass;
class ChunkLoadClass;

struct BoxRayAPTContextStruct;

class AABTreeClass : public W3DMPO, public RefCountClass
{
    IMPLEMENT_W3D_POOL(AABTreeClass);

public:
    AABTreeClass();
    AABTreeClass(AABTreeBuilderClass *builder);
    AABTreeClass(const AABTreeClass &that);
    ~AABTreeClass();

    void Load_W3D(ChunkLoadClass &cload);

    int Get_Node_Count() { return m_nodeCount; }
    int Get_Poly_Count() { return m_polyCount; }
    int Compute_Ram_Size();
    void Generate_APT(const OBBoxClass &box, SimpleDynVecClass<uint32_t> &apt);
    void Generate_APT(const OBBoxClass &box, const Vector3 &viewdir, SimpleDynVecClass<uint32_t> &apt);

    bool Cast_Ray(RayCollisionTestClass &raytest);
    int Cast_Semi_Infinite_Axis_Aligned_Ray(const Vector3 &start_point, int axis_dir, unsigned char &flags);
    bool Cast_AABox(AABoxCollisionTestClass &boxtest);
    bool Cast_OBBox(OBBoxCollisionTestClass &boxtest);
    bool Intersect_OBBox(OBBoxIntersectionTestClass &boxtest);
    void Scale(float scale);

private:
    AABTreeClass &operator=(const AABTreeClass &that);

    void Read_Poly_Indices(ChunkLoadClass &cload);
    void Read_Nodes(ChunkLoadClass &cload);

    void Build_Tree_Recursive(AABTreeBuilderClass::CullNodeStruct *node, int &curpolyindex);
    void Reset();
    void Set_Mesh(MeshGeometryClass *mesh);
    void Update_Bounding_Boxes();
    void Update_Min_Max(int index, Vector3 &min, Vector3 &max);

    struct CullNodeStruct
    {
        Vector3 m_min;
        Vector3 m_max;

        uint32_t m_frontOrPoly0;
        uint32_t m_backOrPolyCount;

        inline bool Is_Leaf();

        inline int Get_Back_Child();
        inline int Get_Front_Child();
        inline int Get_Poly0();
        inline int Get_Poly_Count();

        inline void Set_Front_Child(uint32_t index);
        inline void Set_Back_Child(uint32_t index);
        inline void Set_Poly0(uint32_t index);
        inline void Set_Poly_Count(uint32_t count);
    };

    struct OBBoxAPTContextStruct
    {
        OBBoxAPTContextStruct(const OBBoxClass &box, SimpleDynVecClass<uint32_t> &apt) : m_box(box), m_APT(apt) {}

        OBBoxClass m_box;
        SimpleDynVecClass<uint32_t> &m_APT;
    };

    struct OBBoxRayAPTContextStruct
    {
        OBBoxRayAPTContextStruct(const OBBoxClass &box, const Vector3 &viewdir, SimpleDynVecClass<uint32_t> &apt) :
            m_box(box), m_viewVector(viewdir), m_APT(apt)
        {
        }

        OBBoxClass m_box;
        Vector3 m_viewVector;
        SimpleDynVecClass<uint32_t> &m_APT;
    };

    void Generate_OBBox_APT_Recursive(CullNodeStruct *node, OBBoxAPTContextStruct &context);
    void Generate_OBBox_APT_Recursive(CullNodeStruct *node, OBBoxRayAPTContextStruct &context);

    bool Cast_Ray_Recursive(CullNodeStruct *node, RayCollisionTestClass &raytest);
    int Cast_Semi_Infinite_Axis_Aligned_Ray_Recursive(CullNodeStruct *node,
        const Vector3 &start_point,
        int axis_r,
        int axis_1,
        int axis_2,
        int direction,
        unsigned char &flags);
    bool Cast_AABox_Recursive(CullNodeStruct *node, AABoxCollisionTestClass &boxtest);
    bool Cast_OBBox_Recursive(CullNodeStruct *node, OBBoxCollisionTestClass &boxtest);
    bool Intersect_OBBox_Recursive(CullNodeStruct *node, OBBoxIntersectionTestClass &boxtest);

    bool Cast_Ray_To_Polys(CullNodeStruct *node, RayCollisionTestClass &raytest);
    int Cast_Semi_Infinite_Axis_Aligned_Ray_To_Polys(CullNodeStruct *node,
        const Vector3 &start_point,
        int axis_r,
        int axis_1,
        int axis_2,
        int direction,
        unsigned char &flags);
    bool Cast_AABox_To_Polys(CullNodeStruct *node, AABoxCollisionTestClass &boxtest);
    bool Cast_OBBox_To_Polys(CullNodeStruct *node, OBBoxCollisionTestClass &boxtest);
    bool Intersect_OBBox_With_Polys(CullNodeStruct *node, OBBoxIntersectionTestClass &boxtest);

    void Update_Bounding_Boxes_Recursive(CullNodeStruct *node);

    int m_nodeCount;
    CullNodeStruct *m_nodes;
    int m_polyCount;
    uint32_t *m_polyIndices;
    MeshGeometryClass *m_mesh;

    friend class MeshClass;
    friend class MeshGeometryClass;
    friend class AuxMeshDataClass;
    friend class AABTreeBuilderClass;
};

inline int AABTreeClass::Compute_Ram_Size()
{
    return m_nodeCount * sizeof(CullNodeStruct) + m_polyCount * sizeof(int) + sizeof(AABTreeClass);
}

inline bool AABTreeClass::Cast_Ray(RayCollisionTestClass &raytest)
{
    captainslog_assert(m_nodes != nullptr);
    return Cast_Ray_Recursive(&(m_nodes[0]), raytest);
}

inline int AABTreeClass::Cast_Semi_Infinite_Axis_Aligned_Ray(const Vector3 &start_point, int axis_dir, unsigned char &flags)
{
    static const int axis_r[6] = { 0, 0, 1, 1, 2, 2 };
    static const int axis_1[6] = { 1, 1, 2, 2, 0, 0 };
    static const int axis_2[6] = { 2, 2, 0, 0, 1, 1 };
    static const int direction[6] = { 1, 0, 1, 0, 1, 0 };
    captainslog_assert(m_nodes != nullptr);
    captainslog_assert(axis_dir >= 0);
    captainslog_assert(axis_dir < 6);

    flags = TRI_RAYCAST_FLAG_NONE;

    return Cast_Semi_Infinite_Axis_Aligned_Ray_Recursive(
        &(m_nodes[0]), start_point, axis_r[axis_dir], axis_1[axis_dir], axis_2[axis_dir], direction[axis_dir], flags);
}

inline bool AABTreeClass::Cast_AABox(AABoxCollisionTestClass &boxtest)
{
    captainslog_assert(m_nodes != nullptr);
    return Cast_AABox_Recursive(&(m_nodes[0]), boxtest);
}

inline bool AABTreeClass::Cast_OBBox(OBBoxCollisionTestClass &boxtest)
{
    captainslog_assert(m_nodes != nullptr);
    return Cast_OBBox_Recursive(&(m_nodes[0]), boxtest);
}

inline bool AABTreeClass::Intersect_OBBox(OBBoxIntersectionTestClass &boxtest)
{
    captainslog_assert(m_nodes != nullptr);
    return Intersect_OBBox_Recursive(&(m_nodes[0]), boxtest);
}

inline void AABTreeClass::Update_Bounding_Boxes()
{
    captainslog_assert(m_nodes != nullptr);
    Update_Bounding_Boxes_Recursive(&(m_nodes[0]));
}

inline bool AABTreeClass::CullNodeStruct::Is_Leaf()
{
    return ((m_frontOrPoly0 & AABTREE_LEAF_FLAG) != 0);
}

inline int AABTreeClass::CullNodeStruct::Get_Front_Child()
{
    captainslog_assert(!Is_Leaf());
    return m_frontOrPoly0;
}

inline int AABTreeClass::CullNodeStruct::Get_Back_Child()
{
    captainslog_assert(!Is_Leaf());
    return m_backOrPolyCount;
}

inline int AABTreeClass::CullNodeStruct::Get_Poly0()
{
    captainslog_assert(Is_Leaf());
    return (m_frontOrPoly0 & ~AABTREE_LEAF_FLAG);
}

inline int AABTreeClass::CullNodeStruct::Get_Poly_Count()
{
    captainslog_assert(Is_Leaf());
    return m_backOrPolyCount;
}

inline void AABTreeClass::CullNodeStruct::Set_Front_Child(uint32_t index)
{
    captainslog_assert(index < 0x7FFFFFFF);
    m_frontOrPoly0 = index;
}

inline void AABTreeClass::CullNodeStruct::Set_Back_Child(uint32_t index)
{
    captainslog_assert(index < 0x7FFFFFFF);
    m_backOrPolyCount = index;
}

inline void AABTreeClass::CullNodeStruct::Set_Poly0(uint32_t index)
{
    captainslog_assert(index < 0x7FFFFFFF);
    m_frontOrPoly0 = (index | AABTREE_LEAF_FLAG);
}

inline void AABTreeClass::CullNodeStruct::Set_Poly_Count(uint32_t count)
{
    captainslog_assert(count < 0x7FFFFFFF);
    m_backOrPolyCount = count;
}
