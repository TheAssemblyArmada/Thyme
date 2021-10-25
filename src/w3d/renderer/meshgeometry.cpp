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
 *
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "meshgeometry.h"
#include "aabtree.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

#ifndef GAME_DLL
SimpleVecClass<Vector4> g_planeEQArray(1024);
#else
extern SimpleVecClass<Vector4> &g_planeEQArray;
#endif

const Vector3 *MeshGeometryClass::Get_Vertex_Normal_Array()
{
#ifdef GAME_DLL
    return Call_Method<const Vector3 *, MeshGeometryClass>(PICK_ADDRESS(0x0083B760, 0x005233D0), this);
#else
    return nullptr;
#endif
}

const char *MeshGeometryClass::Get_Name() const
{
    if (m_meshName) {
        return m_meshName->Get_Array();
    }

    return nullptr;
}

const Vector4 *MeshGeometryClass::Get_Plane_Array(bool create)
{
    Vector4 *planes = Get_Planes(create);
    Compute_Plane_Equations(planes);
    return planes;
}

Vector4 *MeshGeometryClass::Get_Planes(bool create)
{
    g_planeEQArray.Uninitialised_Grow(m_polyCount);
    return &(g_planeEQArray[0]);
}

void MeshGeometryClass::Scale(const Vector3 &sc)
{
    captainslog_assert(m_vertex);
    Vector3 *vert = m_vertex->Get_Array();

    for (int i = 0; i < m_vertexCount; i++) {
        vert[i].X *= sc.X;
        vert[i].Y *= sc.Y;
        vert[i].Z *= sc.Z;
    }

    m_boundBoxMin.Scale(sc);
    m_boundBoxMax.Scale(sc);
    m_boundSphereCenter.Scale(sc);

    float max;
    max = (sc.X > sc.Y) ? sc.X : sc.Y;
    max = (max > sc.Z) ? max : sc.Z;
    m_boundSphereRadius *= max;

    if (sc.X != sc.Y || sc.Y != sc.Z) {
        Set_Flag(DIRTY_VNORMALS, true);
    }

    Set_Flag(DIRTY_PLANES, true);

    if (m_cullTree) {
        if (GameMath::Fabs(sc.X - sc.Y) < GAMEMATH_EPSILON || GameMath::Fabs(sc.X - sc.Z) < GAMEMATH_EPSILON) {
            AABTreeClass *tree = new AABTreeClass();
            *tree = *m_cullTree;
            tree->Set_Mesh(this);
            Ref_Ptr_Set(m_cullTree, tree);
            Ref_Ptr_Release(tree);
            m_cullTree->Scale(sc.X);
        } else {
            Ref_Ptr_Release(m_cullTree);
            Generate_Culling_Tree();
        }
    }
}

void MeshGeometryClass::Generate_Culling_Tree()
{
    AABTreeBuilderClass builder;
    builder.Build_AABTree(m_polyCount, m_poly->Get_Array(), m_vertexCount, m_vertex->Get_Array());

    m_cullTree = new AABTreeClass(&builder);
    m_cullTree->Set_Mesh(this);
}

W3DErrorType MeshGeometryClass::Read_AABTree(ChunkLoadClass &cload)
{
    Ref_Ptr_Release(m_cullTree);
    m_cullTree = new AABTreeClass();
    m_cullTree->Load_W3D(cload);
    m_cullTree->Set_Mesh(this);
    return W3D_ERROR_OK;
}

MeshGeometryClass &MeshGeometryClass::operator=(const MeshGeometryClass &that)
{
    if (this != &that) {
        m_flags = that.m_flags;
        m_sortLevel = that.m_sortLevel;
        m_w3dAttributes = that.m_w3dAttributes;
        m_polyCount = that.m_polyCount;
        m_vertexCount = that.m_vertexCount;

        m_boundBoxMin = that.m_boundBoxMin;
        m_boundBoxMax = that.m_boundBoxMax;
        m_boundSphereCenter = that.m_boundSphereCenter;
        m_boundSphereRadius = that.m_boundSphereRadius;

        ShareBufferClass<char> *name = const_cast<ShareBufferClass<char> *>(that.m_meshName);
        Ref_Ptr_Set(m_meshName, name);
        ShareBufferClass<char> *user = const_cast<ShareBufferClass<char> *>(that.m_userText);
        Ref_Ptr_Set(m_userText, user);
        ShareBufferClass<TriIndex> *poly = const_cast<ShareBufferClass<TriIndex> *>(that.m_poly);
        Ref_Ptr_Set(m_poly, poly);
        ShareBufferClass<uint8_t> *surf = const_cast<ShareBufferClass<uint8_t> *>(that.m_polySurfaceType);
        Ref_Ptr_Set(m_polySurfaceType, surf);
        ShareBufferClass<Vector3> *vert = const_cast<ShareBufferClass<Vector3> *>(that.m_vertex);
        Ref_Ptr_Set(m_vertex, vert);
        ShareBufferClass<Vector3> *norm = const_cast<ShareBufferClass<Vector3> *>(that.m_vertexNorm);
        Ref_Ptr_Set(m_vertexNorm, norm);
        ShareBufferClass<Vector4> *eq = const_cast<ShareBufferClass<Vector4> *>(that.m_planeEq);
        Ref_Ptr_Set(m_planeEq, eq);
        ShareBufferClass<uint32_t> *shade = const_cast<ShareBufferClass<uint32_t> *>(that.m_vertexShadeIdx);
        Ref_Ptr_Set(m_vertexShadeIdx, shade);
        ShareBufferClass<uint16_t> *bone = const_cast<ShareBufferClass<uint16_t> *>(that.m_vertexBoneLink);
        Ref_Ptr_Set(m_vertexBoneLink, bone);

        Ref_Ptr_Release(m_cullTree);

        if (that.m_cullTree) {
            m_cullTree = new AABTreeClass();
            *m_cullTree = *that.m_cullTree;
            m_cullTree->Set_Mesh(this);
        }
    }
    return *this;
}
