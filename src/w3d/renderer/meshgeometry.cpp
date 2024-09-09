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
#include "aabox.h"
#include "aabtree.h"
#include "camera.h"
#include "chunkio.h"
#include "htree.h"
#include "obbox.h"
#include "plane.h"
#include "rinfo.h"
#include "sphere.h"
#include "vp.h"
#include "w3d_file.h"
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
    if (Get_Flag(DIRTY_VNORMALS)) {
        Compute_Vertex_Normals(Get_Vert_Normals());
    }
    return Get_Vert_Normals();
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

MeshGeometryClass::MeshGeometryClass() :
    m_meshName(nullptr),
    m_userText(nullptr),
    m_flags(0),
    m_sortLevel(SORT_LEVEL_NONE),
    m_w3dAttributes(0),
    m_polyCount(0),
    m_vertexCount(0),
    m_poly(nullptr),
    m_vertex(nullptr),
    m_vertexNorm(nullptr),
    m_planeEq(nullptr),
    m_vertexShadeIdx(nullptr),
    m_vertexBoneLink(nullptr),
    m_polySurfaceType(nullptr),
    m_boundBoxMin(0, 0, 0),
    m_boundBoxMax(1, 1, 1),
    m_boundSphereCenter(0, 0, 0),
    m_boundSphereRadius(1),
    m_cullTree(nullptr)
{
}

MeshGeometryClass::MeshGeometryClass(const MeshGeometryClass &that) :
    m_meshName(nullptr),
    m_userText(nullptr),
    m_flags(0),
    m_sortLevel(SORT_LEVEL_NONE),
    m_w3dAttributes(0),
    m_polyCount(0),
    m_vertexCount(0),
    m_poly(nullptr),
    m_vertex(nullptr),
    m_vertexNorm(nullptr),
    m_planeEq(nullptr),
    m_vertexShadeIdx(nullptr),
    m_vertexBoneLink(nullptr),
    m_polySurfaceType(nullptr),
    m_boundBoxMin(0, 0, 0),
    m_boundBoxMax(1, 1, 1),
    m_boundSphereCenter(0, 0, 0),
    m_boundSphereRadius(1),
    m_cullTree(nullptr)
{
    *this = that;
}

MeshGeometryClass::~MeshGeometryClass()
{
    Reset_Geometry(0, 0);
}

void MeshGeometryClass::Reset_Geometry(int polycount, int vertcount)
{
    m_flags = 0;
    m_polyCount = 0;
    m_vertexCount = 0;
    m_sortLevel = SORT_LEVEL_NONE;

    Ref_Ptr_Release(m_meshName);
    Ref_Ptr_Release(m_userText);
    Ref_Ptr_Release(m_poly);
    Ref_Ptr_Release(m_polySurfaceType);
    Ref_Ptr_Release(m_vertex);
    Ref_Ptr_Release(m_vertexNorm);
    Ref_Ptr_Release(m_planeEq);
    Ref_Ptr_Release(m_vertexShadeIdx);
    Ref_Ptr_Release(m_vertexBoneLink);
    Ref_Ptr_Release(m_cullTree);

    m_polyCount = polycount;
    m_vertexCount = vertcount;

    if ((polycount != 0) && (vertcount != 0)) {
        m_poly = New_Share_Buffer<TriIndex>(m_polyCount, "MeshGeometryClass::Poly");
        m_polySurfaceType = New_Share_Buffer<uint8_t>(m_polyCount, "MeshGeometryClass::PolySurfaceType");
        m_vertex = New_Share_Buffer<Vector3>(m_vertexCount, "MeshGeometryClass::Vertex");

        m_poly->Clear();
        m_polySurfaceType->Clear();
        m_vertex->Clear();

        m_vertexNorm = New_Share_Buffer<Vector3>(m_vertexCount, "MeshGeometryClass::VertexNorm");
        m_vertexNorm->Clear();
    }
}

void MeshGeometryClass::Set_Name(const char *newname)
{
    if (m_meshName) {
        m_meshName->Release_Ref();
    }

    if (newname) {
        m_meshName = New_Share_Buffer<char>(strlen(newname) + 1, "MeshGeometryClass::MeshName");
        strcpy(m_meshName->Get_Array(), newname);
    }
}

const char *MeshGeometryClass::Get_User_Text()
{
    if (m_userText) {
        return m_userText->Get_Array();
    }

    return nullptr;
}

void MeshGeometryClass::Set_User_Text(char *usertext)
{
    if (m_userText) {
        m_userText->Release_Ref();
    }

    if (usertext) {
        m_userText = New_Share_Buffer<char>(strlen(usertext) + 1, "MeshGeometryClass::UserText");
        strcpy(m_userText->Get_Array(), usertext);
    }
}

void MeshGeometryClass::Get_Bounding_Box(AABoxClass *set_box)
{
    captainslog_assert(set_box != nullptr);
    set_box->m_center = (m_boundBoxMax + m_boundBoxMin) * 0.5f;
    set_box->m_extent = (m_boundBoxMax - m_boundBoxMin) * 0.5f;
}

void MeshGeometryClass::Get_Bounding_Sphere(SphereClass *set_sphere)
{
    captainslog_assert(set_sphere != nullptr);
    set_sphere->Center = m_boundSphereCenter;
    set_sphere->Radius = m_boundSphereRadius;
}

void MeshGeometryClass::Generate_Rigid_APT(const Vector3 &view_dir, SimpleDynVecClass<uint32_t> &apt)
{
    const Vector3 *loc = Get_Vertex_Array();
    const Vector4 *norms = Get_Plane_Array();
    const TriIndex *polys = Get_Polygon_Array();
    TriClass tri;

    for (int poly_counter = 0; poly_counter < m_polyCount; poly_counter++) {
        tri.V[0] = &(loc[polys[poly_counter][0]]);
        tri.V[1] = &(loc[polys[poly_counter][1]]);
        tri.V[2] = &(loc[polys[poly_counter][2]]);
        tri.N = Thyme::To_Vector3_Ptr(&(norms[poly_counter]));

        if (Vector3::Dot_Product(*tri.N, view_dir) < 0.0f) {
            apt.Add(poly_counter);
        }
    }
}

void MeshGeometryClass::Generate_Rigid_APT(const OBBoxClass &local_box, SimpleDynVecClass<uint32_t> &apt)
{
    if (m_cullTree != nullptr) {
        m_cullTree->Generate_APT(local_box, apt);
    } else {
        const Vector3 *loc = Get_Vertex_Array();
        const Vector4 *norms = Get_Plane_Array();
        const TriIndex *polys = Get_Polygon_Array();
        TriClass tri;

        for (int poly_counter = 0; poly_counter < m_polyCount; poly_counter++) {
            tri.V[0] = &(loc[polys[poly_counter][0]]);
            tri.V[1] = &(loc[polys[poly_counter][1]]);
            tri.V[2] = &(loc[polys[poly_counter][2]]);
            tri.N = Thyme::To_Vector3_Ptr(&(norms[poly_counter]));

            if (CollisionMath::Intersection_Test(local_box, tri)) {
                apt.Add(poly_counter);
            }
        }
    }
}

void MeshGeometryClass::Generate_Rigid_APT(
    const OBBoxClass &local_box, const Vector3 &viewdir, SimpleDynVecClass<uint32_t> &apt)
{
    if (m_cullTree != nullptr) {
        m_cullTree->Generate_APT(local_box, viewdir, apt);
    } else {
        const Vector3 *loc = Get_Vertex_Array();
        const Vector4 *norms = Get_Plane_Array();
        const TriIndex *polys = Get_Polygon_Array();
        TriClass tri;

        for (int poly_counter = 0; poly_counter < m_polyCount; poly_counter++) {
            tri.V[0] = &(loc[polys[poly_counter][0]]);
            tri.V[1] = &(loc[polys[poly_counter][1]]);
            tri.V[2] = &(loc[polys[poly_counter][2]]);
            tri.N = Thyme::To_Vector3_Ptr(&(norms[poly_counter]));

            if (Vector3::Dot_Product(*tri.N, viewdir) < 0.0f) {
                if (CollisionMath::Intersection_Test(local_box, tri)) {
                    apt.Add(poly_counter);
                }
            }
        }
    }
}

void MeshGeometryClass::Generate_Skin_APT(
    const OBBoxClass &world_box, SimpleDynVecClass<uint32_t> &apt, const Vector3 *world_vertex_locs)
{
    captainslog_assert(world_vertex_locs);
    const TriIndex *polys = Get_Polygon_Array();
    TriClass tri;

    for (int poly_counter = 0; poly_counter < m_polyCount; poly_counter++) {

        tri.V[0] = &(world_vertex_locs[polys[poly_counter][0]]);
        tri.V[1] = &(world_vertex_locs[polys[poly_counter][1]]);
        tri.V[2] = &(world_vertex_locs[polys[poly_counter][2]]);

        static const Vector3 dummy_vec(0.0f, 0.0f, 1.0f);
        tri.N = &dummy_vec;

        if (CollisionMath::Intersection_Test(world_box, tri)) {
            apt.Add(poly_counter);
        }
    }
}

bool MeshGeometryClass::Contains(const Vector3 &point)
{
    float yes = 0.0f;
    float no = 0.0f;

    for (int axis_dir = 0; axis_dir < 6; axis_dir++) {
        unsigned char flags = TRI_RAYCAST_FLAG_NONE;
        int intersections = Cast_Semi_Infinite_Axis_Aligned_Ray(point, axis_dir, flags);

        if (flags & TRI_RAYCAST_FLAG_START_IN_TRI) {
            return true;
        }

        float weight = flags & TRI_RAYCAST_FLAG_HIT_EDGE ? 0.1f : 1.0f;

        if (intersections & 0x01) {
            yes += weight;
        } else {
            no += weight;
        }
    }

    return yes > no;
}

bool MeshGeometryClass::Cast_Ray(RayCollisionTestClass &raytest)
{
    bool hit = false;

    if (m_cullTree) {
        hit = m_cullTree->Cast_Ray(raytest);
    } else {
        hit = Cast_Ray_Brute_Force(raytest);
    }

    return hit;
}

bool MeshGeometryClass::Cast_AABox(AABoxCollisionTestClass &boxtest)
{
    bool hit = false;

    if (m_cullTree) {
        hit = m_cullTree->Cast_AABox(boxtest);
    } else {
        hit = Cast_AABox_Brute_Force(boxtest);
    }

    return hit;
}

bool MeshGeometryClass::Cast_OBBox(OBBoxCollisionTestClass &boxtest)
{
    bool hit = false;

    if (m_cullTree) {
        hit = m_cullTree->Cast_OBBox(boxtest);
    } else {
        hit = Cast_OBBox_Brute_Force(boxtest);
    }

    return hit;
}

bool MeshGeometryClass::Intersect_OBBox(OBBoxIntersectionTestClass &boxtest)
{
    bool hit = false;

    if (m_cullTree) {
        hit = m_cullTree->Intersect_OBBox(boxtest);
    } else {
        hit = Intersect_OBBox_Brute_Force(boxtest);
    }

    return hit;
}

bool MeshGeometryClass::Cast_World_Space_AABox(AABoxCollisionTestClass &boxtest, const Matrix3D &transform)
{
    bool hit = false;

    if ((transform[0][0] == 1.0f) && (transform[1][1] == 1.0f)) {
        hit = Cast_AABox_Identity(boxtest, -transform.Get_Translation());
    } else if ((transform[0][1] == -1.0f) && (transform[1][0] == 1.0f)) {
        hit = Cast_AABox_Z90(boxtest, -transform.Get_Translation());
    } else if ((transform[0][0] == -1.0f) && (transform[1][1] == -1.0f)) {
        hit = Cast_AABox_Z180(boxtest, -transform.Get_Translation());
    } else if ((transform[0][1] == 1.0f) && (transform[1][0] == -1.0f)) {
        hit = Cast_AABox_Z270(boxtest, -transform.Get_Translation());
    } else {
        Matrix3D world_to_obj;
        transform.Get_Orthogonal_Inverse(world_to_obj);
        OBBoxCollisionTestClass obbox(boxtest, world_to_obj);

        if (m_cullTree) {
            hit = m_cullTree->Cast_OBBox(obbox);
        } else {
            hit = Cast_OBBox_Brute_Force(obbox);
        }

        if (hit) {
            Matrix3D::Rotate_Vector(transform, obbox.m_result->normal, &(obbox.m_result->normal));
            if (boxtest.m_result->compute_contact_point) {
                Matrix3D::Transform_Vector(transform, obbox.m_result->contact_point, &(obbox.m_result->contact_point));
            }
        }
    }

    return hit;
}

int MeshGeometryClass::Cast_Semi_Infinite_Axis_Aligned_Ray(const Vector3 &start_point, int axis_dir, unsigned char &flags)
{
    int count = 0;

    if (m_cullTree) {
        count = m_cullTree->Cast_Semi_Infinite_Axis_Aligned_Ray(start_point, axis_dir, flags);
    } else {
        const Vector3 *loc = Get_Vertex_Array();
        const Vector4 *plane = Get_Plane_Array();
        const TriIndex *polyverts = Get_Polygon_Array();

        static const int axis_r[6] = { 0, 0, 1, 1, 2, 2 };
        static const int axis_1[6] = { 1, 1, 2, 2, 0, 0 };
        static const int axis_2[6] = { 2, 2, 0, 0, 1, 1 };
        static const int direction[6] = { 1, 0, 1, 0, 1, 0 };
        captainslog_assert(axis_dir >= 0);
        captainslog_assert(axis_dir < 6);

        flags = TRI_RAYCAST_FLAG_NONE;
        int poly_count = Get_Polygon_Count();

        for (int poly_counter = 0; poly_counter < poly_count; poly_counter++) {
            const Vector3 &v0 = loc[polyverts[poly_counter][0]];
            const Vector3 &v1 = loc[polyverts[poly_counter][1]];
            const Vector3 &v2 = loc[polyverts[poly_counter][2]];
            const Vector4 &tri_plane = plane[poly_counter];

            count += (unsigned int)Cast_Semi_Infinite_Axis_Aligned_Ray_To_Triangle(v0,
                v1,
                v2,
                tri_plane,
                start_point,
                axis_r[axis_dir],
                axis_1[axis_dir],
                axis_2[axis_dir],
                direction[axis_dir],
                flags);
        }
    }

    return count;
}

bool MeshGeometryClass::Cast_AABox_Identity(AABoxCollisionTestClass &boxtest, const Vector3 &translation)
{
    AABoxCollisionTestClass newbox(boxtest);
    newbox.Translate(translation);

    if (m_cullTree) {
        return m_cullTree->Cast_AABox(newbox);
    } else {
        return Cast_AABox_Brute_Force(newbox);
    }
}

bool MeshGeometryClass::Cast_AABox_Z90(AABoxCollisionTestClass &boxtest, const Vector3 &translation)
{
    AABoxCollisionTestClass newbox(boxtest);
    newbox.Translate(translation);
    newbox.Rotate(AABoxCollisionTestClass::ROTATE_Z270);

    bool hit;
    if (m_cullTree) {
        hit = m_cullTree->Cast_AABox(newbox);
    } else {
        hit = Cast_AABox_Brute_Force(newbox);
    }

    if (hit) {
        float tmp = boxtest.m_result->normal.X;
        boxtest.m_result->normal.X = -boxtest.m_result->normal.Y;
        boxtest.m_result->normal.Y = tmp;
    }

    return hit;
}

bool MeshGeometryClass::Cast_AABox_Z180(AABoxCollisionTestClass &boxtest, const Vector3 &translation)
{
    AABoxCollisionTestClass newbox(boxtest);
    newbox.Translate(translation);
    newbox.Rotate(AABoxCollisionTestClass::ROTATE_Z180);

    bool hit;
    if (m_cullTree) {
        hit = m_cullTree->Cast_AABox(newbox);
    } else {
        hit = Cast_AABox_Brute_Force(newbox);
    }

    if (hit) {
        boxtest.m_result->normal.X = -boxtest.m_result->normal.X;
        boxtest.m_result->normal.Y = -boxtest.m_result->normal.Y;
    }

    return hit;
}

bool MeshGeometryClass::Cast_AABox_Z270(AABoxCollisionTestClass &boxtest, const Vector3 &translation)
{
    AABoxCollisionTestClass newbox(boxtest);
    newbox.Translate(translation);
    newbox.Rotate(AABoxCollisionTestClass::ROTATE_Z90);

    bool hit;
    if (m_cullTree) {
        hit = m_cullTree->Cast_AABox(newbox);
    } else {
        hit = Cast_AABox_Brute_Force(newbox);
    }

    if (hit) {
        float tmp = boxtest.m_result->normal.X;
        boxtest.m_result->normal.X = boxtest.m_result->normal.Y;
        boxtest.m_result->normal.Y = -tmp;
    }

    return hit;
}

bool MeshGeometryClass::Intersect_OBBox_Brute_Force(OBBoxIntersectionTestClass &localtest)
{
    TriClass tri;
    const Vector3 *loc = Get_Vertex_Array();
    const TriIndex *polyverts = Get_Polygon_Array();
    const Vector4 *norms = Get_Plane_Array();

    for (int i = 0; i < Get_Polygon_Count(); i++) {
        tri.V[0] = &(loc[polyverts[i][0]]);
        tri.V[1] = &(loc[polyverts[i][1]]);
        tri.V[2] = &(loc[polyverts[i][2]]);
        tri.N = Thyme::To_Vector3_Ptr(&(norms[i]));

        if (CollisionMath::Intersection_Test(localtest.m_box, tri)) {
            return true;
        }
    }
    return false;
}

bool MeshGeometryClass::Cast_Ray_Brute_Force(RayCollisionTestClass &raytest)
{
    int i;
    TriClass tri;
    const Vector3 *loc = Get_Vertex_Array();
    const TriIndex *polyverts = Get_Polygon_Array();
    const Vector4 *norms = Get_Plane_Array();

    bool hit = false;
    for (i = 0; i < Get_Polygon_Count(); i++) {
        tri.V[0] = &(loc[polyverts[i][0]]);
        tri.V[1] = &(loc[polyverts[i][1]]);
        tri.V[2] = &(loc[polyverts[i][2]]);
        tri.N = Thyme::To_Vector3_Ptr(&(norms[i]));

        hit = hit | CollisionMath::Collide(raytest.m_ray, tri, raytest.m_result);
        if (hit) {
            raytest.m_result->surface_type = Get_Poly_Surface_Type(i);
        }

        if (raytest.m_result->start_bad)
            return true;
    }
    return hit;
}

bool MeshGeometryClass::Cast_AABox_Brute_Force(AABoxCollisionTestClass &boxtest)
{
    TriClass tri;
    int polyhit = -1;

    const Vector3 *loc = Get_Vertex_Array();
    const TriIndex *polyverts = Get_Polygon_Array();
    const Vector4 *norms = Get_Plane_Array();

    for (int i = 0; i < Get_Polygon_Count(); i++) {
        tri.V[0] = &(loc[polyverts[i][0]]);
        tri.V[1] = &(loc[polyverts[i][1]]);
        tri.V[2] = &(loc[polyverts[i][2]]);
        tri.N = Thyme::To_Vector3_Ptr(&(norms[i]));

        if (CollisionMath::Collide(boxtest.m_box, boxtest.m_move, tri, boxtest.m_result)) {
            polyhit = i;
        }

        if (boxtest.m_result->start_bad) {
            return true;
        }
    }
    if (polyhit != -1) {
        boxtest.m_result->surface_type = Get_Poly_Surface_Type(polyhit);
        return true;
    }
    return false;
}

bool MeshGeometryClass::Cast_OBBox_Brute_Force(OBBoxCollisionTestClass &boxtest)
{
    TriClass tri;
    int polyhit = -1;

    const Vector3 *loc = Get_Vertex_Array();
    const TriIndex *polyverts = Get_Polygon_Array();
    const Vector4 *norms = Get_Plane_Array();

    for (int i = 0; i < Get_Polygon_Count(); i++) {
        tri.V[0] = &(loc[polyverts[i][0]]);
        tri.V[1] = &(loc[polyverts[i][1]]);
        tri.V[2] = &(loc[polyverts[i][2]]);
        tri.N = Thyme::To_Vector3_Ptr(&(norms[i]));

        if (CollisionMath::Collide(boxtest.m_box, boxtest.m_move, tri, Vector3(0, 0, 0), boxtest.m_result)) {
            polyhit = i;
        }

        if (boxtest.m_result->start_bad) {
            return true;
        }
    }
    if (polyhit != -1) {
        boxtest.m_result->surface_type = Get_Poly_Surface_Type(polyhit);
        return true;
    }
    return false;
}

void MeshGeometryClass::Compute_Plane_Equations(Vector4 *peq)
{
    captainslog_assert(peq != nullptr);

    TriIndex *poly = m_poly->Get_Array();
    Vector3 *vert = m_vertex->Get_Array();

    for (int pidx = 0; pidx < m_polyCount; pidx++) {
        Vector3 a, b, normal;
        const Vector3 &p0 = vert[poly[pidx][0]];

        Vector3::Subtract(vert[poly[pidx][1]], p0, &a);
        Vector3::Subtract(vert[poly[pidx][2]], p0, &b);
        Vector3::Cross_Product(a, b, &normal);
        normal.Normalize();

        peq[pidx].Set(normal.X, normal.Y, normal.Z, -(Vector3::Dot_Product(p0, normal)));
    }
    Set_Flag(DIRTY_PLANES, false);
}

void MeshGeometryClass::Compute_Vertex_Normals(Vector3 *vnorm)
{
    captainslog_assert(vnorm != nullptr);

    if ((m_polyCount == 0) || (m_vertexCount == 0)) {
        return;
    }

    const Vector4 *peq = Get_Plane_Array();
    TriIndex *poly = m_poly->Get_Array();
    const uint32_t *shadeIx = Get_Vertex_Shade_Index_Array(false);

    if (!shadeIx) {
        VectorProcessorClass::Clear(vnorm, m_vertexCount);

        for (int pidx = 0; pidx < m_polyCount; pidx++) {
            vnorm[poly[pidx].I].X += peq[pidx].X;
            vnorm[poly[pidx].I].Y += peq[pidx].Y;
            vnorm[poly[pidx].I].Z += peq[pidx].Z;

            vnorm[poly[pidx].J].X += peq[pidx].X;
            vnorm[poly[pidx].J].Y += peq[pidx].Y;
            vnorm[poly[pidx].J].Z += peq[pidx].Z;

            vnorm[poly[pidx].K].X += peq[pidx].X;
            vnorm[poly[pidx].K].Y += peq[pidx].Y;
            vnorm[poly[pidx].K].Z += peq[pidx].Z;
        }

    } else {
        VectorProcessorClass::Clear(vnorm, m_vertexCount);

        for (int pidx = 0; pidx < m_polyCount; pidx++) {
            vnorm[shadeIx[poly[pidx].I]].X += peq[pidx].X;
            vnorm[shadeIx[poly[pidx].I]].Y += peq[pidx].Y;
            vnorm[shadeIx[poly[pidx].I]].Z += peq[pidx].Z;

            vnorm[shadeIx[poly[pidx].J]].X += peq[pidx].X;
            vnorm[shadeIx[poly[pidx].J]].Y += peq[pidx].Y;
            vnorm[shadeIx[poly[pidx].J]].Z += peq[pidx].Z;

            vnorm[shadeIx[poly[pidx].K]].X += peq[pidx].X;
            vnorm[shadeIx[poly[pidx].K]].Y += peq[pidx].Y;
            vnorm[shadeIx[poly[pidx].K]].Z += peq[pidx].Z;
        }

        for (unsigned int vidx = 0; vidx < (unsigned int)m_vertexCount; vidx++) {
            if (shadeIx[vidx] == vidx) {
                vnorm[vidx].Normalize();
            } else {
                vnorm[vidx] = vnorm[shadeIx[vidx]];
            }
        }
    }

    VectorProcessorClass::Normalize(vnorm, m_vertexCount);
    Set_Flag(DIRTY_VNORMALS, false);
}

void MeshGeometryClass::Compute_Bounds(Vector3 *verts)
{
    m_boundBoxMin.Set(0.0f, 0.0f, 0.0f);
    m_boundBoxMax.Set(0.0f, 0.0f, 0.0f);
    m_boundSphereCenter.Set(0.0f, 0.0f, 0.0f);
    m_boundSphereRadius = 0.0f;

    if (m_vertexCount == 0) {
        return;
    }

    if (verts == nullptr) {
        verts = m_vertex->Get_Array();
    }

    VectorProcessorClass::MinMax(verts, m_boundBoxMin, m_boundBoxMax, m_vertexCount);

    m_boundSphereCenter = (m_boundBoxMin + m_boundBoxMax) / 2.0f;
    m_boundSphereRadius = (float)(m_boundBoxMax - m_boundSphereCenter).Length2();
    m_boundSphereRadius = ((float)GameMath::Sqrt(m_boundSphereRadius)) * 1.00001f;
    Set_Flag(DIRTY_BOUNDS, false);
}

Vector3 *MeshGeometryClass::Get_Vert_Normals()
{
    captainslog_assert(m_vertexNorm);
    return m_vertexNorm->Get_Array();
}

void MeshGeometryClass::Compute_Plane(int pidx, PlaneClass *set_plane) const
{
    captainslog_assert(pidx >= 0);
    captainslog_assert(pidx < m_polyCount);
    TriIndex &poly = m_poly->Get_Array()[pidx];
    Vector3 *verts = m_vertex->Get_Array();

    set_plane->Set(verts[poly.I], verts[poly.J], verts[poly.K]);
}

W3DErrorType MeshGeometryClass::Load_W3D(ChunkLoadClass &cload)
{
    cload.Open_Chunk();

    if (cload.Cur_Chunk_ID() != W3D_CHUNK_MESH_HEADER3) {
        captainslog_warn(("Old format mesh mesh, no longer supported.\n"));
        goto Error;
    }

    W3dMeshHeader3Struct header;

    if (cload.Read(&header, sizeof(W3dMeshHeader3Struct)) != sizeof(W3dMeshHeader3Struct)) {
        goto Error;
    }

    cload.Close_Chunk();
    char *tmpname;
    int namelen;

    Reset_Geometry(header.NumTris, header.NumVertices);

    namelen = strlen(header.ContainerName);
    namelen += strlen(header.MeshName);
    namelen += 2;
    m_w3dAttributes = header.Attributes;
    m_sortLevel = header.SortLevel;
    tmpname = new char[namelen];
    memset(tmpname, 0, sizeof(char) * namelen);

    if (strlen(header.ContainerName) > 0) {
        strcpy(tmpname, header.ContainerName);
        strcat(tmpname, ".");
    }

    strcat(tmpname, header.MeshName);
    Set_Name(tmpname);
    delete[] tmpname;
    tmpname = nullptr;

    m_boundBoxMin.Set(header.Min.x, header.Min.y, header.Min.z);
    m_boundBoxMax.Set(header.Max.x, header.Max.y, header.Max.z);

    m_boundSphereCenter.Set(header.SphCenter.x, header.SphCenter.y, header.SphCenter.z);
    m_boundSphereRadius = header.SphRadius;

    if (header.Version >= 0x40001) {
        int geometry_type = header.Attributes & W3D_MESH_FLAG_GEOMETRY_TYPE_MASK;
        switch (geometry_type) {
            case W3D_MESH_FLAG_GEOMETRY_TYPE_NORMAL:
                break;
            case W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_ALIGNED:
                Set_Flag(ALIGNED, true);
                break;
            case W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_ORIENTED:
                Set_Flag(ORIENTED, true);
                break;
            case W3D_MESH_FLAG_GEOMETRY_TYPE_SKIN:
                Set_Flag(SKIN, true);
                break;
        }
    }

    if (header.Attributes & W3D_MESH_FLAG_TWO_SIDED) {
        Set_Flag(TWO_SIDED, true);
    }

    if (header.Attributes & W3D_MESH_FLAG_CAST_SHADOW) {
        Set_Flag(CAST_SHADOW, true);
    }

    Read_Chunks(cload);

    if ((header.Version < 0x30000) && (Get_Flag(SKIN))) {
        uint16_t *links = Get_Bone_Links();
        captainslog_assert(links);

        for (int bi = 0; bi < Get_Vertex_Count(); bi++) {
            links[bi] += 1;
        }
    }

    if ((((m_w3dAttributes & W3D_MESH_FLAG_COLLISION_TYPE_MASK) >> W3D_MESH_FLAG_COLLISION_TYPE_SHIFT) != 0)
        && (m_cullTree == nullptr)) {
        Generate_Culling_Tree();
    }

    return W3D_ERROR_OK;

Error:

    return W3D_ERROR_LOAD_FAILED;
}

W3DErrorType MeshGeometryClass::Read_Chunks(ChunkLoadClass &cload)
{
    while (cload.Open_Chunk()) {
        W3DErrorType error = W3D_ERROR_OK;

        switch (cload.Cur_Chunk_ID()) {
            case W3D_CHUNK_VERTICES:
                error = Read_Vertices(cload);
                break;
            case W3D_CHUNK_SURRENDER_NORMALS:
            case W3D_CHUNK_VERTEX_NORMALS:
                error = Read_Vertex_Normals(cload);
                break;
            case W3D_CHUNK_TRIANGLES:
                error = Read_Triangles(cload);
                break;
            case W3D_CHUNK_MESH_USER_TEXT:
                error = Read_User_Text(cload);
                break;
            case W3D_CHUNK_VERTEX_INFLUENCES:
                error = Read_Vertex_Influences(cload);
                break;
            case W3D_CHUNK_VERTEX_SHADE_INDICES:
                error = Read_Vertex_Shade_Indices(cload);
                break;
            case W3D_CHUNK_AABTREE:
                Read_AABTree(cload);
                break;
            default:
                break;
        }

        cload.Close_Chunk();

        if (error != W3D_ERROR_OK) {
            return error;
        }
    }

    return W3D_ERROR_OK;
}

W3DErrorType MeshGeometryClass::Read_Vertices(ChunkLoadClass &cload)
{
    W3dVectorStruct vert;
    Vector3 *loc = m_vertex->Get_Array();
    captainslog_assert(loc);

    for (int i = 0; i < Get_Vertex_Count(); i++) {

        if (cload.Read(&vert, sizeof(W3dVectorStruct)) != sizeof(W3dVectorStruct)) {
            return W3D_ERROR_LOAD_FAILED;
        }

        loc[i].X = vert.x;
        loc[i].Y = vert.y;
        loc[i].Z = vert.z;
    }

    return W3D_ERROR_OK;
}

W3DErrorType MeshGeometryClass::Read_Vertex_Normals(ChunkLoadClass &cload)
{
    W3dVectorStruct norm;
    Vector3 *mdlnorms = Get_Vert_Normals();
    captainslog_assert(mdlnorms);

    for (int i = 0; i < m_vertexCount; i++) {
        if (cload.Read(&norm, sizeof(W3dVectorStruct)) != sizeof(W3dVectorStruct)) {
            return W3D_ERROR_LOAD_FAILED;
        }

        mdlnorms[i].Set(norm.x, norm.y, norm.z);
    }

    return W3D_ERROR_OK;
}

W3DErrorType MeshGeometryClass::Read_Triangles(ChunkLoadClass &cload)
{
    W3dTriStruct tri;

    TriIndex *vi = Get_Polys();
    Set_Flag(DIRTY_PLANES, false);
    Vector4 *peq = Get_Planes();
    uint8_t *surface_types = Get_Poly_Surface_Type_Array();

    for (int i = 0; i < Get_Polygon_Count(); i++) {

        if (cload.Read(&tri, sizeof(W3dTriStruct)) != sizeof(W3dTriStruct)) {
            return W3D_ERROR_LOAD_FAILED;
        }

        vi[i].I = tri.Vindex[0];
        vi[i].J = tri.Vindex[1];
        vi[i].K = tri.Vindex[2];

        peq[i].X = tri.Normal.x;
        peq[i].Y = tri.Normal.y;
        peq[i].Z = tri.Normal.z;
        peq[i].W = -tri.Dist;

        captainslog_assert(tri.Attributes < 256);
        surface_types[i] = (uint8_t)(tri.Attributes);
    }

    return W3D_ERROR_OK;
}

W3DErrorType MeshGeometryClass::Read_User_Text(ChunkLoadClass &cload)
{
    unsigned int textlen = cload.Cur_Chunk_Length();

    if (m_userText != nullptr) {
        return W3D_ERROR_OK;
    }

    m_userText = New_Share_Buffer<char>(textlen, "MeshGeometryClass::UserText");

    if (cload.Read(m_userText->Get_Array(), textlen) != textlen) {
        return W3D_ERROR_LOAD_FAILED;
    }

    return W3D_ERROR_OK;
}

W3DErrorType MeshGeometryClass::Read_Vertex_Influences(ChunkLoadClass &cload)
{
    W3dVertInfStruct vinf;
    uint16_t *links = Get_Bone_Links(true);
    captainslog_assert(links);

    for (int i = 0; i < Get_Vertex_Count(); i++) {
        if (cload.Read(&vinf, sizeof(W3dVertInfStruct)) != sizeof(W3dVertInfStruct)) {
            return W3D_ERROR_LOAD_FAILED;
        }

        links[i] = vinf.BoneIdx[0];
    }

    Set_Flag(SKIN, true);
    return W3D_ERROR_OK;
}

W3DErrorType MeshGeometryClass::Read_Vertex_Shade_Indices(ChunkLoadClass &cload)
{
    uint32_t *shade_index = Get_Shade_Indices(true);
    uint32_t si;

    for (int i = 0; i < Get_Vertex_Count(); i++) {
        if (cload.Read(&si, sizeof(uint32_t)) != sizeof(uint32_t)) {
            return W3D_ERROR_LOAD_FAILED;
        }
        shade_index[i] = si;
    }
    return W3D_ERROR_OK;
}

void MeshGeometryClass::Get_Deformed_Vertices(Vector3 *dst_vert, Vector3 *dst_norm, const HTreeClass *htree)
{
    int vertex_count = Get_Vertex_Count();
    Vector3 *src_vert = m_vertex->Get_Array();
    Vector3 *src_norm = m_vertexNorm->Get_Array();
    uint16_t *bonelink = m_vertexBoneLink->Get_Array();

    for (int vi = 0; vi < vertex_count;) {
        const Matrix3D &tm = htree->Get_Transform(bonelink[vi]);
        Matrix3D mytm = tm;

        int idx = bonelink[vi];
        int cnt;

        for (cnt = vi; cnt < vertex_count; cnt++) {
            if (idx != bonelink[cnt]) {
                break;
            }
        }

        VectorProcessorClass::Transform(dst_vert + vi, src_vert + vi, mytm, cnt - vi);
        mytm.Set_Translation(Vector3(0.0f, 0.0f, 0.0f));
        VectorProcessorClass::Transform(dst_norm + vi, src_norm + vi, mytm, cnt - vi);
        vi = cnt;
    }
}

void MeshGeometryClass::Get_Deformed_Vertices(Vector3 *dst_vert, const HTreeClass *htree)
{
    Vector3 *src_vert = m_vertex->Get_Array();
    uint16_t *bonelink = m_vertexBoneLink->Get_Array();

    for (int vi = 0; vi < Get_Vertex_Count(); vi++) {
        const Matrix3D &tm = htree->Get_Transform(bonelink[vi]);
        Matrix3D::Transform_Vector(tm, src_vert[vi], &(dst_vert[vi]));
    }
}

void MeshGeometryClass::Get_Deformed_Screenspace_Vertices(
    Vector4 *dst_vert, const RenderInfoClass &rinfo, const Matrix3D &mesh_tm, const HTreeClass *htree)
{
    Matrix4 prj = rinfo.m_camera.Get_Projection_Matrix() * rinfo.m_camera.Get_View_Matrix() * mesh_tm;

    Vector3 *src_vert = m_vertex->Get_Array();
    int vertex_count = Get_Vertex_Count();

    if (Get_Flag(SKIN) && m_vertexBoneLink && htree) {
        uint16_t *bonelink = m_vertexBoneLink->Get_Array();

        for (int vi = 0; vi < vertex_count;) {
            int idx = bonelink[vi];
            Matrix4 tm = prj * htree->Get_Transform(idx);
            int cnt;

            for (cnt = vi; cnt < vertex_count; cnt++) {
                if (idx != bonelink[cnt]) {
                    break;
                }
            }

            VectorProcessorClass::Transform(dst_vert + vi, src_vert + vi, tm, cnt - vi);
            vi = cnt;
        }
    } else {
        VectorProcessorClass::Transform(dst_vert, src_vert, prj, vertex_count);
    }
}
