/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Mesh Class
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "mesh.h"
#include "aabtree.h"
#include "aaplane.h"
#include "assetmgr.h"
#include "camera.h"
#include "chunkio.h"
#include "coltest.h"
#include "decalmsh.h"
#include "dx8indexbuffer.h"
#include "dx8polygonrenderer.h"
#include "dx8renderer.h"
#include "htree.h"
#include "inttest.h"
#include "matinfo.h"
#include "matpass.h"
#include "meshgeometry.h"
#include "meshmdl.h"
#include "proto.h"
#include "rinfo.h"
#include "shader.h"
#include "texture.h"
#include "tri.h"
#include "vertmaterial.h"
#include "w3d.h"
#include "w3d_file.h"
#include "w3derr.h"

static unsigned int g_meshDebugIdCount;
static SimpleDynVecClass<uint32_t> g_tempApt;

MeshClass::MeshClass() :
    m_model(nullptr),
    m_decalMesh(nullptr),
    m_lightEnvironment(nullptr),
    m_alphaOverride(1.0f),
    m_emissiveScale(1.0f),
    m_opacityOverride(1.0f),
    m_baseVertexOffset(0),
    m_nextVisibleSkin(nullptr),
    m_meshDebugId(g_meshDebugIdCount++),
    m_isDisabledByDebugger(false)
{
}

MeshClass::MeshClass(const MeshClass &that) :
    RenderObjClass(that),
    m_model(nullptr),
    m_decalMesh(nullptr),
    m_lightEnvironment(nullptr),
    m_alphaOverride(1.0f),
    m_emissiveScale(1.0f),
    m_opacityOverride(1.0f),
    m_baseVertexOffset(that.m_baseVertexOffset),
    m_nextVisibleSkin(nullptr),
    m_meshDebugId(g_meshDebugIdCount++),
    m_isDisabledByDebugger(false)
{
    MeshModelClass *model = const_cast<MeshModelClass *>(that.m_model);
    Ref_Ptr_Set(m_model, model);
}

MeshClass &MeshClass::operator=(const MeshClass &that)
{
    if (this != &that) {
        RenderObjClass::operator=(that);
        MeshModelClass *model = const_cast<MeshModelClass *>(that.m_model);
        Ref_Ptr_Set(m_model, model);
        m_baseVertexOffset = that.m_baseVertexOffset;

        Ref_Ptr_Release(m_decalMesh);
        m_lightEnvironment = nullptr;
    }

    return *this;
}

MeshClass::~MeshClass()
{
    Free();
}

bool MeshClass::Contains(const Vector3 &point)
{
    Vector3 obj_point;
    Matrix3D::Inverse_Transform_Vector(m_transform, point, &obj_point);
    return m_model->Contains(obj_point);
}

void MeshClass::Free()
{
    Ref_Ptr_Release(m_model);
    Ref_Ptr_Release(m_decalMesh);
}

RenderObjClass *MeshClass::Clone() const
{
    return new MeshClass(*this);
}

const char *MeshClass::Get_Name() const
{
    return m_model->Get_Name();
}

void MeshClass::Set_Name(const char *name)
{
    m_model->Set_Name(name);
}

uint32_t MeshClass::Get_W3D_Flags()
{
    return m_model->m_w3dAttributes;
}

const char *MeshClass::Get_User_Text() const
{
    return m_model->Get_User_Text();
}

MaterialInfoClass *MeshClass::Get_Material_Info()
{
    if (m_model != nullptr) {
        if (m_model->m_matInfo != nullptr) {
            m_model->m_matInfo->Add_Ref();
            return m_model->m_matInfo;
        }
    }

    return nullptr;
}

MeshModelClass *MeshClass::Get_Model()
{
    if (m_model != nullptr) {
        m_model->Add_Ref();
    }

    return m_model;
}

void MeshClass::Scale(float scale)
{
    if (scale == 1.0f) {
        return;
    }

    Vector3 sc;
    sc.X = sc.Y = sc.Z = scale;
    Make_Unique(false);
    m_model->Make_Geometry_Unique();
    m_model->Scale(sc);

    Invalidate_Cached_Bounding_Volumes();
    RenderObjClass *container = Get_Container();

    if (container != nullptr) {
        container->Update_Obj_Space_Bounding_Volumes();
    }
}

void MeshClass::Scale(float scalex, float scaley, float scalez)
{
    Vector3 sc;
    sc.X = scalex;
    sc.Y = scaley;
    sc.Z = scalez;
    Make_Unique(false);
    m_model->Make_Geometry_Unique();
    m_model->Scale(sc);

    Invalidate_Cached_Bounding_Volumes();
    RenderObjClass *container = Get_Container();

    if (container != nullptr) {
        container->Update_Obj_Space_Bounding_Volumes();
    }
}

void MeshClass::Get_Deformed_Vertices(Vector3 *dst_vert, Vector3 *dst_norm)
{
    captainslog_assert(m_model->Get_Flag(MeshGeometryClass::SKIN));
    m_model->Get_Deformed_Vertices(dst_vert, dst_norm, m_container->Get_HTree());
}

void MeshClass::Get_Deformed_Vertices(Vector3 *dst_vert)
{
    captainslog_assert(m_model->Get_Flag(MeshGeometryClass::SKIN));
    captainslog_assert(m_container != nullptr);
    captainslog_assert(m_container->Get_HTree() != nullptr);

    m_model->Get_Deformed_Vertices(dst_vert, m_container->Get_HTree());
}

void MeshClass::Create_Decal(DecalGeneratorClass *generator)
{
    captainslog_dbgassert(0, "decals not supported");
}

void MeshClass::Delete_Decal(unsigned long decal_id)
{
    captainslog_dbgassert(0, "decals not supported");
}

int MeshClass::Get_Num_Polys() const
{
    if (m_model != nullptr) {
        int num_passes = m_model->Get_Pass_Count();
        captainslog_assert(num_passes > 0);
        int poly_count = m_model->Get_Polygon_Count();
        return num_passes * poly_count;
    } else {
        return 0;
    }
}

void MeshClass::Render(RenderInfoClass &rinfo)
{
    if (Is_Not_Hidden_At_All() == false) {
        return;
    }

    unsigned int sort_level = (unsigned int)m_model->Get_Sort_Level();

    if (W3D::Are_Static_Sort_Lists_Enabled() && sort_level != SORT_LEVEL_NONE) {
        Set_Lighting_Environment(rinfo.m_lightEnvironment);
        m_alphaOverride = rinfo.m_alphaOverride;
        m_opacityOverride = rinfo.m_opacityOverride;
        m_emissiveScale = rinfo.m_emissiveScale;
        W3D::Add_To_Static_Sort_List(this, sort_level);
    } else {
        const FrustumClass &frustum = rinfo.m_camera.Get_Frustum();

        if (m_model->Get_Flag(MeshGeometryClass::SKIN)
            || CollisionMath::Overlap_Test(frustum, Get_Bounding_Box()) != CollisionMath::OUTSIDE) {
            bool rendered_something = false;

            if (m_model->m_polygonRendererList.Is_Empty()) {
                m_model->Register_For_Rendering();
                captainslog_assert(!m_model->m_polygonRendererList.Is_Empty());
            }

            if (sort_level == SORT_LEVEL_NONE) {
                Set_Lighting_Environment(rinfo.m_lightEnvironment);
                m_alphaOverride = rinfo.m_alphaOverride;
                m_opacityOverride = rinfo.m_opacityOverride;
                m_emissiveScale = rinfo.m_emissiveScale;
            }

            DX8FVFCategoryContainer *fvf_container =
                m_model->m_polygonRendererList.Peek_Head()->Get_Texture_Category()->Get_Container();

            bool render_base_passes =
                ((rinfo.Current_Override_Flags() & RenderInfoClass::RINFO_OVERRIDE_ADDITIONAL_PASSES_ONLY) == 0);
            bool is_alpha = (m_model->Get_Single_Shader().Get_Alpha_Test() == ShaderClass::ALPHATEST_ENABLE)
                || (m_model->Get_Single_Shader().Get_Src_Blend_Func() == ShaderClass::SRCBLEND_SRC_ALPHA);

            if ((rinfo.Current_Override_Flags() & RenderInfoClass::RINFO_OVERRIDE_SHADOW_RENDERING) && (is_alpha == true)) {
                render_base_passes = true;
            }

            if (render_base_passes) {
                MultiListIterator<DX8PolygonRendererClass> it(&(m_model->m_polygonRendererList));

                while (!it.Is_Done()) {
                    DX8PolygonRendererClass *polygon_renderer = it.Peek_Obj();
                    polygon_renderer->Get_Texture_Category()->Add_Render_Task(polygon_renderer, this);
                    it.Next();
                }

                rendered_something = true;
            }

            for (int i = 0; i < rinfo.Additional_Pass_Count(); i++) {

                MaterialPassClass *matpass = rinfo.Peek_Additional_Pass(i);

                if ((!Is_Translucent()) || (matpass->Is_Enabled_On_Translucent_Meshes())) {
                    if (rinfo.Current_Override_Flags() & RenderInfoClass::RINFO_OVERRIDE_ADDITIONAL_PASSES_ONLY) {
                        fvf_container->Add_Delayed_Visible_Material_Pass(matpass, this);
                    } else {
                        fvf_container->Add_Visible_Material_Pass(matpass, this);
                    }

                    rendered_something = true;
                }
            }

            if (rendered_something && m_model->Get_Flag(MeshGeometryClass::SKIN)) {
                static_cast<DX8SkinFVFCategoryContainer *>(fvf_container)->Add_Visible_Skin(this);
            }
        }
    }
}

void MeshClass::Render_Material_Pass(MaterialPassClass *pass, IndexBufferClass *ib)
{
#ifdef BUILD_WITH_D3D8
    Vector3 emissive(-1.0f, -1.0f, -1.0f);
    float opacity = -1.0f;

    if (m_lightEnvironment != nullptr) {
        DX8Wrapper::Set_Light_Environment(m_lightEnvironment);
    }

    if (m_model->Get_Flag(MeshModelClass::SKIN)) {

        if (m_opacityOverride != 1.0f) {
            VertexMaterialClass *m = pass->Get_Material();

            if (m != nullptr) {
                opacity = m->Get_Opacity();
                m->Set_Opacity(m_opacityOverride);
            }
        }

        if (m_emissiveScale != 1.0f) {
            VertexMaterialClass *m = pass->Get_Material();

            if (m != nullptr) {
                m->Get_Emissive(&emissive);
                m->Set_Emissive(emissive * m_emissiveScale);
            }
        }

        pass->Install_Materials();
        DX8Wrapper::Set_Index_Buffer(ib, 0);
        DX8Wrapper::Set_World_Identity();

        MultiListIterator<DX8PolygonRendererClass> it(&(m_model->m_polygonRendererList));
        while (!it.Is_Done()) {
            if (!it.Peek_Obj()->Get_Pass()) {
                it.Peek_Obj()->Render(m_baseVertexOffset);
            }

            it.Next();
        }

        if (opacity >= 0) {
            pass->Get_Material()->Set_Opacity(opacity);
        }

        if (emissive.X >= 0) {
            pass->Get_Material()->Set_Emissive(emissive);
        }

        pass->UnInstall_Materials();

    } else if ((pass->Get_Cull_Volume() != nullptr) && (MaterialPassClass::Is_Per_Polygon_Culling_Enabled())) {

        g_tempApt.Delete_All(false);

        Matrix3D modeltminv;
        Get_Transform().Get_Orthogonal_Inverse(modeltminv);

        OBBoxClass localbox;
        OBBoxClass::Transform(modeltminv, *(pass->Get_Cull_Volume()), &localbox);

        Vector3 view_dir;
        localbox.m_basis.Get_Z_Vector(&view_dir);
        view_dir = -view_dir;

        if (m_model->Has_Cull_Tree()) {
            m_model->Generate_Rigid_APT(localbox, view_dir, g_tempApt);
        } else {
            m_model->Generate_Rigid_APT(view_dir, g_tempApt);
        }

        if (g_tempApt.Count() > 0) {

            int buftype = DX8IndexBufferClass::BUFFER_TYPE_DYNAMIC_DX8;
            if (m_model->Get_Flag(MeshGeometryClass::SORT) && W3D::Is_Sorting_Enabled()) {
                buftype = DX8IndexBufferClass::BUFFER_TYPE_DYNAMIC_SORTING;
            }

            int min_v = m_model->Get_Vertex_Count();
            int max_v = 0;

            DynamicIBAccessClass dynamic_ib(buftype, g_tempApt.Count() * 3);
            {
                DynamicIBAccessClass::WriteLockClass lock(&dynamic_ib);
                unsigned short *indices = lock.Get_Index_Array();
                const TriIndex *polys = m_model->Get_Polygon_Array();

                for (int i = 0; i < g_tempApt.Count(); i++) {
                    unsigned v0 = polys[g_tempApt[i]].I;
                    unsigned v1 = polys[g_tempApt[i]].J;
                    unsigned v2 = polys[g_tempApt[i]].K;

                    indices[i * 3 + 0] = (unsigned short)v0;
                    indices[i * 3 + 1] = (unsigned short)v1;
                    indices[i * 3 + 2] = (unsigned short)v2;

                    min_v = GameMath::Min(v0, min_v);
                    min_v = GameMath::Min(v1, min_v);
                    min_v = GameMath::Min(v2, min_v);

                    max_v = GameMath::Max(v0, max_v);
                    max_v = GameMath::Max(v1, max_v);
                    max_v = GameMath::Max(v2, max_v);
                }
            }

            int vertex_offset = m_model->m_polygonRendererList.Peek_Head()->Get_Vertex_Offset();
            pass->Install_Materials();

            DX8Wrapper::Set_Transform(D3DTS_WORLD, Get_Transform());
            DX8Wrapper::Set_Index_Buffer(dynamic_ib, vertex_offset);

            DX8Wrapper::Draw_Triangles(0, g_tempApt.Count(), min_v, max_v - min_v + 1);
            pass->UnInstall_Materials();
        }
    } else {

        if (m_opacityOverride != 1.0f) {
            VertexMaterialClass *m = pass->Get_Material();

            if (m != nullptr) {
                opacity = m->Get_Opacity();
                m->Set_Opacity(m_opacityOverride);
            }
        }

        if (m_emissiveScale != 1.0f) {
            VertexMaterialClass *m = pass->Get_Material();

            if (m != nullptr) {
                m->Get_Emissive(&emissive);
                m->Set_Emissive(emissive * m_emissiveScale);
            }
        }

        pass->Install_Materials();
        DX8Wrapper::Set_Index_Buffer(ib, 0);
        DX8Wrapper::Set_Transform(D3DTS_WORLD, m_transform);

        MultiListIterator<DX8PolygonRendererClass> it(&(m_model->m_polygonRendererList));
        while (!it.Is_Done()) {
            if (!it.Peek_Obj()->Get_Pass()) {
                it.Peek_Obj()->Render(m_baseVertexOffset);
            }

            it.Next();
        }

        if (opacity >= 0) {
            pass->Get_Material()->Set_Opacity(opacity);
        }

        if (emissive.X >= 0) {
            pass->Get_Material()->Set_Emissive(emissive);
        }

        pass->UnInstall_Materials();
    }
#endif
}

void MeshClass::Special_Render(SpecialRenderInfoClass &rinfo)
{
    if ((Is_Not_Hidden_At_All() == false) && (rinfo.m_renderType != SpecialRenderInfoClass::RENDER_SHADOW)) {
        return;
    }

    if (rinfo.m_renderType == SpecialRenderInfoClass::RENDER_VIS) {
        captainslog_dbgassert(0, "vis not supported");
    }

    if (rinfo.m_renderType == SpecialRenderInfoClass::RENDER_SHADOW) {
        const HTreeClass *htree = nullptr;
        if (m_container != nullptr) {
            htree = m_container->Get_HTree();
        }

        m_model->Shadow_Render(rinfo, m_transform, htree);
    }
}

void MeshClass::Replace_Texture(TextureClass *texture, TextureClass *new_texture)
{
    m_model->Replace_Texture(texture, new_texture);
}

void MeshClass::Replace_VertexMaterial(VertexMaterialClass *vmat, VertexMaterialClass *new_vmat)
{
    m_model->Replace_VertexMaterial(vmat, new_vmat);
}

void MeshClass::Make_Unique(bool force)
{
    if (m_model->Num_Refs() == 1 && !force) {
        return;
    }

    MeshModelClass *newmesh = new MeshModelClass(*m_model);
    Ref_Ptr_Set(m_model, newmesh);
    Ref_Ptr_Release(newmesh);
}

W3DErrorType MeshClass::Load_W3D(ChunkLoadClass &cload)
{
    Vector3 boxmin, boxmax;
    Free();
    m_model = new MeshModelClass();

    if (m_model == nullptr) {
        captainslog_error("MeshClass::Load - Failed to allocate model");
        return W3D_ERROR_LOAD_FAILED;
    }

    if (m_model->Load_W3D(cload) != W3D_ERROR_OK) {
        Free();
        return W3D_ERROR_LOAD_FAILED;
    }

    int col_bits = (m_model->m_w3dAttributes & W3D_MESH_FLAG_COLLISION_TYPE_MASK) >> W3D_MESH_FLAG_COLLISION_TYPE_SHIFT;
    Set_Collision_Type(col_bits << 1);
    Set_Hidden(m_model->m_w3dAttributes & W3D_MESH_FLAG_HIDDEN);

    int is_translucent = m_model->Get_Flag(MeshModelClass::SORT);
    int is_alpha = 0;
    int is_additive = 0;

    if (m_model->Has_Shader_Array(0)) {
        for (int i = 0; i < m_model->Get_Polygon_Count(); i++) {
            ShaderClass shader = m_model->Get_Shader(i, 0);
            is_translucent |= (shader.Get_Alpha_Test() == ShaderClass::ALPHATEST_ENABLE);
            is_alpha |= (shader.Get_Dst_Blend_Func() != ShaderClass::DSTBLEND_ZERO
                            || shader.Get_Src_Blend_Func() != ShaderClass::SRCBLEND_ONE)
                && shader.Get_Alpha_Test() != ShaderClass::ALPHATEST_ENABLE;
            is_additive |= shader.Get_Dst_Blend_Func() == ShaderClass::DSTBLEND_ONE
                && shader.Get_Src_Blend_Func() == ShaderClass::SRCBLEND_ONE;
        }
    } else {
        ShaderClass shader = m_model->Get_Single_Shader(0);
        is_translucent |= (shader.Get_Alpha_Test() == ShaderClass::ALPHATEST_ENABLE);
        is_alpha |= (shader.Get_Dst_Blend_Func() != ShaderClass::DSTBLEND_ZERO
                        || shader.Get_Src_Blend_Func() != ShaderClass::SRCBLEND_ONE)
            && shader.Get_Alpha_Test() != ShaderClass::ALPHATEST_ENABLE;
        is_additive |= shader.Get_Dst_Blend_Func() == ShaderClass::DSTBLEND_ONE
            && shader.Get_Src_Blend_Func() == ShaderClass::SRCBLEND_ONE;
    }

    Set_Translucent(is_translucent);
    Set_Alpha(is_alpha);
    Set_Additive(is_additive);

    return W3D_ERROR_OK;
}

bool MeshClass::Cast_Ray(RayCollisionTestClass &raytest)
{
    if ((Get_Collision_Type() & raytest.m_collisionType) == 0) {
        return false;
    }

    if (raytest.m_checkAlpha && Is_Alpha() != 0) {
        return false;
    }

    if (raytest.m_allowHidden && Is_Hidden() != 0) {
        return false;
    }

    if (Is_Animation_Hidden()) {
        return false;
    }

    if (raytest.m_result->start_bad) {
        return false;
    }

    Matrix3D world_to_obj;
    Matrix3D world = Get_Transform();

    if (m_model->Get_Flag(MeshModelClass::ALIGNED)) {
        Vector3 mesh_position;
        world.Get_Translation(&mesh_position);
        world.Obj_Look_At(mesh_position, mesh_position - raytest.m_ray.Get_Dir(), 0.0f);
    } else if (m_model->Get_Flag(MeshModelClass::ORIENTED)) {
        Vector3 mesh_position;
        world.Get_Translation(&mesh_position);
        world.Obj_Look_At(mesh_position, raytest.m_ray.Get_P0(), 0.0f);
    }

    world.Get_Inverse(world_to_obj);
    RayCollisionTestClass objray(raytest, world_to_obj);

    captainslog_assert(m_model);

    bool hit = m_model->Cast_Ray(objray);

    if (hit) {
        raytest.m_collidedRenderObj = this;
        Matrix3D::Rotate_Vector(world, raytest.m_result->normal, &(raytest.m_result->normal));

        if (raytest.m_result->compute_contact_point) {
            Matrix3D::Transform_Vector(world, raytest.m_result->contact_point, &(raytest.m_result->contact_point));
        }
    }

    return hit;
}

bool MeshClass::Cast_AABox(AABoxCollisionTestClass &boxtest)
{
    if ((Get_Collision_Type() & boxtest.m_collisionType) == 0) {
        return false;
    }

    if (boxtest.m_result->start_bad)
        return false;

    captainslog_assert(m_model);

    bool hit = m_model->Cast_World_Space_AABox(boxtest, Get_Transform());

    if (hit) {
        boxtest.m_collidedRenderObj = this;
    }

    return hit;
}

bool MeshClass::Cast_OBBox(OBBoxCollisionTestClass &boxtest)
{
    if ((Get_Collision_Type() & boxtest.m_collisionType) == 0) {
        return false;
    }

    if (boxtest.m_result->start_bad) {
        return false;
    }

    const Matrix3D &tm = Get_Transform();
    Matrix3D world_to_obj;
    tm.Get_Orthogonal_Inverse(world_to_obj);
    OBBoxCollisionTestClass localtest(boxtest, world_to_obj);

    captainslog_assert(m_model);

    bool hit = m_model->Cast_OBBox(localtest);

    if (hit) {
        boxtest.m_collidedRenderObj = this;
        Matrix3D::Rotate_Vector(tm, boxtest.m_result->normal, &(boxtest.m_result->normal));

        if (boxtest.m_result->compute_contact_point) {
            Matrix3D::Transform_Vector(tm, boxtest.m_result->contact_point, &(boxtest.m_result->contact_point));
        }
    }

    return hit;
}

bool MeshClass::Intersect_AABox(AABoxIntersectionTestClass &boxtest)
{
    if ((Get_Collision_Type() & boxtest.m_collisionType) == 0) {
        return false;
    }

    Matrix3D inv_tm;
    Get_Transform().Get_Orthogonal_Inverse(inv_tm);
    OBBoxIntersectionTestClass local_test(boxtest, inv_tm);
    captainslog_assert(m_model);
    return m_model->Intersect_OBBox(local_test);
}

bool MeshClass::Intersect_OBBox(OBBoxIntersectionTestClass &boxtest)
{
    if ((Get_Collision_Type() & boxtest.m_collisionType) == 0)
        return false;

    Matrix3D inv_tm;
    Get_Transform().Get_Orthogonal_Inverse(inv_tm);
    OBBoxIntersectionTestClass local_test(boxtest, inv_tm);
    captainslog_assert(m_model);
    return m_model->Intersect_OBBox(local_test);
}

void MeshClass::Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const
{
    if (m_model != nullptr) {
        m_model->Get_Bounding_Sphere(&sphere);
    } else {
        sphere.Init(Vector3(0, 0, 0), 1);
    }
}

void MeshClass::Get_Obj_Space_Bounding_Box(AABoxClass &box) const
{
    if (m_model != nullptr) {
        m_model->Get_Bounding_Box(&box);
    } else {
        box.Init(Vector3(0, 0, 0), Vector3(1, 1, 1));
    }
}

void MeshClass::Generate_Culling_Tree()
{
    m_model->Generate_Culling_Tree();
}

void MeshClass::Add_Dependencies_To_List(DynamicVectorClass<StringClass> &file_list, bool textures_only)
{
    MaterialInfoClass *material = Get_Material_Info();
    if (material != nullptr) {
        for (int index = 0; index < material->Texture_Count(); index++) {
            TextureClass *texture = material->Peek_Texture(index);

            if (texture != nullptr) {
                file_list.Add(texture->Get_Full_Path());
            }
        }

        material->Release_Ref();
    }

    RenderObjClass::Add_Dependencies_To_List(file_list, textures_only);
}

void MeshClass::Update_Cached_Bounding_Volumes() const
{
    Get_Obj_Space_Bounding_Sphere(m_cachedBoundingSphere);

    m_cachedBoundingSphere.Center = Get_Transform() * m_cachedBoundingSphere.Center;

    if (m_model->Get_Flag(MeshModelClass::ALIGNED) || m_model->Get_Flag(MeshModelClass::ORIENTED)) {
        m_cachedBoundingBox.m_center = m_cachedBoundingSphere.Center;
        m_cachedBoundingBox.m_extent.Set(
            m_cachedBoundingSphere.Radius, m_cachedBoundingSphere.Radius, m_cachedBoundingSphere.Radius);
    } else {
        Get_Obj_Space_Bounding_Box(m_cachedBoundingBox);
        m_cachedBoundingBox.Transform(Get_Transform());
    }

    Validate_Cached_Bounding_Volumes();
}

int MeshClass::Get_Sort_Level() const
{
    if (m_model != nullptr) {
        return (m_model->Get_Sort_Level());
    }

    return SORT_LEVEL_NONE;
}

void MeshClass::Set_Sort_Level(int level)
{
    if (m_model != nullptr) {
        m_model->Set_Sort_Level(level);
    }
}

int MeshClass::Get_Draw_Call_Count() const
{
    if (m_model == nullptr) {
        return 0;
    }

    int count = m_model->m_polygonRendererList.Count();

    if (count <= 0) {
        if (m_model->m_matInfo && m_model->m_matInfo->Texture_Count() > 0) {
            count = m_model->m_matInfo->Texture_Count();
        } else {
            count = 1;
        }
    }

    return count;
}

PrototypeClass *MeshLoaderClass::Load_W3D(ChunkLoadClass &cload)
{
    MeshClass *mesh = new MeshClass();

    if (mesh->Load_W3D(cload) != W3D_ERROR_OK) {
        mesh->Release_Ref();
        return nullptr;
    }

    PrimitivePrototypeClass *proto = new PrimitivePrototypeClass(mesh);
    mesh->Release_Ref();
    return proto;
}

int MeshClass::Get_Draw_Call_Count()
{
    if (!m_model) {
        return 0;
    }

    int count = m_model->m_polygonRendererList.Count();

    if (count <= 0) {
        if (!m_model->m_matInfo) {
            return 1;
        }

        count = m_model->m_matInfo->Texture_Count();

        if (count <= 0) {
            return 1;
        }
    }

    return count;
}
