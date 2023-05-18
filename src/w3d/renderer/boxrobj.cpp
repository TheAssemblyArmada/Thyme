/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Box render objects
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "boxrobj.h"
#include "coltest.h"
#include "dx8indexbuffer.h"
#include "dx8vertexbuffer.h"
#include "dx8wrapper.h"
#include "inttest.h"
#include "rinfo.h"
#include "shader.h"
#include "vector3i.h"
#include "vertmaterial.h"
#include "w3d_util.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif
#include <cstring>

using std::strcpy;

VertexMaterialClass *g_boxMaterial;
ShaderClass g_boxShader;
bool BoxRenderObjClass::s_isInitted;
int BoxRenderObjClass::s_displayMask;

static Vector3i16 g_boxFaces[12] = {
    Vector3i16(0, 1, 2),
    Vector3i16(0, 2, 3),
    Vector3i16(4, 7, 6),
    Vector3i16(4, 6, 5),
    Vector3i16(0, 3, 7),
    Vector3i16(0, 7, 4),
    Vector3i16(1, 5, 6),
    Vector3i16(1, 6, 2),
    Vector3i16(4, 5, 1),
    Vector3i16(4, 1, 0),
    Vector3i16(3, 2, 6),
    Vector3i16(3, 6, 7),
};

static Vector3 g_boxVerts[8] = {
    Vector3(1, 1, 1),
    Vector3(-1, 1, 1),
    Vector3(-1, -1, 1),
    Vector3(1, -1, 1),
    Vector3(1, 1, -1),
    Vector3(-1, 1, -1),
    Vector3(-1, -1, -1),
    Vector3(1, -1, -1),
};

static Vector3 g_boxVertexNormals[8] = {
    Vector3(0.57735026f, 0.57735026f, 0.57735026f),
    Vector3(-0.57735026f, 0.57735026f, 0.57735026f),
    Vector3(-0.57735026f, -0.57735026f, 0.57735026f),
    Vector3(0.57735026f, -0.57735026f, 0.57735026f),
    Vector3(0.57735026f, 0.57735026f, -0.57735026f),
    Vector3(-0.57735026f, 0.57735026f, -0.57735026f),
    Vector3(-0.57735026f, -0.57735026f, -0.57735026f),
    Vector3(0.57735026f, -0.57735026f, -0.57735026f),
};

void BoxRenderObjClass::Init()
{
    captainslog_assert(s_isInitted == false);
    captainslog_assert(g_boxMaterial == nullptr);

    g_boxMaterial = new VertexMaterialClass();
    g_boxMaterial->Set_Ambient(0, 0, 0);
    g_boxMaterial->Set_Diffuse(0, 0, 0);
    g_boxMaterial->Set_Specular(0, 0, 0);
    g_boxMaterial->Set_Emissive(1, 1, 1);
    g_boxMaterial->Set_Opacity(1);
    g_boxMaterial->Set_Shininess(0);
    g_boxShader = ShaderClass::s_presetAlphaSolidShader;
    s_isInitted = true;
}

void BoxRenderObjClass::Shutdown()
{
    captainslog_assert(s_isInitted == true);

    Ref_Ptr_Release(g_boxMaterial);
    s_isInitted = false;
}

void BoxRenderObjClass::Set_Box_Display_Mask(int mask)
{
    s_displayMask = mask;
}

BoxRenderObjClass::BoxRenderObjClass(const BoxRenderObjClass &src)
{
    *this = src;
}

BoxRenderObjClass::BoxRenderObjClass(const W3dBoxStruct &src) : m_opacity(0.25f)
{
    Set_Name(src.Name);
    W3dUtilityClass::Convert_Color(src.Color, &m_color);
    W3dUtilityClass::Convert_Vector(src.Center, &m_objSpaceCenter);
    W3dUtilityClass::Convert_Vector(src.Extent, &m_objSpaceExtent);
    Set_Collision_Type(
        2 * ((src.Attributes & W3D_BOX_ATTRIBUTE_COLLISION_TYPE_MASK) >> W3D_BOX_ATTRIBUTE_COLLISION_TYPE_SHIFT));
}

BoxRenderObjClass::BoxRenderObjClass() :
    m_color(1, 1, 1), m_objSpaceCenter(0, 0, 0), m_objSpaceExtent(1, 1, 1), m_opacity(0.25f)
{
    memset(m_name, 0, sizeof(m_name));
}

BoxRenderObjClass &BoxRenderObjClass::operator=(const BoxRenderObjClass &that)
{
    if (this != &that) {
        RenderObjClass::operator=(that);
        Set_Name(that.Get_Name());
        m_color = that.m_color;
        m_objSpaceCenter = that.m_objSpaceCenter;
        m_objSpaceExtent = that.m_objSpaceExtent;
    }
    return *this;
}

void BoxRenderObjClass::Set_Color(const Vector3 &color)
{
    m_color = color;
}

void BoxRenderObjClass::Render_Box(RenderInfoClass &rinfo, const Vector3 &center, const Vector3 &extent)
{
#ifdef BUILD_WITH_D3D8
    if (s_isInitted && s_displayMask & Get_Collision_Type()) {
        static Vector3 verts[8];
        for (int i = 0; i < 8; i++) {
            verts[i].X = g_boxVerts[i].X * extent.X + center.X;
            verts[i].Y = g_boxVerts[i].Y * extent.Y + center.Y;
            verts[i].Z = g_boxVerts[i].Z * extent.Z + center.Z;
        }

        unsigned int color = DX8Wrapper::Convert_Color(m_color, m_opacity);

        DynamicVBAccessClass vb_access(2, D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX2, 8);
        {
            DynamicVBAccessClass::WriteLockClass lock(&vb_access);
            VertexFormatXYZNDUV2 *v = lock.Get_Formatted_Vertex_Array();

            for (int i = 0; i < 8; ++i) {
                v[i].x = verts[i].X;
                v[i].y = verts[i].Y;
                v[i].z = verts[i].Z;

                v[i].nx = g_boxVertexNormals[i].X;
                v[i].ny = g_boxVertexNormals[i].Y;
                v[i].nz = g_boxVertexNormals[i].Z;
                v[i].diffuse = color;
            }
        }

        DynamicIBAccessClass ib_access(2, 36);
        {
            DynamicIBAccessClass::WriteLockClass lock(&ib_access);
            unsigned short *indexes = lock.Get_Index_Array();

            for (int i = 0; i < 12; i++) {
                indexes[i] = g_boxFaces[i].I;
                indexes[i + 1] = g_boxFaces[i].J;
                indexes[i + 2] = g_boxFaces[i].K;
            }
        }

        DX8Wrapper::Set_Material(g_boxMaterial);
        DX8Wrapper::Set_Shader(g_boxShader);
        DX8Wrapper::Set_Texture(0, nullptr);
        DX8Wrapper::Set_Index_Buffer(ib_access, 0);
        DX8Wrapper::Set_Vertex_Buffer(vb_access);
        SphereClass s;
        Get_Obj_Space_Bounding_Sphere(s);
        DX8Wrapper::Draw_Triangles(2, 0, 12, 0, 8);
    }
#endif
}

void BoxRenderObjClass::Vis_Render_Box(RenderInfoClass &rinfo, const Vector3 &center, const Vector3 &extent)
{
    captainslog_dbgassert(0, "vis not supported");
}

OBBoxRenderObjClass::OBBoxRenderObjClass(const OBBoxRenderObjClass &src)
{
    *this = src;
}

OBBoxRenderObjClass::OBBoxRenderObjClass(const W3dBoxStruct &src) : BoxRenderObjClass(src)
{
    Update_Cached_Box();
}

OBBoxRenderObjClass::OBBoxRenderObjClass()
{
    Update_Cached_Box();
}

OBBoxRenderObjClass &OBBoxRenderObjClass::operator=(const OBBoxRenderObjClass &that)
{
    if (this != &that) {
        BoxRenderObjClass::operator=(that);
        m_cachedBox = that.m_cachedBox;
    }
    return *this;
}

OBBoxRenderObjClass::OBBoxRenderObjClass(const OBBoxClass &src)
{
    m_objSpaceCenter = Vector3(0, 0, 0);
    m_objSpaceExtent = src.m_extent;
    Matrix3D m;
    m.Set(src.m_basis, src.m_center);
    Set_Transform(m);
    Update_Cached_Box();
}

RenderObjClass *OBBoxRenderObjClass::Clone() const
{
    return new OBBoxRenderObjClass(*this);
}

int OBBoxRenderObjClass::Class_ID() const
{
    return CLASSID_OBBOX;
}

void OBBoxRenderObjClass::Render(RenderInfoClass &rinfo)
{
#ifdef BUILD_WITH_D3D8
    DX8Wrapper::Set_Transform(D3DTS_WORLD, Get_Transform());
    Render_Box(rinfo, m_objSpaceCenter, m_objSpaceExtent);
#endif
}

void OBBoxRenderObjClass::Special_Render(SpecialRenderInfoClass &rinfo)
{
    if (rinfo.m_renderType == SpecialRenderInfoClass::RENDER_VIS) {
        captainslog_dbgassert(0, "vis not supported");
    }
}

void OBBoxRenderObjClass::Set_Transform(const Matrix3D &tm)
{
    RenderObjClass::Set_Transform(tm);
    Update_Cached_Box();
}

void OBBoxRenderObjClass::Set_Position(const Vector3 &pos)
{
    RenderObjClass::Set_Position(pos);
    Update_Cached_Box();
}

bool OBBoxRenderObjClass::Cast_Ray(RayCollisionTestClass &col)
{
    if ((Get_Collision_Type() & col.m_collisionType) == 0 || Is_Animation_Hidden() || col.m_result->start_bad
        || !CollisionMath::Collide(col.m_ray, m_cachedBox, col.m_result)) {
        return false;
    }

    col.m_collidedRenderObj = this;
    return true;
}

bool OBBoxRenderObjClass::Cast_AABox(AABoxCollisionTestClass &col)
{
    if ((Get_Collision_Type() & col.m_collisionType) == 0) {
        return false;
    }

    if (col.m_result->start_bad) {
        return false;
    }

    Vector3 v = Vector3(0, 0, 0);

    if (!CollisionMath::Collide(col.m_box, col.m_move, m_cachedBox, v, col.m_result)) {
        return false;
    }

    col.m_collidedRenderObj = this;
    return true;
}

bool OBBoxRenderObjClass::Cast_OBBox(OBBoxCollisionTestClass &col)
{
    if ((Get_Collision_Type() & col.m_collisionType) == 0) {
        return false;
    }

    if (col.m_result->start_bad) {
        return false;
    }

    Vector3 v = Vector3(0, 0, 0);

    if (!CollisionMath::Collide(col.m_box, col.m_move, m_cachedBox, v, col.m_result)) {
        return false;
    }

    col.m_collidedRenderObj = this;
    return true;
}

bool OBBoxRenderObjClass::Intersect_AABox(AABoxIntersectionTestClass &col)
{
    if ((Get_Collision_Type() & col.m_collisionType) != 0) {
        return CollisionMath::Intersection_Test(m_cachedBox, col.m_box);
    }

    return false;
}

bool OBBoxRenderObjClass::Intersect_OBBox(OBBoxIntersectionTestClass &col)
{
    if ((Get_Collision_Type() & col.m_collisionType) != 0) {
        return CollisionMath::Intersection_Test(m_cachedBox, col.m_box);
    }
    return false;
}

void OBBoxRenderObjClass::Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const
{
    sphere.Init(m_objSpaceCenter, m_objSpaceExtent.Length());
}

void OBBoxRenderObjClass::Get_Obj_Space_Bounding_Box(AABoxClass &box) const
{
    box.Init(m_objSpaceCenter, m_objSpaceExtent);
}

void OBBoxRenderObjClass::Update_Cached_Box()
{
    Matrix3D::Transform_Vector(m_transform, m_objSpaceCenter, &m_cachedBox.m_center);
    m_cachedBox.m_extent = m_objSpaceExtent;
    m_cachedBox.m_basis = m_transform;
}

AABoxRenderObjClass::AABoxRenderObjClass(const AABoxRenderObjClass &src)
{
    *this = src;
}

AABoxRenderObjClass::AABoxRenderObjClass(const W3dBoxStruct &src) : BoxRenderObjClass(src)
{
    Update_Cached_Box();
}

AABoxRenderObjClass::AABoxRenderObjClass()
{
    Update_Cached_Box();
}

AABoxRenderObjClass &AABoxRenderObjClass::operator=(const AABoxRenderObjClass &that)
{
    if (this != &that) {
        BoxRenderObjClass::operator=(that);
        m_cachedBox = that.m_cachedBox;
    }
    return *this;
}

AABoxRenderObjClass::AABoxRenderObjClass(const AABoxClass &src)
{
    m_objSpaceCenter = Vector3(0, 0, 0);
    m_objSpaceExtent = src.m_extent;
    Set_Position(src.m_center);
    Update_Cached_Box();
}

RenderObjClass *AABoxRenderObjClass::Clone() const
{
    return new AABoxRenderObjClass(*this);
}

int AABoxRenderObjClass::Class_ID() const
{
    return CLASSID_AABOX;
}

void AABoxRenderObjClass::Render(RenderInfoClass &rinfo)
{
#ifdef BUILD_WITH_D3D8
    Matrix3D m(true);
    m.Translate(m_transform.Get_Translation());
    DX8Wrapper::Set_Transform(D3DTS_WORLD, m);
    Render_Box(rinfo, m_objSpaceCenter, m_objSpaceExtent);
#endif
}

void AABoxRenderObjClass::Special_Render(SpecialRenderInfoClass &rinfo)
{
    if (rinfo.m_renderType == SpecialRenderInfoClass::RENDER_VIS) {
        captainslog_dbgassert(0, "vis not supported");
    }
}

void AABoxRenderObjClass::Set_Transform(const Matrix3D &tm)
{
    RenderObjClass::Set_Transform(tm);
    Update_Cached_Box();
}

void AABoxRenderObjClass::Set_Position(const Vector3 &pos)
{
    RenderObjClass::Set_Position(pos);
    Update_Cached_Box();
}

bool AABoxRenderObjClass::Cast_Ray(RayCollisionTestClass &col)
{
    if ((Get_Collision_Type() & col.m_collisionType) == 0 || Is_Animation_Hidden() || col.m_result->start_bad
        || !CollisionMath::Collide(col.m_ray, m_cachedBox, col.m_result)) {
        return false;
    }

    col.m_collidedRenderObj = this;
    return true;
}

bool AABoxRenderObjClass::Cast_AABox(AABoxCollisionTestClass &col)
{
    if ((Get_Collision_Type() & col.m_collisionType) == 0 || col.m_result->start_bad
        || !CollisionMath::Collide(col.m_box, col.m_move, m_cachedBox, col.m_result)) {
        return false;
    }

    col.m_collidedRenderObj = this;
    return true;
}

bool AABoxRenderObjClass::Cast_OBBox(OBBoxCollisionTestClass &col)
{
    if ((Get_Collision_Type() & col.m_collisionType) == 0) {
        return false;
    }

    if (col.m_result->start_bad) {
        return false;
    }

    Vector3 v = Vector3(0, 0, 0);

    if (!CollisionMath::Collide(col.m_box, col.m_move, m_cachedBox, v, col.m_result)) {
        return false;
    }

    col.m_collidedRenderObj = this;
    return true;
}

bool AABoxRenderObjClass::Intersect_AABox(AABoxIntersectionTestClass &col)
{
    if ((Get_Collision_Type() & col.m_collisionType) != 0) {
        return CollisionMath::Intersection_Test(m_cachedBox, col.m_box);
    }

    return false;
}

bool AABoxRenderObjClass::Intersect_OBBox(OBBoxIntersectionTestClass &col)
{
    if ((Get_Collision_Type() & col.m_collisionType) != 0) {
        return CollisionMath::Intersection_Test(m_cachedBox, col.m_box);
    }
    return false;
}

void AABoxRenderObjClass::Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const
{
    sphere.Init(m_objSpaceCenter, m_objSpaceExtent.Length());
}

void AABoxRenderObjClass::Get_Obj_Space_Bounding_Box(AABoxClass &box) const
{
    box.Init(m_objSpaceCenter, m_objSpaceExtent);
}

void AABoxRenderObjClass::Update_Cached_Box()
{
    m_cachedBox.m_center = m_objSpaceCenter + m_transform.Get_Translation();
    m_cachedBox.m_extent = m_objSpaceExtent;
}

PrototypeClass *BoxLoaderClass::Load_W3D(ChunkLoadClass &cload)
{
    W3dBoxStruct box;
    cload.Read(&box, sizeof(box));
    return new BoxPrototypeClass(box);
}

void BoxRenderObjClass::Set_Name(const char *name)
{
    captainslog_assert(name != nullptr);
    captainslog_assert(strlen(name) < 32);
    strcpy(m_name, name);
}
