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

    void Build_AABTree(int polycount, TriIndex *polys, int vertcount, Vector3 *verts);
    void Build_AABTree(int polycount, Vector3i *polys, int vertcount, Vector3 *verts);
    void Export(ChunkSaveClass &csave);

    int Node_Count();
    int Poly_Count();

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

        int m_index;
        Vector3 m_min;
        Vector3 m_max;
        CullNodeStruct *m_front;
        CullNodeStruct *m_back;
        int m_polyCount;
        int *m_polyIndices;
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
        int m_frontCount;
        int m_backCount;
        Vector3 m_bMin;
        Vector3 m_bMax;
        Vector3 m_fMin;
        Vector3 m_fMax;
        AAPlaneClass m_plane;
    };

    struct SplitArraysStruct
    {
        SplitArraysStruct() : m_frontCount(0), m_backCount(0), m_frontPolys(nullptr), m_backPolys(nullptr) {}

        int m_frontCount;
        int m_backCount;
        int *m_frontPolys;
        int *m_backPolys;
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
    void Build_Tree(CullNodeStruct *node, int polycount, int *polyindices);
    SplitChoiceStruct Select_Splitting_Plane(int polycount, int *polyindices);
    SplitChoiceStruct Compute_Plane_Score(int polycont, int *polyindices, const AAPlaneClass &plane);
    void Split_Polys(int polycount, int *polyindices, const SplitChoiceStruct &sc, SplitArraysStruct *arrays);
    OverlapType Which_Side(const AAPlaneClass &plane, int poly_index);
    void Compute_Bounding_Box(CullNodeStruct *node);
    int Assign_Index(CullNodeStruct *node, int index);
    int Node_Count_Recursive(CullNodeStruct *node, int curcount);
    void Update_Min(int poly_index, Vector3 &set_min);
    void Update_Max(int poly_index, Vector3 &set_max);
    void Update_Min_Max(int poly_index, Vector3 &set_min, Vector3 &set_max);

    void Build_W3D_AABTree_Recursive(
        CullNodeStruct *node, W3dMeshAABTreeNode *w3dnodes, uint32_t *poly_indices, int &cur_node, int &cur_poly);
    CullNodeStruct *m_root;
    int m_curPolyIndex;

    int m_polyCount;
    TriIndex *m_polys;
    int m_vertCount;
    Vector3 *m_verts;

    friend class AABTreeClass;
};
