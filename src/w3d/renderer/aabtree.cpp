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
#include "aabtree.h"
#include "aabtreebuilder.h"
#include "camera.h"
#include "chunkio.h"
#include "coltest.h"
#include "inttest.h"
#include "meshgeometry.h"
#include "tri.h"
#include "w3d_file.h"

AABTreeClass::AABTreeClass() : m_nodeCount(0), m_nodes(nullptr), m_polyCount(0), m_polyIndices(nullptr), m_mesh(nullptr) {}

AABTreeClass::AABTreeClass(AABTreeBuilderClass *builder)
{
    m_nodeCount = builder->Node_Count();
    m_nodes = new AABTreeClass::CullNodeStruct[m_nodeCount];

    m_polyCount = builder->Poly_Count();
    m_polyIndices = new uint32_t[m_polyCount];

    int curpolyindex = 0;
    Build_Tree_Recursive(builder->m_root, curpolyindex);
}

AABTreeClass::AABTreeClass(const AABTreeClass &that) :
    m_nodeCount(0), m_nodes(nullptr), m_polyCount(0), m_polyIndices(0), m_mesh(nullptr)
{
    *this = that;
}

AABTreeClass::~AABTreeClass()
{
    Reset();
}

AABTreeClass &AABTreeClass::operator=(const AABTreeClass &that)
{
    Reset();
    m_nodeCount = that.m_nodeCount;

    if (m_nodeCount > 0) {
        m_nodes = new CullNodeStruct[m_nodeCount];
        memcpy(m_nodes, that.m_nodes, m_nodeCount * sizeof(CullNodeStruct));
    }

    m_polyCount = that.m_polyCount;

    if (m_polyCount > 0) {
        m_polyIndices = new uint32_t[m_polyCount];
        memcpy(m_polyIndices, that.m_polyIndices, m_polyCount * sizeof(uint32_t));
    }

    m_mesh = that.m_mesh;
    return *this;
}

void AABTreeClass::Reset()
{
    m_nodeCount = 0;

    if (m_nodes) {
        delete[] m_nodes;
        m_nodes = nullptr;
    }

    m_polyCount = 0;

    if (m_polyIndices) {
        delete[] m_polyIndices;
        m_polyIndices = nullptr;
    }

    if (m_mesh) {
        m_mesh = nullptr;
    }
}

void AABTreeClass::Build_Tree_Recursive(AABTreeBuilderClass::CullNodeStruct *node, int &curpolyindex)
{
    CullNodeStruct *newnode = &(m_nodes[node->m_index]);
    newnode->m_min = node->m_min;
    newnode->m_max = node->m_max;

    if (node->m_front != nullptr) {
        captainslog_assert(node->m_back != nullptr);
        newnode->Set_Front_Child(node->m_front->m_index);
        newnode->Set_Back_Child(node->m_back->m_index);
    } else {
        newnode->Set_Poly0(curpolyindex);
        newnode->Set_Poly_Count(node->m_polyCount);
    }

    for (int pcounter = 0; pcounter < node->m_polyCount; pcounter++) {
        m_polyIndices[curpolyindex++] = node->m_polyIndices[pcounter];
    }

    if (node->m_front) {
        Build_Tree_Recursive(node->m_front, curpolyindex);
    }
    if (node->m_back) {
        Build_Tree_Recursive(node->m_back, curpolyindex);
    }
}

void AABTreeClass::Set_Mesh(MeshGeometryClass *mesh)
{
    m_mesh = mesh;
}

void AABTreeClass::Generate_APT(const OBBoxClass &box, SimpleDynVecClass<uint32_t> &apt)
{
    OBBoxAPTContextStruct context(box, apt);
    Generate_OBBox_APT_Recursive(&(m_nodes[0]), context);
}

void AABTreeClass::Generate_OBBox_APT_Recursive(CullNodeStruct *node, OBBoxAPTContextStruct &context)
{
    AABoxClass nodebox;
    nodebox.Init_Min_Max(node->m_min, node->m_max);

    if (!CollisionMath::Intersection_Test(context.m_box, nodebox)) {
        return;
    }

    if (node->Is_Leaf()) {
        int polycount = node->Get_Poly_Count();
        int poly0 = node->Get_Poly0();

        if (polycount > 0) {
            TriClass tri;
            const Vector3 *loc = m_mesh->Get_Vertex_Array();
            const TriIndex *polys = m_mesh->Get_Polygon_Array();

            for (int poly_counter = 0; poly_counter < polycount; poly_counter++) {
                int poly_index = m_polyIndices[poly0 + poly_counter];
                tri.V[0] = &(loc[polys[poly_index][0]]);
                tri.V[1] = &(loc[polys[poly_index][1]]);
                tri.V[2] = &(loc[polys[poly_index][2]]);
                Vector3 normal;
                tri.N = &normal;
                tri.Compute_Normal();

                if (CollisionMath::Intersection_Test(context.m_box, tri)) {
                    context.m_APT.Add(poly_index);
                }
            }
        }
    } else {
        Generate_OBBox_APT_Recursive(&(m_nodes[node->Get_Front_Child()]), context);
        Generate_OBBox_APT_Recursive(&(m_nodes[node->Get_Back_Child()]), context);
    }
}

void AABTreeClass::Generate_APT(const OBBoxClass &box, const Vector3 &viewdir, SimpleDynVecClass<uint32_t> &apt)
{
    OBBoxRayAPTContextStruct context(box, viewdir, apt);
    Generate_OBBox_APT_Recursive(&(m_nodes[0]), context);
}

void AABTreeClass::Generate_OBBox_APT_Recursive(CullNodeStruct *node, OBBoxRayAPTContextStruct &context)
{
    AABoxClass nodebox;
    nodebox.Init_Min_Max(node->m_min, node->m_max);

    if (!CollisionMath::Intersection_Test(context.m_box, nodebox)) {
        return;
    }

    if (node->Is_Leaf()) {
        int polycount = node->Get_Poly_Count();
        int poly0 = node->Get_Poly0();

        if (polycount > 0) {
            TriClass tri;
            const Vector3 *loc = m_mesh->Get_Vertex_Array();
            const TriIndex *polys = m_mesh->Get_Polygon_Array();

            for (int poly_counter = 0; poly_counter < polycount; poly_counter++) {
                int poly_index = m_polyIndices[poly0 + poly_counter];
                tri.V[0] = &(loc[polys[poly_index][0]]);
                tri.V[1] = &(loc[polys[poly_index][1]]);
                tri.V[2] = &(loc[polys[poly_index][2]]);
                Vector3 normal;
                tri.N = &normal;
                tri.Compute_Normal();

                if (Vector3::Dot_Product(*tri.N, context.m_viewVector) < 0.0f) {
                    if (CollisionMath::Intersection_Test(context.m_box, tri)) {
                        context.m_APT.Add(poly_index);
                    }
                }
            }
        }
    } else {
        Generate_OBBox_APT_Recursive(&(m_nodes[node->Get_Front_Child()]), context);
        Generate_OBBox_APT_Recursive(&(m_nodes[node->Get_Back_Child()]), context);
    }
}

bool AABTreeClass::Cast_Ray_Recursive(CullNodeStruct *node, RayCollisionTestClass &raytest)
{
    if (raytest.Cull(node->m_min, node->m_max)) {
        return false;
    }

    bool res = false;

    if (node->Is_Leaf()) {
        return Cast_Ray_To_Polys(node, raytest);
    } else {
        res |= Cast_Ray_Recursive(&(m_nodes[node->Get_Front_Child()]), raytest);
        res |= Cast_Ray_Recursive(&(m_nodes[node->Get_Back_Child()]), raytest);
    }

    return res;
}

int AABTreeClass::Cast_Semi_Infinite_Axis_Aligned_Ray_Recursive(CullNodeStruct *node,
    const Vector3 &start_point,
    int axis_r,
    int axis_1,
    int axis_2,
    int direction,
    unsigned char &flags)
{
    constexpr float sign[2] = { -1.0f, 1.0f };
    float bounds[2];
    float start[2];
    bounds[0] = node->m_min[axis_r] * sign[0];
    bounds[1] = node->m_max[axis_r] * sign[1];
    start[0] = start_point[axis_r] * sign[0];
    start[1] = start_point[axis_r] * sign[1];

    if (start_point[axis_1] < node->m_min[axis_1] || start_point[axis_2] < node->m_min[axis_2]
        || start_point[axis_1] > node->m_max[axis_1] || start_point[axis_2] > node->m_max[axis_2]
        || start[direction] > bounds[direction]) {
        return 0;
    }

    int count = 0;

    if (node->Is_Leaf()) {
        return Cast_Semi_Infinite_Axis_Aligned_Ray_To_Polys(node, start_point, axis_r, axis_1, axis_2, direction, flags);
    } else {
        count += Cast_Semi_Infinite_Axis_Aligned_Ray_Recursive(
            &(m_nodes[node->Get_Front_Child()]), start_point, axis_r, axis_1, axis_2, direction, flags);
        count += Cast_Semi_Infinite_Axis_Aligned_Ray_Recursive(
            &(m_nodes[node->Get_Back_Child()]), start_point, axis_r, axis_1, axis_2, direction, flags);
    }

    return count;
}

bool AABTreeClass::Cast_AABox_Recursive(CullNodeStruct *node, AABoxCollisionTestClass &boxtest)
{
    if (boxtest.Cull(node->m_min, node->m_max)) {
        return false;
    }

    bool res = false;

    if (node->Is_Leaf()) {
        return Cast_AABox_To_Polys(node, boxtest);
    } else {
        res |= Cast_AABox_Recursive(&(m_nodes[node->Get_Front_Child()]), boxtest);
        res |= Cast_AABox_Recursive(&(m_nodes[node->Get_Back_Child()]), boxtest);
    }

    return res;
}

bool AABTreeClass::Cast_OBBox_Recursive(CullNodeStruct *node, OBBoxCollisionTestClass &boxtest)
{
    if (boxtest.Cull(node->m_min, node->m_max)) {
        return false;
    }

    bool res = false;

    if (node->Is_Leaf()) {
        return Cast_OBBox_To_Polys(node, boxtest);
    } else {
        res |= Cast_OBBox_Recursive(&(m_nodes[node->Get_Front_Child()]), boxtest);
        res |= Cast_OBBox_Recursive(&(m_nodes[node->Get_Back_Child()]), boxtest);
    }

    return res;
}

bool AABTreeClass::Intersect_OBBox_Recursive(AABTreeClass::CullNodeStruct *node, OBBoxIntersectionTestClass &test)
{
    if (test.Cull(node->m_min, node->m_max)) {
        return false;
    }

    bool res = false;

    if (node->Is_Leaf()) {
        return Intersect_OBBox_With_Polys(node, test);
    } else {
        res |= Intersect_OBBox_Recursive(&(m_nodes[node->Get_Front_Child()]), test);
        res |= Intersect_OBBox_Recursive(&(m_nodes[node->Get_Back_Child()]), test);
    }

    return res;
}

bool AABTreeClass::Cast_Ray_To_Polys(CullNodeStruct *node, RayCollisionTestClass &raytest)
{
    if (node->Get_Poly_Count() > 0) {
        TriClass tri;

        const Vector3 *loc = m_mesh->Get_Vertex_Array();
        const TriIndex *polyverts = m_mesh->Get_Polygon_Array();

        int polyhit = -1;
        int poly0 = node->Get_Poly0();
        int polycount = node->Get_Poly_Count();

        for (int poly_counter = 0; poly_counter < polycount; poly_counter++) {
            int poly_index = m_polyIndices[poly0 + poly_counter];
            tri.V[0] = &(loc[polyverts[poly_index][0]]);
            tri.V[1] = &(loc[polyverts[poly_index][1]]);
            tri.V[2] = &(loc[polyverts[poly_index][2]]);
            Vector3 normal;
            tri.N = &normal;
            tri.Compute_Normal();

            if (CollisionMath::Collide(raytest.m_ray, tri, raytest.m_result)) {
                polyhit = poly_index;
            }

            if (raytest.m_result->start_bad) {
                return true;
            }
        }

        if (polyhit != -1) {
            raytest.m_result->surface_type = m_mesh->Get_Poly_Surface_Type(polyhit);
            return true;
        }
    }

    return false;
}

int AABTreeClass::Cast_Semi_Infinite_Axis_Aligned_Ray_To_Polys(CullNodeStruct *node,
    const Vector3 &start_point,
    int axis_r,
    int axis_1,
    int axis_2,
    int direction,
    unsigned char &flags)
{
    int count = 0;

    if (node->Get_Poly_Count() > 0) {
        const Vector3 *loc = m_mesh->Get_Vertex_Array();
        const TriIndex *polyverts = m_mesh->Get_Polygon_Array();
        const Vector4 *plane = m_mesh->Get_Plane_Array();
        int poly0 = node->Get_Poly0();
        int polycount = node->Get_Poly_Count();

        for (int poly_counter = 0; poly_counter < polycount; poly_counter++) {
            int poly_index = m_polyIndices[poly0 + poly_counter];
            const Vector3 &v0 = loc[polyverts[poly_index][0]];
            const Vector3 &v1 = loc[polyverts[poly_index][1]];
            const Vector3 &v2 = loc[polyverts[poly_index][2]];
            const Vector4 &tri_plane = plane[poly_index];
            count += (unsigned int)Cast_Semi_Infinite_Axis_Aligned_Ray_To_Triangle(
                v0, v1, v2, tri_plane, start_point, axis_r, axis_1, axis_2, direction, flags);
        }
    }

    return count;
}

bool AABTreeClass::Cast_AABox_To_Polys(CullNodeStruct *node, AABoxCollisionTestClass &boxtest)
{
    int polycount = node->Get_Poly_Count();

    if (polycount > 0) {
        TriClass tri;

        const Vector3 *loc = m_mesh->Get_Vertex_Array();
        const TriIndex *polyverts = m_mesh->Get_Polygon_Array();

        int polyhit = -1;
        int poly0 = node->Get_Poly0();
        int polycount2 = node->Get_Poly_Count();

        for (int poly_counter = 0; poly_counter < polycount2; poly_counter++) {
            int poly_index = m_polyIndices[poly0 + poly_counter];
            tri.V[0] = &(loc[polyverts[poly_index][0]]);
            tri.V[1] = &(loc[polyverts[poly_index][1]]);
            tri.V[2] = &(loc[polyverts[poly_index][2]]);
            Vector3 normal;
            tri.N = &normal;
            tri.Compute_Normal();

            if (CollisionMath::Collide(boxtest.m_box, boxtest.m_move, tri, boxtest.m_result)) {
                polyhit = poly_index;
            }

            if (boxtest.m_result->start_bad) {
                return true;
            }
        }

        if (polyhit != -1) {
            boxtest.m_result->surface_type = m_mesh->Get_Poly_Surface_Type(polyhit);
            return true;
        }
    }
    return false;
}

bool AABTreeClass::Cast_OBBox_To_Polys(CullNodeStruct *node, OBBoxCollisionTestClass &boxtest)
{
    int poly0 = node->Get_Poly0();
    int polycount = node->Get_Poly_Count();

    if (polycount > 0) {
        TriClass tri;

        const Vector3 *loc = m_mesh->Get_Vertex_Array();
        const TriIndex *polyverts = m_mesh->Get_Polygon_Array();

        int polyhit = -1;

        for (int poly_counter = 0; poly_counter < polycount; poly_counter++) {
            int poly_index = m_polyIndices[poly0 + poly_counter];
            tri.V[0] = &(loc[polyverts[poly_index][0]]);
            tri.V[1] = &(loc[polyverts[poly_index][1]]);
            tri.V[2] = &(loc[polyverts[poly_index][2]]);
            Vector3 normal;
            tri.N = &normal;
            tri.Compute_Normal();

            if (CollisionMath::Collide(boxtest.m_box, boxtest.m_move, tri, Vector3(0, 0, 0), boxtest.m_result)) {
                polyhit = poly_index;
            }

            if (boxtest.m_result->start_bad) {
                return true;
            }
        }

        if (polyhit != -1) {
            boxtest.m_result->surface_type = m_mesh->Get_Poly_Surface_Type(polyhit);
            return true;
        }
    }

    return false;
}

bool AABTreeClass::Intersect_OBBox_With_Polys(CullNodeStruct *node, OBBoxIntersectionTestClass &test)
{
    int poly0 = node->Get_Poly0();
    int polycount = node->Get_Poly_Count();

    if (polycount > 0) {
        TriClass tri;

        const Vector3 *loc = m_mesh->Get_Vertex_Array();
        const TriIndex *polyverts = m_mesh->Get_Polygon_Array();

        for (int poly_counter = 0; poly_counter < polycount; poly_counter++) {
            int poly_index = m_polyIndices[poly0 + poly_counter];
            tri.V[0] = &(loc[polyverts[poly_index][0]]);
            tri.V[1] = &(loc[polyverts[poly_index][1]]);
            tri.V[2] = &(loc[polyverts[poly_index][2]]);
            Vector3 normal;
            tri.N = &normal;
            tri.Compute_Normal();

            if (CollisionMath::Intersection_Test(test.m_box, tri)) {
                return true;
            }
        }
    }

    return false;
}

void AABTreeClass::Update_Bounding_Boxes_Recursive(CullNodeStruct *node)
{
    node->m_min.Set(100000.0f, 100000.0f, 100000.0f);
    node->m_max.Set(-100000.0f, -100000.0f, -100000.0f);

    if (node->Is_Leaf() == false) {

        Update_Bounding_Boxes_Recursive(&(m_nodes[node->Get_Front_Child()]));
        Update_Bounding_Boxes_Recursive(&(m_nodes[node->Get_Back_Child()]));

        int front = node->Get_Front_Child();
        int back = node->Get_Back_Child();

        if (m_nodes[front].m_min.X < node->m_min.X) {
            node->m_min.X = m_nodes[front].m_min.X;
        }

        if (m_nodes[front].m_max.X > node->m_max.X) {
            node->m_max.X = m_nodes[front].m_max.X;
        }

        if (m_nodes[front].m_min.Y < node->m_min.Y) {
            node->m_min.Y = m_nodes[front].m_min.Y;
        }

        if (m_nodes[front].m_max.Y > node->m_max.Y) {
            node->m_max.Y = m_nodes[front].m_max.Y;
        }

        if (m_nodes[front].m_min.Z < node->m_min.Z) {
            node->m_min.Z = m_nodes[front].m_min.Z;
        }

        if (m_nodes[front].m_max.Z > node->m_max.Z) {
            node->m_max.Z = m_nodes[front].m_max.Z;
        }

        if (m_nodes[back].m_min.X < node->m_min.X) {
            node->m_min.X = m_nodes[back].m_min.X;
        }

        if (m_nodes[back].m_max.X > node->m_max.X) {
            node->m_max.X = m_nodes[back].m_max.X;
        }

        if (m_nodes[back].m_min.Y < node->m_min.Y) {
            node->m_min.Y = m_nodes[back].m_min.Y;
        }

        if (m_nodes[back].m_max.Y > node->m_max.Y) {
            node->m_max.Y = m_nodes[back].m_max.Y;
        }

        if (m_nodes[back].m_min.Z < node->m_min.Z) {
            node->m_min.Z = m_nodes[back].m_min.Z;
        }

        if (m_nodes[back].m_max.Z > node->m_max.Z) {
            node->m_max.Z = m_nodes[back].m_max.Z;
        }
    } else {
        int poly0 = node->Get_Poly0();
        int polycount = node->Get_Poly_Count();

        for (int poly_index = 0; poly_index < polycount; poly_index++) {
            int pi = m_polyIndices[poly0 + poly_index];
            Update_Min_Max(pi, node->m_min, node->m_max);
        }
    }

    captainslog_assert(node->m_min.X != 100000.0f);
    captainslog_assert(node->m_min.Y != 100000.0f);
    captainslog_assert(node->m_min.Z != 100000.0f);
    captainslog_assert(node->m_max.X != -100000.0f);
    captainslog_assert(node->m_max.Y != -100000.0f);
    captainslog_assert(node->m_max.Z != -100000.0f);
}

void AABTreeClass::Update_Min_Max(int poly_index, Vector3 &min, Vector3 &max)
{
    for (int vert_index = 0; vert_index < 3; vert_index++) {

        const TriIndex *polyverts = m_mesh->Get_Polygon_Array() + poly_index;
        const Vector3 *point = m_mesh->Get_Vertex_Array() + (*polyverts)[vert_index];

        if (point->X < min.X) {
            min.X = point->X;
        }

        if (point->Y < min.Y) {
            min.Y = point->Y;
        }

        if (point->Z < min.Z) {
            min.Z = point->Z;
        }

        if (point->X > max.X) {
            max.X = point->X;
        }

        if (point->Y > max.Y) {
            max.Y = point->Y;
        }

        if (point->Z > max.Z) {
            max.Z = point->Z;
        }
    }
}

void AABTreeClass::Load_W3D(ChunkLoadClass &cload)
{
    Reset();

    W3dMeshAABTreeHeader header;
    cload.Open_Chunk();
    captainslog_assert(cload.Cur_Chunk_ID() == W3D_CHUNK_AABTREE_HEADER);
    cload.Read(&header, sizeof(header));
    cload.Close_Chunk();

    m_nodeCount = header.NodeCount;
    m_polyCount = header.PolyCount;
    m_nodes = new CullNodeStruct[m_nodeCount];
    m_polyIndices = new uint32_t[m_polyCount];

    while (cload.Open_Chunk()) {
        switch (cload.Cur_Chunk_ID()) {
            case W3D_CHUNK_AABTREE_POLYINDICES:
                Read_Poly_Indices(cload);
                break;
            case W3D_CHUNK_AABTREE_NODES:
                Read_Nodes(cload);
                break;
        }

        cload.Close_Chunk();
    }
}

void AABTreeClass::Read_Poly_Indices(ChunkLoadClass &cload)
{
    cload.Read(m_polyIndices, sizeof(uint32_t) * m_polyCount);
}

void AABTreeClass::Read_Nodes(ChunkLoadClass &cload)
{
    W3dMeshAABTreeNode w3dnode;

    for (int i = 0; i < m_nodeCount; i++) {
        cload.Read(&w3dnode, sizeof(w3dnode));

        m_nodes[i].m_min.X = w3dnode.Min.x;
        m_nodes[i].m_min.Y = w3dnode.Min.y;
        m_nodes[i].m_min.Z = w3dnode.Min.z;

        m_nodes[i].m_max.X = w3dnode.Max.x;
        m_nodes[i].m_max.Y = w3dnode.Max.y;
        m_nodes[i].m_max.Z = w3dnode.Max.z;

        m_nodes[i].m_frontOrPoly0 = w3dnode.FrontOrPoly0;
        m_nodes[i].m_backOrPolyCount = w3dnode.BackOrPolyCount;
    }
}

void AABTreeClass::Scale(float scale)
{
    for (int i = 0; i < m_nodeCount; i++) {
        m_nodes[i].m_min *= scale;
        m_nodes[i].m_max *= scale;
    }
}
