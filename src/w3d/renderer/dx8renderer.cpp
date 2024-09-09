/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Mesh Renderer
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "dx8renderer.h"
#include "camera.h"
#include "decalmsh.h"
#include "dx8fvf.h"
#include "dx8indexbuffer.h"
#include "dx8polygonrenderer.h"
#include "dx8vertexbuffer.h"
#include "mapper.h"
#include "matpass.h"
#include "mesh.h"
#include "w3d.h"
#ifndef GAME_DLL
DX8MeshRendererClass g_theDX8MeshRenderer;
DynamicVectorClass<Vector3> g_tempVertexBuffer;
DynamicVectorClass<Vector3> g_tempNormalBuffer;
MultiListClass<DX8TextureCategoryClass> g_textureCategoryDeleteList;
MultiListClass<DX8FVFCategoryContainer> g_fvfCategoryContainerDeleteList;
MultiListClass<MeshModelClass> g_registeredMeshList;
bool DX8TextureCategoryClass::s_forceMultiply;
#else
#include "hooker.h"
#endif

bool Compare_Materials(const VertexMaterialClass *a, const VertexMaterialClass *b)
{
    int crc1;
    int crc2;

    if (a != nullptr) {
        crc1 = a->Get_CRC();
    } else {
        crc1 = 0;
    }

    if (b != nullptr) {
        crc2 = b->Get_CRC();
    } else {
        crc2 = 0;
    }

    return crc1 == crc2;
}

struct Textures_Material_And_Shader_Booking_Struct
{
    Textures_Material_And_Shader_Booking_Struct() : m_addedTypeCount(0)
    {
        memset(m_addedMaterials, 0, sizeof(m_addedMaterials));
        memset(m_addedTextures, 0, sizeof(m_addedTextures));
    }

    enum
    {
        MAX_ADDED_TYPE_COUNT = 64
    };

    bool Add_Textures_Material_And_Shader(TextureClass **texs, VertexMaterialClass *mat, ShaderClass shd);
    TextureClass *m_addedTextures[2][MAX_ADDED_TYPE_COUNT];
    VertexMaterialClass *m_addedMaterials[MAX_ADDED_TYPE_COUNT];
    ShaderClass m_addedShaders[MAX_ADDED_TYPE_COUNT];
    unsigned int m_addedTypeCount;
};

bool Textures_Material_And_Shader_Booking_Struct::Add_Textures_Material_And_Shader(
    TextureClass **texs, VertexMaterialClass *mat, ShaderClass shd)
{
    for (unsigned int i = 0; i < m_addedTypeCount; i++) {
        bool b = true;

        for (int j = 0; j < ARRAY_SIZE(m_addedTextures); j++) {
            b = b && texs[j] == m_addedTextures[j][i];
        }

        if (b && Compare_Materials(mat, m_addedMaterials[i]) && shd == m_addedShaders[i]) {
            return false;
        }
    }

    captainslog_assert(m_addedTypeCount < MAX_ADDED_TYPE_COUNT);
    for (int k = 0; k < ARRAY_SIZE(m_addedTextures); k++) {
        m_addedTextures[k][m_addedTypeCount] = texs[k];
    }

    m_addedMaterials[m_addedTypeCount] = mat;
    m_addedShaders[m_addedTypeCount] = shd;
    m_addedTypeCount++;
    return true;
}

class PolyRenderTaskClass : public AutoPoolClass<PolyRenderTaskClass, 256>
{
public:
    PolyRenderTaskClass(DX8PolygonRendererClass *renderer, MeshClass *mesh) :
        m_renderer(renderer), m_mesh(mesh), m_nextVisible(nullptr)
    {
        captainslog_assert(renderer != nullptr);
        captainslog_assert(mesh != nullptr);
        m_mesh->Add_Ref();
    }
    ~PolyRenderTaskClass() { m_mesh->Release_Ref(); }

    DX8PolygonRendererClass *Peek_Polygon_Renderer() { return m_renderer; }
    MeshClass *Peek_Mesh() { return m_mesh; }

    PolyRenderTaskClass *Get_Next_Visible() { return m_nextVisible; }

    void Set_Next_Visible(PolyRenderTaskClass *visible) { m_nextVisible = visible; }

private:
    DX8PolygonRendererClass *m_renderer;
    MeshClass *m_mesh;
    PolyRenderTaskClass *m_nextVisible;
};

class MatPassTaskClass : public AutoPoolClass<MatPassTaskClass, 256>
{
public:
    MatPassTaskClass(MaterialPassClass *pass, MeshClass *mesh) : m_materialPass(pass), m_mesh(mesh), m_nextVisible(nullptr)
    {
        captainslog_assert(m_materialPass != nullptr);
        captainslog_assert(m_mesh != nullptr);
        m_materialPass->Add_Ref();
        m_mesh->Add_Ref();
    }

    ~MatPassTaskClass()
    {
        m_materialPass->Release_Ref();
        m_mesh->Release_Ref();
    }

    MaterialPassClass *Peek_Material_Pass() { return m_materialPass; }
    MeshClass *Peek_Mesh() { return m_mesh; }

    MatPassTaskClass *Get_Next_Visible() { return m_nextVisible; }

    void Set_Next_Visible(MatPassTaskClass *task) { m_nextVisible = task; }

private:
    MaterialPassClass *m_materialPass;
    MeshClass *m_mesh;
    MatPassTaskClass *m_nextVisible;
};

class Vertex_Split_Table
{
public:
    Vertex_Split_Table(MeshModelClass *mmc);
    ~Vertex_Split_Table();

    const Vector3 *Get_Vertex_Array() { return m_mmc->Get_Vertex_Array(); }
    const Vector3 *Get_Vertex_Normal_Array() { return m_mmc->Get_Vertex_Normal_Array(); }
    const unsigned int *Get_Color_Array(unsigned int array_index) { return m_mmc->Get_Color_Array(array_index, false); }
    const Vector2 *Get_UV_Array(unsigned int array_index) { return m_mmc->Get_UV_Array_By_Index(array_index); }
    unsigned int Get_Vertex_Count() { return m_mmc->Get_Vertex_Count(); }
    unsigned int Get_Polygon_Count() { return m_polygonCount; }
    unsigned int Get_Pass_Count() { return m_mmc->Get_Pass_Count(); }
    MeshModelClass *Get_Mesh_Model_Class() { return m_mmc; }
    TriIndex *Get_Polygon_Array(unsigned int) { return m_polygonArray; }

    TextureClass *Peek_Texture(unsigned int pidx, unsigned int pass, unsigned int stage);
    VertexMaterialClass *Peek_Material(unsigned int pidx, unsigned int pass);
    ShaderClass Peek_Shader(unsigned int pidx, unsigned int pass);

private:
    MeshModelClass *m_mmc;
    bool m_npatchEnable;
    unsigned int m_polygonCount;
    TriIndex *m_polygonArray;
    bool m_allocatedPolygonArray;
};

Vertex_Split_Table::Vertex_Split_Table(MeshModelClass *mmc) :
    m_mmc(mmc), m_npatchEnable(false), m_allocatedPolygonArray(false)
{
    m_polygonCount = mmc->Get_Polygon_Count();

    m_allocatedPolygonArray = true;
    m_polygonArray = new TriIndex[m_polygonCount];
    memcpy(m_polygonArray, mmc->Get_Polygon_Array(), m_mmc->Get_Polygon_Count() * sizeof(TriIndex));
}

Vertex_Split_Table::~Vertex_Split_Table()
{
    if (m_allocatedPolygonArray) {
        delete[] m_polygonArray;
    }
}

TextureClass *Vertex_Split_Table::Peek_Texture(unsigned int pidx, unsigned int pass, unsigned int stage)
{
    if (m_mmc->Has_Texture_Array(pass, stage)) {
        if (pidx < (unsigned int)m_mmc->Get_Polygon_Count()) {
            return m_mmc->Peek_Texture(pidx, pass, stage);
        } else {
            captainslog_dbgassert(false, "GapFillerClass removed");
            return nullptr;
        }
    } else {
        return m_mmc->Peek_Single_Texture(pass, stage);
    }
}

VertexMaterialClass *Vertex_Split_Table::Peek_Material(unsigned int pidx, unsigned int pass)
{
    if (m_mmc->Has_Material_Array(pass)) {
        if (pidx < (unsigned int)m_mmc->Get_Polygon_Count()) {
            return m_mmc->Peek_Material(m_mmc->Get_Polygon_Array()[pidx][0], pass);
        } else {
            captainslog_dbgassert(false, "GapFillerClass removed");
            return nullptr;
        }
    } else {
        return m_mmc->Peek_Single_Material(pass);
    }
}

ShaderClass Vertex_Split_Table::Peek_Shader(unsigned int pidx, unsigned int pass)
{
    if (m_mmc->Has_Shader_Array(pass)) {
        if (pidx < (unsigned int)m_mmc->Get_Polygon_Count()) {
            return m_mmc->Get_Shader(pidx, pass);
        } else {
            captainslog_dbgassert(false, "GapFillerClass removed");
            return ShaderClass();
        }
    } else {
        return m_mmc->Get_Single_Shader(pass);
    }
}

void DX8MeshRendererClass::Init()
{
    if (m_textureCategoryContainerListSkin == nullptr) {
        m_textureCategoryContainerListSkin = new MultiListClass<DX8FVFCategoryContainer>;
    }
}

void DX8MeshRendererClass::Shutdown()
{
    Invalidate(true);
    Clear_Pending_Delete_Lists();
    g_tempVertexBuffer.Clear();
    g_tempNormalBuffer.Clear();
}

void Invalidate_FVF_Category_Container_List(MultiListClass<DX8FVFCategoryContainer> &categoryContainer)
{
    for (DX8FVFCategoryContainer *i = categoryContainer.Remove_Head(); i != nullptr; i = categoryContainer.Remove_Head()) {
        delete i;
    }
}

void DX8MeshRendererClass::Invalidate(bool shutdown)
{
    g_registeredMeshList.Reset_List();

    for (int i = 0; i < m_textureCategoryContainerListsRigid.Count(); i++) {
        Invalidate_FVF_Category_Container_List(*m_textureCategoryContainerListsRigid[i]);
        delete m_textureCategoryContainerListsRigid[i];
    }

    if (m_textureCategoryContainerListSkin != nullptr) {
        Invalidate_FVF_Category_Container_List(*m_textureCategoryContainerListSkin);
        m_textureCategoryContainerListSkin = 0;
    }

    if (!shutdown) {
        m_textureCategoryContainerListSkin = new MultiListClass<DX8FVFCategoryContainer>;
    }

    m_textureCategoryContainerListsRigid.Delete_All();
}

void DX8MeshRendererClass::Clear_Pending_Delete_Lists()
{
    for (MultiListObjectClass *i = g_textureCategoryDeleteList.Remove_Head(); i != nullptr;
         i = g_textureCategoryDeleteList.Remove_Head()) {
        delete i;
    }

    for (MultiListObjectClass *i = g_fvfCategoryContainerDeleteList.Remove_Head(); i != nullptr;
         i = g_fvfCategoryContainerDeleteList.Remove_Head()) {
        delete i;
    }
}

void DX8MeshRendererClass::Request_Log_Statistics()
{
    // does nothing
}

DX8MeshRendererClass::DX8MeshRendererClass() :
    m_enableLighting(true), m_camera(nullptr), m_textureCategoryContainerListSkin(nullptr), m_visibleDecalMeshes(nullptr)
{
}

DX8MeshRendererClass::~DX8MeshRendererClass()
{
    Invalidate(true);
    Clear_Pending_Delete_Lists();

    if (m_textureCategoryContainerListSkin != nullptr) {
        delete m_textureCategoryContainerListSkin;
    }
}

void Render_FVF_Category_Container_List(MultiListClass<DX8FVFCategoryContainer> &containers)
{
    for (MultiListIterator<DX8FVFCategoryContainer> container(&containers); container; ++container) {
        container->Render();
    }
}

void Render_FVF_Category_Container_List_Delayed_Passes(MultiListClass<DX8FVFCategoryContainer> &containers)
{
    for (MultiListIterator<DX8FVFCategoryContainer> container(&containers); container; ++container) {
        container->Render_Delayed_Procedural_Material_Passes();
    }
}

void DX8MeshRendererClass::Flush()
{
    if (m_camera != nullptr) {
        Log_Statistics_String(true);

        for (int i = 0; i < m_textureCategoryContainerListsRigid.Count(); i++) {
            Render_FVF_Category_Container_List(*m_textureCategoryContainerListsRigid[i]);
        }

        Render_FVF_Category_Container_List(*m_textureCategoryContainerListSkin);
        Render_Decal_Meshes();

        for (int i = 0; i < m_textureCategoryContainerListsRigid.Count(); i++) {
            Render_FVF_Category_Container_List_Delayed_Passes(*m_textureCategoryContainerListsRigid[i]);
        }

        DX8Wrapper::Set_Vertex_Buffer(nullptr, 0);
        DX8Wrapper::Set_Index_Buffer(nullptr, 0);
    }
}

void DX8MeshRendererClass::Log_Statistics_String(bool only_visible)
{
    // does nothing
}

void Add_Rigid_Mesh_To_Container(
    MultiListClass<DX8FVFCategoryContainer> *container_list, unsigned int fvf, MeshModelClass *mmc)
{
    captainslog_assert(container_list != nullptr);
    bool sorting = W3D::Is_Sorting_Enabled() && mmc->Get_Flag(MeshGeometryClass::SORT) && mmc->Get_Sort_Level() == 0;

    for (MultiListIterator<DX8FVFCategoryContainer> container(container_list); container; ++container) {
        if (container->Is_Sorting() == sorting && container->Check_If_Mesh_Fits(mmc)) {
            container->Add_Mesh(mmc);
            return;
        }
    }

    DX8RigidFVFCategoryContainer *container = new DX8RigidFVFCategoryContainer(fvf, sorting);
    container_list->Add_Tail(container);
    container->Add_Mesh(mmc);
}

void DX8MeshRendererClass::Register_Mesh_Type(MeshModelClass *mmc)
{
    bool skin = mmc->Get_Flag(MeshGeometryClass::SKIN) && mmc->Get_Vertex_Bone_Links();
    bool sorting = mmc->Get_Flag(MeshGeometryClass::SORT) && W3D::Is_Sorting_Enabled() && mmc->Get_Sort_Level() == 0;

    if (skin) {
        captainslog_assert(m_textureCategoryContainerListSkin != nullptr);
        MultiListIterator<DX8FVFCategoryContainer> container(m_textureCategoryContainerListSkin);

        while (!container.Is_Done()) {
            if (container->Is_Sorting() == sorting && container->Check_If_Mesh_Fits(mmc)) {
                container->Add_Mesh(mmc);
                return;
            }

            container.Next();
        }

        DX8SkinFVFCategoryContainer *skin_fvf = new DX8SkinFVFCategoryContainer(sorting);
        m_textureCategoryContainerListSkin->Add_Tail(skin_fvf);
        skin_fvf->Add_Mesh(mmc);
    } else {
        captainslog_assert(g_registeredMeshList.Contains(mmc) == false);
        if (!g_registeredMeshList.Contains(mmc)) {

            unsigned int fvf = DX8FVFCategoryContainer::Define_FVF(mmc, m_enableLighting);

            for (int i = 0; i < m_textureCategoryContainerListsRigid.Count(); i++) {
                MultiListClass<DX8FVFCategoryContainer> *list = m_textureCategoryContainerListsRigid[i];
                captainslog_assert(list != nullptr);
                DX8FVFCategoryContainer *container = list->Get_Head();

                if (container == nullptr || container->Get_FVF() == fvf) {
                    Add_Rigid_Mesh_To_Container(list, fvf, mmc);
                    if (!mmc->m_polygonRendererList.Is_Empty()) {
                        g_registeredMeshList.Add(mmc);
                    }
                    return;
                }
            }

            MultiListClass<DX8FVFCategoryContainer> *containerList = new MultiListClass<DX8FVFCategoryContainer>;
            m_textureCategoryContainerListsRigid.Add(containerList);
            Add_Rigid_Mesh_To_Container(containerList, fvf, mmc);

            if (!mmc->m_polygonRendererList.Is_Empty()) {
                g_registeredMeshList.Add(mmc);
            }
        }
    }
}

void DX8MeshRendererClass::Unregister_Mesh_Type(MeshModelClass *mmc)
{
    for (DX8PolygonRendererClass *i = mmc->m_polygonRendererList.Remove_Head(); i != nullptr;
         i = mmc->m_polygonRendererList.Remove_Head()) {
        delete i;
    }

    g_registeredMeshList.Remove(mmc);
}

void DX8MeshRendererClass::Add_To_Render_List(DecalMeshClass *decalmesh)
{
    captainslog_assert(decalmesh != nullptr);
    decalmesh->Set_Next_Visible(m_visibleDecalMeshes);
    m_visibleDecalMeshes = decalmesh;
}

void DX8MeshRendererClass::Render_Decal_Meshes()
{
#ifdef BUILD_WITH_D3D8
    DecalMeshClass *d = m_visibleDecalMeshes;
    if (d != nullptr) {
        DX8Wrapper::Set_DX8_Render_State(D3DRS_ZBIAS, 8);

        do {
            d->Render();
            d = d->Peek_Next_Visible();
        } while (d != nullptr);

        m_visibleDecalMeshes = 0;
        DX8Wrapper::Set_DX8_Render_State(D3DRS_ZBIAS, 0);
    }
#endif
}

DX8TextureCategoryClass::DX8TextureCategoryClass(
    DX8FVFCategoryContainer *container, TextureClass **texs, ShaderClass shd, VertexMaterialClass *mat, int pass) :
    m_pass(pass), m_shader(shd), m_material(mat), m_container(container), m_renderTaskHead(0)
{
    captainslog_assert(pass >= 0);
    captainslog_assert(pass < DX8FVFCategoryContainer::MAX_PASSES);

    for (int i = 0; i < ARRAY_SIZE(m_textures); i++) {
        m_textures[i] = nullptr;
        Ref_Ptr_Set(m_textures[i], texs[i]);
    }

    if (m_material != nullptr) {
        m_material->Add_Ref();
    }
}

DX8TextureCategoryClass::~DX8TextureCategoryClass()
{
    for (;;) {
        DX8PolygonRendererClass *r = m_polygonRendererList.Get_Head();

        if (r == nullptr) {
            break;
        }

        g_theDX8MeshRenderer.Unregister_Mesh_Type(r->Get_Mesh_Model_Class());
    }

    for (int i = 0; i < ARRAY_SIZE(m_textures); i++) {
        Ref_Ptr_Release(m_textures[i]);
    }

    Ref_Ptr_Release(m_material);
}

void DX8TextureCategoryClass::Add_Render_Task(DX8PolygonRendererClass *p_renderer, MeshClass *p_mesh)
{
    PolyRenderTaskClass *task = new PolyRenderTaskClass(p_renderer, p_mesh);
    task->Set_Next_Visible(m_renderTaskHead);
    m_renderTaskHead = task;
    m_container->Add_Visible_Texture_Category(this, m_pass);
}

void DX8TextureCategoryClass::Render()
{
    for (int i = 0; i < ARRAY_SIZE(m_textures); i++) {
        DX8Wrapper::Set_Texture(i, Peek_Texture(i));
    }

    DX8Wrapper::Set_Material(Peek_Material());
    ShaderClass shader = Get_Shader();
    ShaderClass shader2 = shader;
    shader2.Set_Src_Blend_Func(ShaderClass::SRCBLEND_SRC_ALPHA);
    shader2.Set_Dst_Blend_Func(ShaderClass::DSTBLEND_ONE_MINUS_SRC_ALPHA);
    DX8Wrapper::Set_Shader(shader);

    if (s_forceMultiply) {
        if (shader.Get_Dst_Blend_Func() == ShaderClass::DSTBLEND_ZERO) {
            shader.Set_Dst_Blend_Func(ShaderClass::DSTBLEND_SRC_COLOR);
            shader.Set_Src_Blend_Func(ShaderClass::SRCBLEND_ZERO);
            DX8Wrapper::Set_Shader(shader);
            DX8Wrapper::Apply_Render_State_Changes();
#ifdef BUILD_WITH_D3D8
            DX8Wrapper::Set_DX8_Render_State(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
#endif
        }
    }

    bool b = false;
    PolyRenderTaskClass *task = m_renderTaskHead;
    PolyRenderTaskClass *task2 = nullptr;
    while (task != nullptr) {
        DX8PolygonRendererClass *renderer = task->Peek_Polygon_Renderer();
        MeshClass *mesh = task->Peek_Mesh();
        captainslog_assert(mesh != nullptr);

        if (mesh->Get_Base_Vertex_Offset() == 0xFFFF) {
            task2 = task;
            task = task->Get_Next_Visible();
            b = true;
        } else {
            if (mesh->Get_Lighting_Environment() != nullptr) {
                DX8Wrapper::Set_Light_Environment(mesh->Get_Lighting_Environment());
            }

            Matrix3D tm = mesh->Get_Transform();
            bool identity = mesh->Is_Transform_Identity();
            Matrix3D tm2;
            MeshModelClass *mmc = mesh->Peek_Model();

            if (mmc->Get_Flag(MeshGeometryClass::ALIGNED)) {
                CameraClass *camera = g_theDX8MeshRenderer.Peek_Camera();
                Matrix3D tm3 = camera->Get_Transform();
                Vector3 v1 = tm3.Get_Z_Vector();
                Matrix3D tm4 = mesh->Get_Transform();
                Vector3 v2 = tm4.Get_Translation();
                tm2.Obj_Look_At(v1, v1 + v2, 0);
                tm = tm2;
            } else if (mmc->Get_Flag(MeshGeometryClass::ORIENTED)) {
                Vector3 v1;
                Vector3 v2;
                CameraClass *camera = g_theDX8MeshRenderer.Peek_Camera();
                Matrix3D tm3 = camera->Get_Transform();
                tm3.Get_Translation(&v2);
                Matrix3D tm4 = mesh->Get_Transform();
                tm4.Get_Translation(&v1);
                tm2.Obj_Look_At(v1, v2, 0);
                tm = tm2;
            } else if (mmc->Get_Flag(MeshGeometryClass::SKIN)) {
                tm = Matrix3D::Identity;
                identity = true;
            }

            if (identity) {
#ifdef BUILD_WITH_D3D8
                DX8Wrapper::Set_World_Identity();
#endif
            } else {
#ifdef BUILD_WITH_D3D8
                DX8Wrapper::Set_Transform(D3DTS_WORLD, tm);
#endif
            }

            if (mesh->Get_ObjectScale() != 1.0f) {
#ifdef BUILD_WITH_D3D8
                DX8Wrapper::Set_DX8_Render_State(D3DRS_NORMALIZENORMALS, 1);
#endif
            }

            RenderObjClass::Material_Override *mat_override =
                static_cast<RenderObjClass::Material_Override *>(mesh->Get_User_Data());

            if (mmc->Get_Flag(MeshGeometryClass::SORT) && W3D::Is_Sorting_Enabled()) {
                renderer->Render_Sorted(mesh->Get_Base_Vertex_Offset(), mesh->Get_Bounding_Sphere());
            } else if (mesh->Get_Alpha_Override() != 1.0f
                || (mat_override != nullptr && mat_override->m_structID == RenderObjClass::USER_DATA_MATERIAL_OVERRIDE)) {
                float opacity = m_material->Get_Opacity();
                Vector3 diffuse;
                m_material->Get_Diffuse(&diffuse);
                TextureMapperClass *mapper = m_material->Peek_Mapper(0);
                unsigned int sync;
                Vector2 uv;

                if (mat_override != nullptr && mat_override->m_structID == RenderObjClass::USER_DATA_MATERIAL_OVERRIDE
                    && mapper != nullptr && mapper->Mapper_ID() == TextureMapperClass::MAPPER_ID_LINEAR_OFFSET) {
                    LinearOffsetTextureMapperClass *l = static_cast<LinearOffsetTextureMapperClass *>(mapper);
                    sync = l->Get_Sync_Time();
                    l->Set_Sync_Time(W3D::Get_Sync_Time());
                    uv = l->Get_Current_UV_Offset();
                    l->Set_Current_UV_Offset(mat_override->m_customUVOffset);
                } else {
                    mapper = nullptr;
                }

                if (mesh->Get_Alpha_Override() == 1.0f) {
                    renderer->Render(mesh->Get_Base_Vertex_Offset());
                } else {
                    if (mesh->Is_Additive()) {
                        m_material->Set_Diffuse(
                            mesh->Get_Alpha_Override(), mesh->Get_Alpha_Override(), mesh->Get_Alpha_Override());
                        shader2 = shader;
                    }

                    m_material->Set_Opacity(mesh->Get_Alpha_Override());
                    DX8Wrapper::Set_Shader(shader2);
                    DX8Wrapper::Apply_Render_State_Changes();
#ifdef BUILD_WITH_D3D8
                    DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHAREF, mesh->Get_Alpha_Override() * 96);
#endif
                    renderer->Render(mesh->Get_Base_Vertex_Offset());
#ifdef BUILD_WITH_D3D8
                    DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHAREF, 96);
#endif
                    m_material->Set_Opacity(opacity);
                    m_material->Set_Diffuse(diffuse.X, diffuse.Y, diffuse.Z);
                    DX8Wrapper::Set_Shader(shader);
                }

                if (mapper != nullptr) {
                    LinearOffsetTextureMapperClass *l = static_cast<LinearOffsetTextureMapperClass *>(mapper);
                    l->Set_Sync_Time(sync);
                    l->Set_Current_UV_Offset(uv);
                }

                DX8Wrapper::Set_Material(nullptr);
                DX8Wrapper::Set_Material(m_material);
            } else {
                renderer->Render(mesh->Get_Base_Vertex_Offset());
            }

            if (mesh->Get_ObjectScale() != 1.0f) {
#ifdef BUILD_WITH_D3D8
                DX8Wrapper::Set_DX8_Render_State(D3DRS_NORMALIZENORMALS, 0);
#endif
            }

            PolyRenderTaskClass *task3 = task->Get_Next_Visible();

            if (task2 != nullptr) {
                task2->Set_Next_Visible(task3);
            } else {
                m_renderTaskHead = task3;
            }

            if (task != nullptr) {
                delete task;
            }

            task = task3;
        }
    }

    if (!b) {
        captainslog_assert(m_renderTaskHead == nullptr);
        Clear_Render_List();
    }
}

unsigned int DX8TextureCategoryClass::Add_Mesh(Vertex_Split_Table &split_table,
    unsigned int vertex_offset,
    unsigned int index_offset,
    IndexBufferClass *index_buffer,
    unsigned int pass)
{
    int polycount = split_table.Get_Polygon_Count();
    unsigned int index_count = 0;
    int polygons = 0;

    for (int i = 0; i < polycount; i++) {
        if (split_table.Peek_Texture(i, pass, 0) == m_textures[0]) {
            if (split_table.Peek_Texture(i, pass, 1) == m_textures[1]) {
                if (Compare_Materials(split_table.Peek_Material(i, pass), m_material)) {
                    if (split_table.Peek_Shader(i, pass) == m_shader) {
                        polygons++;
                    }
                }
            }
        }
    }

    if (polygons != 0) {
        index_count = 3 * polygons;
        TriIndex *polys = split_table.Get_Polygon_Array(pass);
        DX8PolygonRendererClass *renderer = new DX8PolygonRendererClass(
            index_count, split_table.Get_Mesh_Model_Class(), this, vertex_offset, index_offset, 0, pass);
        m_polygonRendererList.Add_Tail(renderer);
        IndexBufferClass::AppendLockClass lock(index_buffer, index_offset, index_count);
        unsigned short *indices = lock.Get_Index_Array();
        unsigned short vmax = 0;
        unsigned short vmin = 0xFFFF;

        for (int i = 0; i < polycount; i++) {
            if (split_table.Peek_Texture(i, pass, 0) == m_textures[0]) {
                if (split_table.Peek_Texture(i, pass, 1) == m_textures[1]) {
                    if (Compare_Materials(split_table.Peek_Material(i, pass), m_material)) {
                        if (split_table.Peek_Shader(i, pass) == m_shader) {
                            unsigned short index = vertex_offset + polys[i].I;

                            vmin = std::min(index, vmin);
                            vmax = std::max(index, vmax);

                            *indices = index;
                            indices++;

                            index = vertex_offset + polys[i].J;

                            vmin = std::min(index, vmin);
                            vmax = std::max(index, vmax);

                            *indices = index;
                            indices++;

                            index = vertex_offset + polys[i].K;

                            vmin = std::min(index, vmin);
                            vmax = std::max(index, vmax);

                            *indices = index;
                            indices++;
                        }
                    }
                }
            }
        }

        captainslog_assert((vmax - vmin) < split_table.Get_Mesh_Model_Class()->Get_Vertex_Count());
        renderer->Set_Vertex_Index_Range(vmin, vmax - vmin + 1);
        captainslog_assert(index_count <= unsigned(split_table.Get_Polygon_Count() * 3));
    }

    return index_count;
}

void DX8TextureCategoryClass::Log(bool only_visible)
{
    // does nothing
}

void DX8TextureCategoryClass::Remove_Polygon_Renderer(DX8PolygonRendererClass *p_renderer)
{
    m_polygonRendererList.Remove(p_renderer);
    p_renderer->Set_Texture_Category(nullptr);

    if (m_polygonRendererList.Get_Head() == nullptr) {
        m_container->Remove_Texture_Category(this);
        g_textureCategoryDeleteList.Add_Tail(this);
    }
}

void DX8TextureCategoryClass::Add_Polygon_Renderer(
    DX8PolygonRendererClass *p_renderer, DX8PolygonRendererClass *add_after_this)
{
    captainslog_assert(p_renderer != nullptr);
    captainslog_assert(!m_polygonRendererList.Contains(p_renderer));
    if (add_after_this != nullptr) {
        bool res = m_polygonRendererList.Add_After(p_renderer, add_after_this, false);
        captainslog_assert(res);
    } else {
        m_polygonRendererList.Add(p_renderer);
    }

    p_renderer->Set_Texture_Category(this);
}

DX8FVFCategoryContainer::DX8FVFCategoryContainer(unsigned int FVF, bool sorting) :
    m_visibleMatpassHead(nullptr),
    m_visibleMatpassTail(nullptr),
    m_indexBuffer(nullptr),
    m_usedIndices(0),
    m_FVF(FVF),
    m_passes(MAX_PASSES),
    m_uvCoordinateChannels(0),
    m_sorting(sorting),
    m_anythingToRender(false),
    m_anyDelayedPassesToRender(false)
{
#ifdef BUILD_WITH_D3D8
    if ((FVF & D3DFVF_TEX1) == D3DFVF_TEX1) {
        m_uvCoordinateChannels = 1;
    }
    if ((FVF & D3DFVF_TEX2) == D3DFVF_TEX2) {
        m_uvCoordinateChannels = 2;
    }
    if ((FVF & D3DFVF_TEX3) == D3DFVF_TEX3) {
        m_uvCoordinateChannels = 3;
    }
    if ((FVF & D3DFVF_TEX4) == D3DFVF_TEX4) {
        m_uvCoordinateChannels = 4;
    }
    if ((FVF & D3DFVF_TEX5) == D3DFVF_TEX5) {
        m_uvCoordinateChannels = 5;
    }
    if ((FVF & D3DFVF_TEX6) == D3DFVF_TEX6) {
        m_uvCoordinateChannels = 6;
    }
    if ((FVF & D3DFVF_TEX7) == D3DFVF_TEX7) {
        m_uvCoordinateChannels = 7;
    }
    if ((FVF & D3DFVF_TEX8) == D3DFVF_TEX8) {
        m_uvCoordinateChannels = 8;
    }
#endif
}

DX8FVFCategoryContainer::~DX8FVFCategoryContainer()
{
    Ref_Ptr_Release(m_indexBuffer);
    for (unsigned int i = 0; i < m_passes; i++) {
        for (DX8TextureCategoryClass *j = m_textureCategoryList->Remove_Head(); j != nullptr;
             j = m_textureCategoryList->Remove_Head()) {
            delete j;
        }
    }
}

unsigned int DX8FVFCategoryContainer::Define_FVF(MeshModelClass *mmc, bool enable_lighting)
{
#ifdef BUILD_WITH_D3D8
    if (mmc->Get_Flag(MeshGeometryClass::SORT) && W3D::Is_Sorting_Enabled()) {
        return DX8_FVF_XYZNDUV2;
    }

    int fvf = D3DFVF_XYZ;

    if (mmc->Get_Color_Array(0, false) != nullptr) {
        fvf |= D3DFVF_DIFFUSE;
    }

    if (mmc->Get_Color_Array(1, false) != nullptr) {
        fvf |= D3DFVF_SPECULAR;
    }

    switch (mmc->Get_UV_Array_Count()) {
        case 1:
            fvf |= D3DFVF_TEX1;
            break;
        case 2:
            fvf |= D3DFVF_TEX2;
            break;
        case 3:
            fvf |= D3DFVF_TEX3;
            break;
        case 4:
            fvf |= D3DFVF_TEX4;
            break;
        case 5:
            fvf |= D3DFVF_TEX5;
            break;
        case 6:
            fvf |= D3DFVF_TEX6;
            break;
        case 7:
            fvf |= D3DFVF_TEX7;
            break;
        case 8:
            fvf |= D3DFVF_TEX8;
            break;
    }

    if (mmc->Needs_Vertex_Normals()) {
        fvf |= D3DFVF_NORMAL;
    }

    return fvf;
#else
    return 0;
#endif
}

class PolyRemover : public MultiListObjectClass
{
public:
    DX8TextureCategoryClass *src;
    DX8TextureCategoryClass *dest;
    DX8PolygonRendererClass *pr;
};

void DX8FVFCategoryContainer::Change_Polygon_Renderer_Texture(MultiListClass<DX8PolygonRendererClass> &polygon_renderer_list,
    TextureClass *texture,
    TextureClass *new_texture,
    unsigned int pass,
    unsigned int stage)
{
    captainslog_assert(pass < m_passes);
    MultiListClass<PolyRemover> prl;
    bool foundtexture = false;

    if (texture == new_texture) {
        return;
    } else {
        MultiListIterator<DX8TextureCategoryClass> src_it(&m_textureCategoryList[pass]);

        while (!src_it.Is_Done()) {
            DX8TextureCategoryClass *texcat = src_it.Peek_Obj();

            if (texcat->Peek_Texture(stage) == texture) {
                foundtexture = true;
                MultiListIterator<DX8PolygonRendererClass> poly_it(&polygon_renderer_list);

                while (!poly_it.Is_Done()) {
                    DX8PolygonRendererClass *pr = poly_it.Peek_Obj();

                    if (pr->Get_Texture_Category() == texcat) {
                        DX8TextureCategoryClass *tc = Find_Matching_Texture_Category(new_texture, pass, stage, texcat);

                        if (tc == nullptr) {
                            TextureClass *tmp_textures[DX8TextureCategoryClass::TextureSize];
                            tmp_textures[0] = texcat->Peek_Texture(0);
                            tmp_textures[1] = texcat->Peek_Texture(1);
                            tmp_textures[stage] = new_texture;
                            tc = new DX8TextureCategoryClass(
                                this, tmp_textures, texcat->Get_Shader(), texcat->Peek_Material(), pass);
                            bool b = false;
                            MultiListIterator<DX8TextureCategoryClass> tc_it(&m_textureCategoryList[pass]);

                            while (!tc_it.Is_Done()) {
                                if (tmp_textures[0] == tc_it.Peek_Obj()->Peek_Texture(0)) {
                                    b = true;
                                    m_textureCategoryList[pass].Add_After(tc, tc_it.Peek_Obj());
                                    break;
                                }

                                tc_it.Next();
                            }

                            if (!b) {
                                m_textureCategoryList[pass].Add_Tail(tc);
                            }
                        }

                        PolyRemover *remover = new PolyRemover();
                        remover->src = texcat;
                        remover->dest = tc;
                        remover->pr = pr;
                        prl.Add(remover);
                    }

                    poly_it.Next();
                }
            } else if (foundtexture) {
                break;
            }

            src_it.Next();
        }

        MultiListIterator<PolyRemover> prl_it(&prl);

        while (!prl_it.Is_Done()) {
            PolyRemover *remover = prl_it.Peek_Obj();
            remover->src->Remove_Polygon_Renderer(remover->pr);
            remover->dest->Add_Polygon_Renderer(remover->pr, nullptr);
            prl_it.Remove_Current_Object();
            delete remover;
        }
    }
}

void DX8FVFCategoryContainer::Change_Polygon_Renderer_Material(
    MultiListClass<DX8PolygonRendererClass> &polygon_renderer_list,
    VertexMaterialClass *vmat,
    VertexMaterialClass *new_vmat,
    unsigned int pass)
{
    captainslog_assert(pass < m_passes);
    MultiListClass<PolyRemover> prl;
    bool foundmat = false;

    if (vmat == new_vmat) {
        return;
    } else {
        MultiListIterator<DX8TextureCategoryClass> src_it(&m_textureCategoryList[pass]);

        while (!src_it.Is_Done()) {
            DX8TextureCategoryClass *texcat = src_it.Peek_Obj();

            if (texcat->Peek_Material() == vmat) {
                foundmat = true;
                MultiListIterator<DX8PolygonRendererClass> poly_it(&polygon_renderer_list);

                while (!poly_it.Is_Done()) {
                    DX8PolygonRendererClass *pr = poly_it.Peek_Obj();

                    if (pr->Get_Texture_Category() == texcat) {
                        DX8TextureCategoryClass *tc = Find_Matching_Texture_Category(new_vmat, pass, texcat);

                        if (tc == nullptr) {
                            TextureClass *tmp_textures[DX8TextureCategoryClass::TextureSize];
                            tmp_textures[0] = texcat->Peek_Texture(0);
                            tmp_textures[1] = texcat->Peek_Texture(1);
                            tc = new DX8TextureCategoryClass(this, tmp_textures, texcat->Get_Shader(), new_vmat, pass);
                            bool b = false;
                            MultiListIterator<DX8TextureCategoryClass> tc_it(&m_textureCategoryList[pass]);

                            while (!tc_it.Is_Done()) {
                                if (tmp_textures[0] == tc_it.Peek_Obj()->Peek_Texture(0)) {
                                    b = true;
                                    m_textureCategoryList[pass].Add_After(tc, tc_it.Peek_Obj());
                                    break;
                                }

                                tc_it.Next();
                            }

                            if (!b) {
                                m_textureCategoryList[pass].Add_Tail(tc);
                            }
                        }

                        PolyRemover *remover = new PolyRemover();
                        remover->src = texcat;
                        remover->dest = tc;
                        remover->pr = pr;
                        prl.Add(remover);
                    }

                    poly_it.Next();
                }
            } else if (foundmat) {
                break;
            }

            src_it.Next();
        }

        MultiListIterator<PolyRemover> prl_it(&prl);

        while (!prl_it.Is_Done()) {
            PolyRemover *remover = prl_it.Peek_Obj();
            remover->src->Remove_Polygon_Renderer(remover->pr);
            remover->dest->Add_Polygon_Renderer(remover->pr, nullptr);
            prl_it.Remove_Current_Object();
            delete remover;
        }
    }
}

void DX8FVFCategoryContainer::Remove_Texture_Category(DX8TextureCategoryClass *tex_category)
{
    for (unsigned int i = 0; i < m_passes; i++) {
        m_textureCategoryList[i].Remove(tex_category);
    }

    for (unsigned int i = 0; i < m_passes; i++) {
        if (m_textureCategoryList[i].Peek_Head() != nullptr) {
            return;
        }
    }

    g_fvfCategoryContainerDeleteList.Add_Tail(this);
}

void DX8FVFCategoryContainer::Add_Visible_Material_Pass(MaterialPassClass *pass, MeshClass *mesh)
{
    MatPassTaskClass *task = new MatPassTaskClass(pass, mesh);

    if (m_visibleMatpassHead != nullptr) {
        captainslog_assert(m_visibleMatpassTail != nullptr);
        m_visibleMatpassTail->Set_Next_Visible(task);
    } else {
        captainslog_assert(m_visibleMatpassTail == nullptr);
        m_visibleMatpassHead = task;
    }

    m_visibleMatpassTail = task;
    m_anythingToRender = true;
}

void DX8FVFCategoryContainer::Render_Procedural_Material_Passes()
{
    MatPassTaskClass *task = m_visibleMatpassHead;
    MatPassTaskClass *task2 = nullptr;
    bool b = false;

    while (task != nullptr) {
        if (task->Peek_Mesh()->Get_Base_Vertex_Offset() == 0xFFFF) {
            task2 = task;
            task = task->Get_Next_Visible();
            b = true;
        } else {
            task->Peek_Mesh()->Render_Material_Pass(task->Peek_Material_Pass(), m_indexBuffer);
            MatPassTaskClass *task3 = task->Get_Next_Visible();

            if (task2 != nullptr) {
                task2->Set_Next_Visible(task3);
            } else {
                m_visibleMatpassHead = task3;
            }

            delete task;
            task = task3;
        }
    }

    m_visibleMatpassTail = b ? task2 : nullptr;
}

DX8TextureCategoryClass *DX8FVFCategoryContainer::Find_Matching_Texture_Category(
    VertexMaterialClass *vmat, unsigned int pass, DX8TextureCategoryClass *ref_category)
{
    MultiListIterator<DX8TextureCategoryClass> dest_it(&m_textureCategoryList[pass]);

    while (!dest_it.Is_Done()) {
        if (Compare_Materials(dest_it.Peek_Obj()->Peek_Material(), vmat)) {
            DX8TextureCategoryClass *tc = dest_it.Peek_Obj();
            bool b = true;

            for (int i = 0; i < DX8TextureCategoryClass::TextureSize; i++) {
                b = b && (tc->Peek_Texture(i) == ref_category->Peek_Texture(i));
            }

            if (b) {
                if (tc->Get_Shader() == ref_category->Get_Shader()) {
                    return tc;
                }
            }
        }

        dest_it.Next();
    }

    return nullptr;
}

DX8TextureCategoryClass *DX8FVFCategoryContainer::Find_Matching_Texture_Category(
    TextureClass *texture, unsigned int pass, unsigned int stage, DX8TextureCategoryClass *ref_category)
{
    MultiListIterator<DX8TextureCategoryClass> dest_it(&m_textureCategoryList[pass]);

    while (!dest_it.Is_Done()) {
        if (dest_it.Peek_Obj()->Peek_Texture(stage) == texture) {
            DX8TextureCategoryClass *tc = dest_it.Peek_Obj();
            bool b = true;

            for (int i = 0; i < DX8TextureCategoryClass::TextureSize; i++) {
                if (stage != i) {
                    b = b && (tc->Peek_Texture(i) == ref_category->Peek_Texture(i));
                }
            }

            if (b) {
                if (Compare_Materials(tc->Peek_Material(), ref_category->Peek_Material())) {
                    if (tc->Get_Shader() == ref_category->Get_Shader()) {
                        return tc;
                    }
                }
            }
        }

        dest_it.Next();
    }

    return nullptr;
}

void DX8FVFCategoryContainer::Insert_To_Texture_Category(Vertex_Split_Table &split_table,
    TextureClass **texs,
    VertexMaterialClass *mat,
    ShaderClass shader,
    int pass,
    unsigned int vertex_offset)
{
    bool b = false;
    MultiListIterator<DX8TextureCategoryClass> dst_it(&m_textureCategoryList[pass]);

    while (!dst_it.Is_Done()) {
        DX8TextureCategoryClass *tc = dst_it.Peek_Obj();

        if (tc->Peek_Texture(0) == texs[0]) {
            if (tc->Peek_Texture(1) == texs[1]) {
                if (Compare_Materials(tc->Peek_Material(), mat)) {
                    if (tc->Get_Shader() == shader) {
                        m_usedIndices += tc->Add_Mesh(split_table, vertex_offset, m_usedIndices, m_indexBuffer, pass);
                        b = true;
                        break;
                    }
                }
            }
        }

        dst_it.Next();
    }

    if (!b) {
        DX8TextureCategoryClass *tc = new DX8TextureCategoryClass(this, texs, shader, mat, pass);
        m_usedIndices += tc->Add_Mesh(split_table, vertex_offset, m_usedIndices, m_indexBuffer, pass);

        bool b2 = false;
        MultiListIterator<DX8TextureCategoryClass> tc_it(&m_textureCategoryList[pass]);

        while (!tc_it.Is_Done()) {
            if (texs[0] == tc_it.Peek_Obj()->Peek_Texture(0)) {
                b2 = true;
                m_textureCategoryList[pass].Add_After(tc, tc_it.Peek_Obj());
                break;
            }

            tc_it.Next();
        }

        if (!b2) {
            m_textureCategoryList[pass].Add_Tail(tc);
        }
    }
}

void DX8FVFCategoryContainer::Generate_Texture_Categories(Vertex_Split_Table &split_table, unsigned int vertex_offset)
{
    int polygon_count = split_table.Get_Polygon_Count();
    int index_count = split_table.Get_Pass_Count() * 3 * polygon_count;

    if (m_indexBuffer == nullptr) {
        if (index_count < 12000) {
            index_count = 12000;
        }

        if (m_sorting) {
            m_indexBuffer = new SortingIndexBufferClass(index_count);
        } else {
            m_indexBuffer = new DX8IndexBufferClass(index_count, DX8IndexBufferClass::USAGE_DEFAULT);
        }
    }

    for (unsigned int i = 0; i < split_table.Get_Pass_Count(); i++) {
        Textures_Material_And_Shader_Booking_Struct booking;
        int old_used_indices = m_usedIndices;

        for (int j = 0; j < polygon_count; j++) {
            TextureClass *texs[DX8TextureCategoryClass::TextureSize];
            texs[0] = split_table.Peek_Texture(j, i, 0);
            texs[1] = split_table.Peek_Texture(j, i, 1);
            VertexMaterialClass *vmat = split_table.Peek_Material(j, i);
            ShaderClass shader = split_table.Peek_Shader(j, i);

            if (booking.Add_Textures_Material_And_Shader(texs, vmat, shader)) {
                Insert_To_Texture_Category(split_table, texs, vmat, shader, i, vertex_offset);
            }
        }

        captainslog_assert(m_usedIndices - old_used_indices <= polygon_count * 3);
    }
}

DX8RigidFVFCategoryContainer::DX8RigidFVFCategoryContainer(unsigned int FVF, bool sorting) :
    DX8FVFCategoryContainer(FVF, sorting),
    m_vertexBuffer(nullptr),
    m_usedVertices(0),
    m_delayedVisibleMatpassHead(nullptr),
    m_delayedVisibleMatpassTail(nullptr)
{
}

DX8RigidFVFCategoryContainer::~DX8RigidFVFCategoryContainer()
{
    Ref_Ptr_Release(m_vertexBuffer);
}

void DX8RigidFVFCategoryContainer::Add_Mesh(MeshModelClass *mmc)
{
    captainslog_assert(Check_If_Mesh_Fits(mmc));
    Vertex_Split_Table split(mmc);
    int count = split.Get_Vertex_Count();
    if (m_vertexBuffer == nullptr) {
        unsigned short size = 4000;

        if (count > 4000) {
            size = count;
        }

        if (m_sorting) {
            m_vertexBuffer = new SortingVertexBufferClass(size);
            captainslog_assert(m_vertexBuffer->FVF_Info().Get_FVF() == m_FVF);
        } else {
            m_vertexBuffer = new DX8VertexBufferClass(m_FVF, size, DX8VertexBufferClass::USAGE_DEFAULT, 0);
        }
    }

    VertexBufferClass::AppendLockClass lock(m_vertexBuffer, m_usedVertices, split.Get_Vertex_Count());
    FVFInfoClass f = m_vertexBuffer->FVF_Info();
    char *outverts = static_cast<char *>(lock.Get_Vertex_Array());
    const Vector3 *verts = split.Get_Vertex_Array();
    const Vector3 *normals = split.Get_Vertex_Normal_Array();
    const unsigned int *diffuse = split.Get_Color_Array(0);
    const unsigned int *specular = split.Get_Color_Array(1);

#ifdef BUILD_WITH_D3D8
    for (unsigned int i = 0; i < split.Get_Vertex_Count(); i++) {

        *reinterpret_cast<Vector3 *>(&outverts[f.Get_Location_Offset()]) = verts[i];

        if (m_FVF & D3DFVF_NORMAL) {
            if (normals != nullptr) {
                *reinterpret_cast<Vector3 *>(&outverts[f.Get_Normal_Offset()]) = normals[i];
            }
        }

        if (m_FVF & D3DFVF_DIFFUSE) {
            if (diffuse != nullptr) {
                *reinterpret_cast<unsigned int *>(&outverts[f.Get_Diffuse_Offset()]) = diffuse[i];
            } else {
                *reinterpret_cast<unsigned int *>(&outverts[f.Get_Diffuse_Offset()]) = 0xFFFFFFFF;
            }
        }

        if (m_FVF & D3DFVF_SPECULAR) {
            if (specular != nullptr) {
                *reinterpret_cast<unsigned int *>(&outverts[f.Get_Specular_Offset()]) = specular[i];
            } else {
                *reinterpret_cast<unsigned int *>(&outverts[f.Get_Specular_Offset()]) = 0xFFFFFFFF;
            }
        }

        outverts += f.Get_FVF_Size();
    }

    int texcount = (m_FVF & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;

    for (int i = 0; i < texcount; i++) {
        char *outverts2 = static_cast<char *>(lock.Get_Vertex_Array());
        const Vector2 *uv = split.Get_UV_Array(i);

        if (uv != nullptr) {
            for (unsigned int j = 0; j < split.Get_Vertex_Count(); j++) {
                *reinterpret_cast<Vector2 *>(&outverts2[f.Get_Tex_Offset(i)]) = uv[j];
                outverts2 += f.Get_FVF_Size();
            }
        }
    }
#endif

    Generate_Texture_Categories(split, m_usedVertices);
    m_usedVertices += count;
}

void DX8RigidFVFCategoryContainer::Log(bool only_visible)
{
    // does nothing
}

bool DX8RigidFVFCategoryContainer::Check_If_Mesh_Fits(MeshModelClass *mmc)
{
    if (m_vertexBuffer == nullptr) {
        return true;
    }

    int count = mmc->Get_Polygon_Count();

    return mmc->Get_Vertex_Count() <= m_vertexBuffer->Get_Vertex_Count() - m_usedVertices
        && 3 * count * mmc->Get_Pass_Count() <= m_indexBuffer->Get_Index_Count() - m_usedIndices;
}

void DX8RigidFVFCategoryContainer::Render()
{
    if (Anything_To_Render()) {
        m_anythingToRender = false;
        DX8Wrapper::Set_Vertex_Buffer(m_vertexBuffer, 0);
        DX8Wrapper::Set_Index_Buffer(m_indexBuffer, 0);

        for (unsigned int i = 0; i < m_passes; i++) {
            for (DX8TextureCategoryClass *t = m_visibleTextureCategoryList[i].Remove_Head(); t != nullptr;
                 t = m_visibleTextureCategoryList[i].Remove_Head()) {
                t->Render();
            }
        }

        Render_Procedural_Material_Passes();
    }
}

void DX8RigidFVFCategoryContainer::Add_Delayed_Visible_Material_Pass(MaterialPassClass *pass, MeshClass *mesh)
{
    MatPassTaskClass *task = new MatPassTaskClass(pass, mesh);

    if (m_delayedVisibleMatpassHead != nullptr) {
        captainslog_assert(m_delayedVisibleMatpassTail != nullptr);
        m_delayedVisibleMatpassTail->Set_Next_Visible(task);
    } else {
        captainslog_assert(m_delayedVisibleMatpassTail == nullptr);
        m_delayedVisibleMatpassHead = task;
    }

    m_delayedVisibleMatpassTail = task;
    m_anyDelayedPassesToRender = true;
}

void DX8RigidFVFCategoryContainer::Render_Delayed_Procedural_Material_Passes()
{
    if (Any_Delayed_Passes_To_Render()) {
        m_anyDelayedPassesToRender = false;
        DX8Wrapper::Set_Vertex_Buffer(m_vertexBuffer, 0);
        DX8Wrapper::Set_Index_Buffer(m_indexBuffer, 0);

        MatPassTaskClass *m;

        for (MatPassTaskClass *i = m_delayedVisibleMatpassHead; i != nullptr; i = m) {
            i->Peek_Mesh()->Render_Material_Pass(i->Peek_Material_Pass(), m_indexBuffer);
            m = i->Get_Next_Visible();
            delete i;
        }

        m_delayedVisibleMatpassTail = nullptr;
        m_delayedVisibleMatpassHead = nullptr;
    }
}

DX8SkinFVFCategoryContainer::DX8SkinFVFCategoryContainer(bool sorting) :
    DX8FVFCategoryContainer(DX8_FVF_XYZNUV1, sorting),
    m_visibleVertexCount(0),
    m_visibleSkinHead(nullptr),
    m_visibleSkinTail(nullptr)
{
}

void DX8SkinFVFCategoryContainer::Add_Visible_Skin(MeshClass *mesh)
{
    if (mesh->Peek_Next_Visible_Skin() == nullptr && mesh != m_visibleSkinTail) {

        if (m_visibleSkinHead == nullptr) {
            m_visibleSkinTail = mesh;
        }

        mesh->Set_Next_Visible_Skin(m_visibleSkinHead);
        m_visibleSkinHead = mesh;
        m_visibleVertexCount += mesh->Peek_Model()->Get_Vertex_Count();
    }
}

DX8SkinFVFCategoryContainer::~DX8SkinFVFCategoryContainer()
{
    // does nothing
}

void DX8SkinFVFCategoryContainer::Add_Mesh(MeshModelClass *mmc)
{
    Vertex_Split_Table split(mmc);
    Generate_Texture_Categories(split, 0);
}

void DX8SkinFVFCategoryContainer::Log(bool only_visible)
{
    // does nothing
}

bool DX8SkinFVFCategoryContainer::Check_If_Mesh_Fits(MeshModelClass *mmc)
{
    if (m_indexBuffer == nullptr) {
        return true;
    }

    int count = mmc->Get_Polygon_Count();

    return 3 * count * mmc->Get_Pass_Count() <= m_indexBuffer->Get_Index_Count() - m_usedIndices;
}

void DX8SkinFVFCategoryContainer::Render()
{
    if (Anything_To_Render()) {
        m_anythingToRender = false;
        DX8Wrapper::Set_Vertex_Buffer(nullptr, 0);
        unsigned int vertcount = m_visibleVertexCount;

        if (vertcount > 65535) {
            vertcount = 65535;
        }

        DynamicVBAccessClass vb(m_sorting + 2, DX8_FVF_XYZNDUV2, vertcount);
        MeshClass *mesh1 = m_visibleSkinHead;
        unsigned int renderedvertexcount = 0;

        if (m_visibleVertexCount != 0) {
            for (;;) {
                MeshClass *mesh2 = mesh1;
                unsigned int vertex_offset = 0;
                MeshClass *mesh4 = nullptr;
                MeshClass *mesh3 = nullptr;

                { // added to control the lifetime of the DynamicVBAccessClass::WriteLockClass object
                    DynamicVBAccessClass::WriteLockClass lock(&vb);
                    VertexFormatXYZNDUV2 *vertexes = lock.Get_Formatted_Vertex_Array();

                    if (mesh1 != nullptr) {
                        for (;;) {
                            MeshModelClass *mmc = mesh1->Peek_Model();
                            int mesh_vertex_count = mmc->Get_Vertex_Count();
                            unsigned int offset = mesh_vertex_count + vertex_offset;

                            if (mesh_vertex_count + vertex_offset > vertcount || mesh3) {
                                mesh1->Set_Base_Vertex_Offset(0xFFFF);

                                if (mesh3 == nullptr) {
                                    mesh4 = mesh1;
                                }

                                mesh2 = mesh1->Peek_Next_Visible_Skin();
                            } else {
                                captainslog_assert((vertex_offset + mesh_vertex_count) <= m_visibleVertexCount);
                                // Debug_Statistics::Record_DX8_Skin_Polys_And_Vertices(mesh1->Get_Num_Polys(),
                                // mesh_vertex_count);

                                if (g_tempVertexBuffer.Length() < mesh_vertex_count) {
                                    g_tempVertexBuffer.Resize(mesh_vertex_count);
                                }

                                if (g_tempNormalBuffer.Length() < mesh_vertex_count) {
                                    g_tempNormalBuffer.Resize(mesh_vertex_count);
                                }

                                Vector3 *verts = &g_tempVertexBuffer[0];
                                Vector3 *normals = &g_tempNormalBuffer[0];
                                const Vector2 *uv1 = mmc->Get_UV_Array_By_Index(0);
                                const Vector2 *uv2 = mmc->Get_UV_Array_By_Index(1);
                                unsigned int *colors = mmc->Get_Color_Array(0, false);
                                mesh2->Get_Deformed_Vertices(verts, normals);
                                int vertcount3 = mesh_vertex_count;

                                if (mesh_vertex_count > 0) {
                                    do {
                                        vertexes->x = verts->X;
                                        vertexes->y = verts->Y;
                                        vertexes->z = verts->Z;
                                        vertexes->nx = normals->X;
                                        vertexes->ny = normals->Y;
                                        vertexes->nz = normals->Z;

                                        if (colors != nullptr) {
                                            vertexes->diffuse = *colors;
                                            colors++;
                                        } else {
                                            vertexes->diffuse = 0;
                                        }

                                        if (uv1 != nullptr) {
                                            vertexes->u1 = uv1->X;
                                            vertexes->v1 = uv1->Y;
                                            uv1++;
                                        } else {
                                            vertexes->u1 = 0;
                                            vertexes->v1 = 0;
                                        }

                                        if (uv2 != nullptr) {
                                            vertexes->u2 = uv2->X;
                                            vertexes->v2 = uv2->Y;
                                            uv2++;
                                        } else {
                                            vertexes->u2 = 0;
                                            vertexes->v2 = 0;
                                        }

                                        verts++;
                                        normals++;
                                        vertexes++;
                                        mesh_vertex_count--;
                                    } while (mesh_vertex_count != 0);
                                }

                                mesh2->Set_Base_Vertex_Offset(vertex_offset);
                                vertex_offset = offset;
                                renderedvertexcount += vertcount3;
                                mesh2 = mesh2->Peek_Next_Visible_Skin();
                            }

                            if (mesh2 == nullptr) {
                                break;
                            }

                            mesh1 = mesh2;
                            mesh3 = mesh4;
                        }
                    }
                }

                DX8Wrapper::Set_Vertex_Buffer(vb);
                DX8Wrapper::Set_Index_Buffer(m_indexBuffer, 0);

                for (unsigned int i = 0; i < m_passes; i++) {
                    MultiListIterator<DX8TextureCategoryClass> tex_it(&m_visibleTextureCategoryList[i]);

                    while (!tex_it.Is_Done()) {
                        tex_it.Peek_Obj()->Render();
                        tex_it.Next();
                    }
                }

                Render_Procedural_Material_Passes();

                if (renderedvertexcount >= m_visibleVertexCount) {
                    break;
                }

                mesh1 = mesh4;
            }
        }

        for (unsigned int i = 0; i < m_passes; i++) {
            while (m_visibleTextureCategoryList[i].Remove_Head() != nullptr) {
            }
        }

        captainslog_assert(renderedvertexcount == m_visibleVertexCount);
        Clear_Visible_Skin_List();
    }
}

void DX8SkinFVFCategoryContainer::Add_Delayed_Visible_Material_Pass(MaterialPassClass *pass, MeshClass *mesh)
{
    Add_Visible_Material_Pass(pass, mesh);
}

void DX8SkinFVFCategoryContainer::Render_Delayed_Procedural_Material_Passes()
{
    // does nothing
}

void DX8SkinFVFCategoryContainer::Reset()
{
    Clear_Visible_Skin_List();

    for (unsigned int i = 0; i < m_passes; i++) {
        for (;;) {
            DX8TextureCategoryClass *t = m_textureCategoryList[i].Peek_Head();

            if (t == nullptr) {
                break;
            }

            delete t;
        }
    }

    Ref_Ptr_Release(m_indexBuffer);
    m_usedIndices = 0;
}

void DX8SkinFVFCategoryContainer::Clear_Visible_Skin_List()
{
    for (MeshClass *m = m_visibleSkinHead; m != nullptr; m_visibleSkinHead = m) {
        m = m_visibleSkinHead->Peek_Next_Visible_Skin();
        m_visibleSkinHead->Set_Next_Visible_Skin(nullptr);
    }

    m_visibleSkinHead = nullptr;
    m_visibleSkinTail = nullptr;
    m_visibleVertexCount = 0;
}
