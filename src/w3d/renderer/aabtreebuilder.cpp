/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief aabtree builder
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "aabtreebuilder.h"
#include "chunkio.h"
#include "w3d_file.h"

const float COINCIDENCE_EPSILON = 0.001f;

AABTreeBuilderClass::AABTreeBuilderClass() :
    m_root(nullptr), m_curPolyIndex(0), m_polyCount(0), m_polys(nullptr), m_vertCount(0), m_verts(nullptr)
{
}

AABTreeBuilderClass::~AABTreeBuilderClass()
{
    Reset();
}

void AABTreeBuilderClass::Reset()
{
    if (m_root) {
        delete m_root;
        m_root = nullptr;
    }

    if (m_verts != nullptr) {
        delete[] m_verts;
        m_verts = nullptr;
    }

    if (m_polys != nullptr) {
        delete[] m_polys;
        m_polys = nullptr;
    }
}

void AABTreeBuilderClass::Build_AABTree(int polycount, TriIndex *polys, int vertcount, Vector3 *verts)
{
    captainslog_assert(polycount > 0);
    captainslog_assert(vertcount > 0);
    captainslog_assert(polys != nullptr);
    captainslog_assert(verts != nullptr);

    Reset();

    m_vertCount = vertcount;
    m_polyCount = polycount;
    m_verts = new Vector3[m_vertCount];
    m_polys = new TriIndex[m_polyCount];

    for (int vi = 0; vi < m_vertCount; vi++) {
        m_verts[vi] = verts[vi];
    }

    for (int pi = 0; pi < m_polyCount; pi++) {
        m_polys[pi] = polys[pi];
    }

    int *polyindices = new int[m_polyCount];

    for (int i = 0; i < m_polyCount; i++) {
        polyindices[i] = i;
    }

    m_root = new CullNodeStruct;
    Build_Tree(m_root, m_polyCount, polyindices);
    polyindices = nullptr;

    Compute_Bounding_Box(m_root);
    Assign_Index(m_root, 0);
}

void AABTreeBuilderClass::Build_AABTree(int polycount, Vector3i *polys, int vertcount, Vector3 *verts)
{
    captainslog_assert(polycount > 0);
    captainslog_assert(vertcount > 0);
    captainslog_assert(polys != nullptr);
    captainslog_assert(verts != nullptr);

    Reset();

    m_vertCount = vertcount;
    m_polyCount = polycount;
    m_verts = new Vector3[m_vertCount];
    m_polys = new TriIndex[m_polyCount];

    for (int vi = 0; vi < m_vertCount; vi++) {
        m_verts[vi] = verts[vi];
    }

    for (int pi = 0; pi < m_polyCount; pi++) {
        m_polys[pi].I = polys[pi].I;
        m_polys[pi].J = polys[pi].J;
        m_polys[pi].K = polys[pi].K;
    }

    int *polyindices = new int[m_polyCount];

    for (int i = 0; i < m_polyCount; i++) {
        polyindices[i] = i;
    }

    m_root = new CullNodeStruct;
    Build_Tree(m_root, m_polyCount, polyindices);
    polyindices = nullptr;

    Compute_Bounding_Box(m_root);
    Assign_Index(m_root, 0);
}

void AABTreeBuilderClass::Build_Tree(CullNodeStruct *node, int polycount, int *polyindices)
{
    if (polycount <= MIN_POLYS_PER_NODE) {
        node->m_polyCount = polycount;
        node->m_polyIndices = polyindices;
        return;
    }

    SplitChoiceStruct sc;

    sc = Select_Splitting_Plane(polycount, polyindices);

    if (sc.m_frontCount + sc.m_backCount != polycount) {
        node->m_polyCount = polycount;
        node->m_polyIndices = polyindices;
        return;
    }

    SplitArraysStruct arrays;
    Split_Polys(polycount, polyindices, sc, &arrays);

    delete[] polyindices;

    if (arrays.m_frontCount) {
        captainslog_assert(arrays.m_frontPolys != nullptr);
        node->m_front = new CullNodeStruct;
        Build_Tree(node->m_front, arrays.m_frontCount, arrays.m_frontPolys);
        arrays.m_frontPolys = nullptr;
    }

    if (arrays.m_backCount) {
        captainslog_assert(arrays.m_backPolys != nullptr);
        node->m_back = new CullNodeStruct;
        Build_Tree(node->m_back, arrays.m_backCount, arrays.m_backPolys);
        arrays.m_backPolys = nullptr;
    }
}

AABTreeBuilderClass::SplitChoiceStruct AABTreeBuilderClass::Select_Splitting_Plane(int polycount, int *polyindices)
{
    captainslog_assert(polyindices != nullptr);

    const int NUM_TRYS = 50;

    SplitChoiceStruct best_plane_stats;
    SplitChoiceStruct considered_plane_stats;

    for (int trys = 0; trys < GameMath::Min(NUM_TRYS, polycount); trys++) {
        AAPlaneClass plane;

        int poly_index = polyindices[rand() % polycount];
        int vert_index = rand() % 3;
        const TriIndex *polyverts = m_polys + poly_index;
        const Vector3 *vert = m_verts + (*polyverts)[vert_index];

        switch (rand() % 3) {
            case 0:
                plane.Set(AAPlaneClass::XNORMAL, vert->X);
                break;
            case 1:
                plane.Set(AAPlaneClass::YNORMAL, vert->Y);
                break;
            case 2:
                plane.Set(AAPlaneClass::ZNORMAL, vert->Z);
                break;
        };

        considered_plane_stats = Compute_Plane_Score(polycount, polyindices, plane);

        if (considered_plane_stats.m_cost < best_plane_stats.m_cost) {
            best_plane_stats = considered_plane_stats;
        }
    }

    return best_plane_stats;
}

AABTreeBuilderClass::SplitChoiceStruct AABTreeBuilderClass::Compute_Plane_Score(
    int polycount, int *polyindices, const AAPlaneClass &plane)
{
    SplitChoiceStruct sc;
    sc.m_plane = plane;

    for (int i = 0; i < polycount; i++) {
        switch (Which_Side(plane, polyindices[i])) {
            case FRONT:
            case ON:
            case BOTH: {
                sc.m_frontCount++;
                Update_Min_Max(polyindices[i], sc.m_fMin, sc.m_fMax);
                break;
            }
            case BACK: {
                sc.m_backCount++;
                Update_Min_Max(polyindices[i], sc.m_bMin, sc.m_bMax);
                break;
            }
        }
    }

    sc.m_bMin -= Vector3(GAMEMATH_EPSILON, GAMEMATH_EPSILON, GAMEMATH_EPSILON);
    sc.m_bMax += Vector3(GAMEMATH_EPSILON, GAMEMATH_EPSILON, GAMEMATH_EPSILON);

    float back_cost =
        (sc.m_bMax.X - sc.m_bMin.X) * (sc.m_bMax.Y - sc.m_bMin.Y) * (sc.m_bMax.Z - sc.m_bMin.Z) * sc.m_backCount;
    float front_cost =
        (sc.m_fMax.X - sc.m_fMin.X) * (sc.m_fMax.Y - sc.m_fMin.Y) * (sc.m_fMax.Z - sc.m_fMin.Z) * sc.m_frontCount;
    sc.m_cost = front_cost + back_cost;

    if ((sc.m_frontCount == 0) || (sc.m_backCount == 0)) {
        sc.m_cost = FLT_MAX;
    }

    return sc;
}

AABTreeBuilderClass::OverlapType AABTreeBuilderClass::Which_Side(const AAPlaneClass &plane, int poly_index)
{
    int mask = 0;
    for (int vi = 0; vi < 3; vi++) {

        const Vector3 &point = m_verts[m_polys[poly_index][vi]];
        float delta = point[plane.m_normal] - plane.m_dist;

        if (delta > COINCIDENCE_EPSILON) {
            mask |= FRONT;
        }

        if (delta < -COINCIDENCE_EPSILON) {
            mask |= BACK;
        }

        mask |= ON;
    }

    if (mask == ON) {
        return ON;
    }

    if ((mask & ~(FRONT | ON)) == 0) {
        return FRONT;
    }

    if ((mask & ~(BACK | ON)) == 0) {
        return BACK;
    }

    return BOTH;
}

void AABTreeBuilderClass::Split_Polys(
    int polycount, int *polyindices, const SplitChoiceStruct &sc, SplitArraysStruct *arrays)
{
    if (sc.m_frontCount > 0) {
        arrays->m_frontPolys = new int[sc.m_frontCount];
    }

    if (sc.m_backCount > 0) {
        arrays->m_backPolys = new int[sc.m_backCount];
    }

    arrays->m_frontCount = 0;
    arrays->m_backCount = 0;

    for (int i = 0; i < polycount; i++) {
        switch (Which_Side(sc.m_plane, polyindices[i])) {
            case FRONT:
            case ON:
            case BOTH:
                arrays->m_frontPolys[arrays->m_frontCount++] = polyindices[i];
                break;
            case BACK:
                arrays->m_backPolys[arrays->m_backCount++] = polyindices[i];
                break;
        }
    }

    captainslog_assert(arrays->m_frontCount == sc.m_frontCount);
    captainslog_assert(arrays->m_backCount == sc.m_backCount);
}

void AABTreeBuilderClass::Compute_Bounding_Box(CullNodeStruct *node)
{
    if (node->m_front) {
        Compute_Bounding_Box(node->m_front);
    }

    if (node->m_back) {
        Compute_Bounding_Box(node->m_back);
    }

    const float really_big = GAMEMATH_FLOAT_MAX;

    node->m_min.Set(really_big, really_big, really_big);
    node->m_max.Set(-really_big, -really_big, -really_big);

    for (int poly_index = 0; poly_index < node->m_polyCount; poly_index++) {
        Update_Min_Max(node->m_polyIndices[poly_index], node->m_min, node->m_max);
    }

    if (node->m_front) {
        if (node->m_front->m_min.X < node->m_min.X) {
            node->m_min.X = node->m_front->m_min.X;
        }

        if (node->m_front->m_max.X > node->m_max.X) {
            node->m_max.X = node->m_front->m_max.X;
        }

        if (node->m_front->m_min.Y < node->m_min.Y) {
            node->m_min.Y = node->m_front->m_min.Y;
        }

        if (node->m_front->m_max.Y > node->m_max.Y) {
            node->m_max.Y = node->m_front->m_max.Y;
        }

        if (node->m_front->m_min.Z < node->m_min.Z) {
            node->m_min.Z = node->m_front->m_min.Z;
        }

        if (node->m_front->m_max.Z > node->m_max.Z) {
            node->m_max.Z = node->m_front->m_max.Z;
        }
    }

    if (node->m_back) {
        if (node->m_back->m_min.X < node->m_min.X) {
            node->m_min.X = node->m_back->m_min.X;
        }

        if (node->m_back->m_max.X > node->m_max.X) {
            node->m_max.X = node->m_back->m_max.X;
        }

        if (node->m_back->m_min.Y < node->m_min.Y) {
            node->m_min.Y = node->m_back->m_min.Y;
        }

        if (node->m_back->m_max.Y > node->m_max.Y) {
            node->m_max.Y = node->m_back->m_max.Y;
        }

        if (node->m_back->m_min.Z < node->m_min.Z) {
            node->m_min.Z = node->m_back->m_min.Z;
        }

        if (node->m_back->m_max.Z > node->m_max.Z) {
            node->m_max.Z = node->m_back->m_max.Z;
        }
    }

    captainslog_assert(node->m_min.X != really_big);
    captainslog_assert(node->m_min.Y != really_big);
    captainslog_assert(node->m_min.Z != really_big);
    captainslog_assert(node->m_max.X != -really_big);
    captainslog_assert(node->m_max.Y != -really_big);
    captainslog_assert(node->m_max.Z != -really_big);
}

int AABTreeBuilderClass::Assign_Index(CullNodeStruct *node, int index)
{
    captainslog_assert(node);
    node->m_index = index;
    index++;

    if (node->m_front) {
        index = Assign_Index(node->m_front, index);
    }

    if (node->m_back) {
        index = Assign_Index(node->m_back, index);
    }

    return index;
}

int AABTreeBuilderClass::Node_Count()
{
    if (m_root) {
        return Node_Count_Recursive(m_root, 0);
    } else {
        return 0;
    }
}

int AABTreeBuilderClass::Poly_Count()
{
    return m_polyCount;
}

int AABTreeBuilderClass::Node_Count_Recursive(CullNodeStruct *node, int curcount)
{
    curcount++;

    if (node->m_front) {
        curcount = Node_Count_Recursive(node->m_front, curcount);
    }

    if (node->m_back) {
        curcount = Node_Count_Recursive(node->m_back, curcount);
    }

    return curcount;
}

void AABTreeBuilderClass::Update_Min(int poly_index, Vector3 &min)
{
    for (int vert_index = 0; vert_index < 3; vert_index++) {

        const TriIndex *polyverts = m_polys + poly_index;
        const Vector3 *point = m_verts + (*polyverts)[vert_index];

        if (point->X < min.X) {
            min.X = point->X;
        }

        if (point->Y < min.Y) {
            min.Y = point->Y;
        }

        if (point->Z < min.Z) {
            min.Z = point->Z;
        }
    }
}

void AABTreeBuilderClass::Update_Max(int poly_index, Vector3 &max)
{
    for (int vert_index = 0; vert_index < 3; vert_index++) {

        const TriIndex *polyverts = m_polys + poly_index;
        const Vector3 *point = m_verts + (*polyverts)[vert_index];

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

void AABTreeBuilderClass::Update_Min_Max(int poly_index, Vector3 &min, Vector3 &max)
{
    for (int vert_index = 0; vert_index < 3; vert_index++) {

        const TriIndex *polyverts = m_polys + poly_index;
        const Vector3 *point = m_verts + (*polyverts)[vert_index];

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

void AABTreeBuilderClass::Export(ChunkSaveClass &csave)
{
    csave.Begin_Chunk(W3D_CHUNK_AABTREE);

    W3dMeshAABTreeNode *nodes = new W3dMeshAABTreeNode[Node_Count()];
    uint32_t *poly_indices = new uint32_t[Poly_Count()];

    int cur_node = 0;
    int cur_poly = 0;
    Build_W3D_AABTree_Recursive(m_root, nodes, poly_indices, cur_node, cur_poly);

    csave.Begin_Chunk(W3D_CHUNK_AABTREE_HEADER);
    W3dMeshAABTreeHeader header;
    memset(&header, 0, sizeof(header));
    header.NodeCount = Node_Count();
    header.PolyCount = Poly_Count();
    csave.Write(&header, sizeof(header));
    csave.End_Chunk();

    csave.Begin_Chunk(W3D_CHUNK_AABTREE_POLYINDICES);
    csave.Write(poly_indices, Poly_Count() * sizeof(uint32_t));
    csave.End_Chunk();

    csave.Begin_Chunk(W3D_CHUNK_AABTREE_NODES);

    for (int ni = 0; ni < Node_Count(); ni++) {
        csave.Write(&(nodes[ni]), sizeof(W3dMeshAABTreeNode));
    }

    csave.End_Chunk();

    csave.End_Chunk();
}

void AABTreeBuilderClass::Build_W3D_AABTree_Recursive(AABTreeBuilderClass::CullNodeStruct *node,
    W3dMeshAABTreeNode *w3d_nodes,
    uint32_t *poly_indices,
    int &cur_node,
    int &cur_poly)
{
    W3dMeshAABTreeNode *newnode = &(w3d_nodes[node->m_index]);
    newnode->Min.x = node->m_min.X;
    newnode->Min.y = node->m_min.Y;
    newnode->Min.z = node->m_min.Z;
    newnode->Max.x = node->m_max.X;
    newnode->Max.y = node->m_max.Y;
    newnode->Max.z = node->m_max.Z;

    if (node->m_front != nullptr) {
        captainslog_assert(node->m_back != nullptr);
        newnode->FrontOrPoly0 = node->m_front->m_index;
        newnode->BackOrPolyCount = node->m_back->m_index;
    } else {
        newnode->FrontOrPoly0 = cur_poly | AABTREE_LEAF_FLAG;
        newnode->BackOrPolyCount = node->m_polyCount;
    }

    for (int pcounter = 0; pcounter < node->m_polyCount; pcounter++) {
        poly_indices[cur_poly++] = node->m_polyIndices[pcounter];
    }

    if (node->m_front) {
        Build_W3D_AABTree_Recursive(node->m_front, w3d_nodes, poly_indices, cur_node, cur_poly);
    }
    if (node->m_back) {
        Build_W3D_AABTree_Recursive(node->m_back, w3d_nodes, poly_indices, cur_node, cur_poly);
    }
}
