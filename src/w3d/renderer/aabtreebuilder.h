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
#pragma once
#include "always.h"
#include "aaplane.h"
#include "bittype.h"
#include "meshgeometry.h"
#include "vector3.h"
#include "vector3i.h"

class AABTreeClass;
class ChunkSaveClass;
struct W3dMeshAABTreeNode;

#define AABTREE_LEAF_FLAG 0x80000000

class AABTreeBuilderClass
{
public:
    AABTreeBuilderClass();
    ~AABTreeBuilderClass();

    void Build_AABTree(int32_t polycount, TriIndex *polys, int32_t vertcount, Vector3 *verts);
    void Build_AABTree(int32_t polycount, Vector3i *polys, int32_t vertcount, Vector3 *verts);
    void Export(ChunkSaveClass &csave);

    int32_t Node_Count();
    int32_t Poly_Count();

    enum
    {
        MIN_POLYS_PER_NODE = 4,
        SMALL_VERTEX = -100000,
        BIG_VERTEX = 100000
    };

private:
    struct CullNodeStruct
    {
        CullNodeStruct() :
            m_index(0),
            m_min(0, 0, 0),
            m_max(0, 0, 0),
            m_front(nullptr),
            m_back(nullptr),
            m_polyCount(0),
            m_polyIndices(nullptr)
        {
        }
        ~CullNodeStruct()
        {
            if (m_front) {
                delete m_front;
            }

            if (m_back) {
                delete m_back;
            }

            if (m_polyIndices) {
                delete[] m_polyIndices;
            }
        }

        int32_t m_index;
        Vector3 m_min;
        Vector3 m_max;
        CullNodeStruct *m_front;
        CullNodeStruct *m_back;
        int32_t m_polyCount;
        int32_t *m_polyIndices;
    };

    struct SplitChoiceStruct
    {
        SplitChoiceStruct() :
            m_cost(FLT_MAX),
            m_frontCount(0),
            m_backCount(0),
            m_bMin(BIG_VERTEX, BIG_VERTEX, BIG_VERTEX),
            m_bMax(SMALL_VERTEX, SMALL_VERTEX, SMALL_VERTEX),
            m_fMin(BIG_VERTEX, BIG_VERTEX, BIG_VERTEX),
            m_fMax(SMALL_VERTEX, SMALL_VERTEX, SMALL_VERTEX),
            m_plane(AAPlaneClass::XNORMAL, 0)
        {
        }

        float m_cost;
        int32_t m_frontCount;
        int32_t m_backCount;
        Vector3 m_bMin;
        Vector3 m_bMax;
        Vector3 m_fMin;
        Vector3 m_fMax;
        AAPlaneClass m_plane;
    };

    struct SplitArraysStruct
    {
        SplitArraysStruct() : m_frontCount(0), m_backCount(0), m_frontPolys(nullptr), m_backPolys(nullptr) {}

        int32_t m_frontCount;
        int32_t m_backCount;
        int32_t *m_frontPolys;
        int32_t *m_backPolys;
    };

    enum OverlapType
    {
        POS = 0x01,
        NEG = 0x02,
        ON = 0x04,
        BOTH = 0x08,
        OUTSIDE = POS,
        INSIDE = NEG,
        OVERLAPPED = BOTH,
        FRONT = POS,
        BACK = NEG,
    };

    void Reset();
    void Build_Tree(CullNodeStruct *node, int32_t polycount, int32_t *polyindices);
    SplitChoiceStruct Select_Splitting_Plane(int32_t polycount, int32_t *polyindices);
    SplitChoiceStruct Compute_Plane_Score(int32_t polycont, int32_t *polyindices, const AAPlaneClass &plane);
    void Split_Polys(int32_t polycount, int32_t *polyindices, const SplitChoiceStruct &sc, SplitArraysStruct *arrays);
    OverlapType Which_Side(const AAPlaneClass &plane, int32_t poly_index);
    void Compute_Bounding_Box(CullNodeStruct *node);
    int32_t Assign_Index(CullNodeStruct *node, int32_t index);
    int32_t Node_Count_Recursive(CullNodeStruct *node, int32_t curcount);
    void Update_Min(int32_t poly_index, Vector3 &set_min);
    void Update_Max(int32_t poly_index, Vector3 &set_max);
    void Update_Min_Max(int32_t poly_index, Vector3 &set_min, Vector3 &set_max);

    void Build_W3D_AABTree_Recursive(
        CullNodeStruct *node, W3dMeshAABTreeNode *w3dnodes, uint32_t *poly_indices, int32_t &cur_node, int32_t &cur_poly);
    CullNodeStruct *m_root;
    int32_t m_curPolyIndex;

    int32_t m_polyCount;
    TriIndex *m_polys;
    int32_t m_vertCount;
    Vector3 *m_verts;

    friend class AABTreeClass;
};
